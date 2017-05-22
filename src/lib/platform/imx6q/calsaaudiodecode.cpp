
#include "calsaaudiodecode.h"

CAlsaAudioDecode::CAlsaAudioDecode()
    : CAudioDecode()
    , m_nFormat(SND_PCM_FORMAT_A_LAW)
    , m_pHandle(NULL)
    , m_nFrames(0)
    , m_pParams(NULL)
    , m_vBuffer(16*SZ_1K)
{
    m_nChannel   = 1;
    m_nSample = 8000;
}

CAlsaAudioDecode::~CAlsaAudioDecode()
{
    Stop();
}

// 模块运行
// pArg[IN] 运行参数
// nSize[IN] 参数长度
// 成功返回1，失败返回<0
int32_t CAlsaAudioDecode::Start()
{
    int32_t nRet = 0;

    if (m_nCode == CODE_ID_G711A)
        m_nFormat = SND_PCM_FORMAT_A_LAW;
    else
        return RET_INVALID_ARG;

    nRet = PcmOpen();
    if (nRet != RET_SUCCESS)
    {
        loge("CAlsaAudioDecode::Start audio decode open failed.");
        return nRet;
    }

    nRet = VExThread::Start();
    return nRet;
}

// 模块停止
void CAlsaAudioDecode::Stop()
{
    VExThread::Stop();

    PcmClose();
}

int32_t CAlsaAudioDecode::Decode(const int8_t* pData, uint32_t nData, const void* pHdlPlay, bool bClearAudio)
{
    //static FILE* file = NULL;
    //if (access("./test.g711a", 0) != 0)
    //{
    //    if (file) {
    //        fclose(file); file = NULL;
    //    }
    //    file = fopen("./test.g711a", "wb+");
    //}

    //if (file)
    //{
    //    fwrite(pData, 1, nSize, file);
    //    fflush(file);
    //}

    int32_t nRet = 0;
    if (is_play())                                      // 播放
    {
        if (m_pHdlPlay == pHdlPlay) {                   // 同一个handle
            nRet = PcmPlay((int8_t*)pData, nData, bClearAudio);
        } else {                                        // 不同handle
            nRet = RET_HDL_IS_BUSY;
        }
    } else {                                            // 未播放
        m_bPlaying = true;
        m_pHdlPlay = const_cast<void*>(pHdlPlay);
        nRet = PcmPlay((int8_t*)pData, nData, bClearAudio);
    }
    return nRet;
}

///SGTL5000////////////////////////////////////////////////////////////////////////////////
int32_t CAlsaAudioDecode::PcmOpen()
{
    int rc;
    char sName[] = "default";

    // Open PCM device
    rc = snd_pcm_open(&m_pHandle, sName, SND_PCM_STREAM_PLAYBACK, 0);
    if (rc < 0)
    {
        fprintf(stderr, "unable to open pcm device: %s\n", snd_strerror(rc));
        return RET_START_ERROR;
    }

    /* Allocate a hardware parameters object. */
    snd_pcm_hw_params_alloca(&m_pParams);

    /* Fill it in with default values. */
    snd_pcm_hw_params_any(m_pHandle, m_pParams);

    /* Set the desired hardware parameters. */
    /* Interleaved mode */
    snd_pcm_hw_params_set_access(m_pHandle, m_pParams, SND_PCM_ACCESS_RW_INTERLEAVED);

    /* Signed 16-bit little-endian format */
    snd_pcm_hw_params_set_format(m_pHandle, m_pParams, (snd_pcm_format_t)m_nFormat); //SND_PCM_FORMAT_A_LAW

    /* Two channels (stereo) */
    snd_pcm_hw_params_set_channels(m_pHandle, m_pParams, m_nChannel);

    /* 44100 bits/second sampling rate (CD quality) */
    int dir;
    unsigned int val = m_nSample;
    snd_pcm_hw_params_set_rate_near(m_pHandle, m_pParams, &val, &dir);

    /* Set period size to 32 frames. */
    m_nFrames = 8;
    snd_pcm_hw_params_set_period_size_near(m_pHandle, m_pParams, &m_nFrames, &dir);

    /* Write the parameters to the driver */
    rc = snd_pcm_hw_params(m_pHandle, m_pParams);
    if (rc < 0)
    {
        fprintf(stderr, "unable to set hw parameters: %s", snd_strerror(rc));
        snd_pcm_drain(m_pHandle);
        snd_pcm_close(m_pHandle);
        return RET_START_ERROR;
    }

    /* Use a buffer large enough to hold one period */
    snd_pcm_hw_params_get_period_size(m_pParams, &m_nFrames, &dir);

    //snd_pcm_hw_params_get_period_time(m_pParams, &val, &dir);
    logd("chn %d, sample %d, pcm format %d, frames %d.", 
        m_nChannel, m_nSample, m_nFormat, (int32_t)m_nFrames);
    return RET_SUCCESS;
}

void CAlsaAudioDecode::PcmClose()
{
    if (m_pHandle)
    {
        snd_pcm_drain(m_pHandle);
        snd_pcm_close(m_pHandle);
    }
}

int32_t CAlsaAudioDecode::PcmPlay(int8_t *pData, int32_t nData, bool bReset)
{
    if (bReset)
        m_vBuffer.reset();

    if (m_vBuffer.write_n(pData, nData))
    {
        m_vEvent.notify();
        return RET_SUCCESS;
    }

    return RET_INVALID_HDL;
}

void CAlsaAudioDecode::Process()
{
    VThread::set_cpu(3);
    ASSERT_NO(m_pHandle != NULL);

    int32_t nFreeTimes = 0;
    const int32_t VOC_FRAME = 1024;
    int8_t* pPlayback = new int8_t[VOC_FRAME + 1];
    logi("########################### start %d.", VThread::get_pid());
    while (is_runing() && pPlayback)
    {
        int32_t nPlayback = m_vBuffer.read_n(pPlayback, VOC_FRAME);
        if (nPlayback > 0)
        {
            int32_t nOffset = 0;
            while (nOffset < nPlayback)
            {
                int32_t rc = snd_pcm_writei(m_pHandle, pPlayback+nOffset, nPlayback-nOffset);
                if (rc > 0)
                {
                    nOffset += rc;
                }
                else if (rc == -EPIPE)
                {
                    rc = snd_pcm_prepare(m_pHandle);
                    if (rc < 0)
                    {
                        break;
                    }
                    else if (rc == -ESTRPIPE)
                    {
                        while ((rc = snd_pcm_resume(m_pHandle)) == -EAGAIN) {
                            VThread::msleep(1);/* wait until the suspend flag is released */ 
                        }
                        if (rc < 0) {
                            rc = snd_pcm_prepare(m_pHandle);
                            if (rc < 0)
                                break;
                        }
                    }
                }
                else
                {
                    break;
                }

                nFreeTimes = 0;
            }
        } else {
            VThread::msleep(10);

            if ((nFreeTimes++) >= 50) { // 500ms
                m_pHdlPlay = NULL;
                m_bPlaying = false;
            }
        }

        inc_living();
    }

    if (pPlayback) { delete[] pPlayback; pPlayback = NULL; }
    logi("########################### end %d.", VThread::get_pid());
}

