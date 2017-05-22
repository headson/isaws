/************************************************************************/
/* 作者: SoberPeng 17-05-23
/* 描述: 
/************************************************************************/
#include "calsaaudioencode.h"

CAlsaAudioEncode::CAlsaAudioEncode()
    : CAudioEncode()
    , m_nCount(0)
    , m_nBitSize(0)
    , m_pHandle(NULL)
    , m_nFrames(0)
    , m_pParams(NULL)
    , m_vBuffer(16*SZ_1K)
{
}

CAlsaAudioEncode::~CAlsaAudioEncode()
{
    Stop();
}

// 模块运行
// pArg[IN] 运行参数
// nSize[IN] 参数长度
// 成功返回1，失败返回<0
int32_t CAlsaAudioEncode::Start()
{
    int32_t nRet = 0;
    if (!is_runing())
    {
        if (_nCode == CODE_ID_G711A)
            m_nBitSize = SND_PCM_FORMAT_A_LAW;
        else
            return RET_INVALID_ARG;

        nRet = AudioOpen();
        if (nRet != RET_SUCCESS)// 打开设备
        {
            perror("CFslAudioEncode::Restart Open ALSA failed.\n");
            return nRet;
        }
        //Fm2018weInit();
        nRet = Sgtl5000Init();
        if (nRet != RET_SUCCESS)
        {
            perror("CFslAudioEncode::Restart SGTL5000 i2c set failed.\n");
            return nRet;
        }

        nRet = VExThread::Start();
        logd("Audio chn %d, sample %d, bitrate %d.", _nChannel, _nSample, _nBitrate);
        return nRet;
    }
    return RET_READY_EXIST;
}

// 模块停止
void CAlsaAudioEncode::Stop()
{
    VExThread::Stop();
    m_vBuffer.reset();

    AudioClose();
}

// 插入观察者
int32_t CAlsaAudioEncode::Attach(VOBS_SP& cObs, int32_t nChn, bool bPcm)
{
    int32_t nRet = 0;

    if (bPcm)
    {
        nRet = m_cPcmSubj.Attach(cObs);
        return nRet;
    }

    nRet = m_cAdoSubj.Attach(cObs);
    return nRet;
}

// 释放观察者
int32_t CAlsaAudioEncode::Detach(VOBS_SP& cObs, int32_t nChn, bool bPcm)
{
    if (bPcm)
    {
        m_cPcmSubj.Detach(cObs);
        return 0;
    }

    m_cAdoSubj.Detach(cObs);
    return 0;
}

/*============================================================================
函数功能: 音频采集打开
参数描述:
返回说明:
编写作者: Sober
完成时间: 2014-1-6 10:05:44
------------------------------------------------------------------------------
修改作者: Sober 2014-1-6 10:05:44
修改备注:
=============================================================================*/
int32_t CAlsaAudioEncode::AudioOpen()
{
    int rc;

    // Open PCM device
    rc = snd_pcm_open(&m_pHandle, "default", SND_PCM_STREAM_CAPTURE, 0);
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
    snd_pcm_hw_params_set_format(m_pHandle, m_pParams, (snd_pcm_format_t)m_nBitSize); //SND_PCM_FORMAT_A_LAW

    /* Two channels (stereo) */
    snd_pcm_hw_params_set_channels(m_pHandle, m_pParams, _nChannel);

    /* 44100 bits/second sampling rate (CD quality) */
    int dir;
    unsigned int val = _nSample;
    snd_pcm_hw_params_set_rate_near(m_pHandle, m_pParams, &val, &dir);

    /* Set period size to 32 frames. */
    m_nFrames = 32;
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
    return RET_SUCCESS;
}

/*============================================================================
函数功能: 音频采集关闭
参数描述:
返回说明:
编写作者: Sober
完成时间: 2014-1-6 10:05:44
------------------------------------------------------------------------------
修改作者: Sober 2014-1-6 10:05:44
修改备注:
=============================================================================*/
void CAlsaAudioEncode::AudioClose()
{
    if (m_pHandle)
    {
        snd_pcm_drain(m_pHandle);
        snd_pcm_close(m_pHandle);
        m_pHandle = NULL;
    }
}

void CAlsaAudioEncode::Process()
{
    VThread::set_cpu(3);
    ASSERT_NO(m_pHandle != NULL);

    int32_t nPacket   = 0;
    int32_t nCapture  = 0;
    const int32_t nNeedCapture = 364;
    int8_t* pCapture = new int8_t[SZ_1K];
    int8_t* pPacket = new int8_t[2*SZ_1K];
    logi("########################### start %d.", VThread::get_pid());
    while (is_runing() && pCapture)
    {
        nCapture = snd_pcm_readi(m_pHandle, pCapture, nNeedCapture);
        if (nCapture == -EPIPE) /* EPIPE means overrun */
        {
            fprintf(stderr, "%s[%d] overrun occurred.", __FILE__, __LINE__);
            snd_pcm_prepare(m_pHandle);
        }
        else if (nCapture < 0)
        {
            fprintf(stderr, "error from read: %s", snd_strerror(nCapture));
        }

        memcpy(pPacket+STREAM_BEGIN+nPacket, pCapture, nCapture);
        nPacket += nCapture;
        // AUDIO
        if (nPacket >= nNeedCapture)
        {
            TAG_PkgHeadStream *pHead= (TAG_PkgHeadStream *)pPacket;
            PkgHeadStream((int8_t*)pHead, PKG_ENC_STREAM);
            pHead->eCmdMinor    = 0x7c;
            pHead->nSeq         = m_nCount++;
            pHead->eFrm         = (uint8_t)EFRM_I;
            pHead->nLast        = (uint8_t)0;
            pHead->nTime        = VDateTime::get().to_msec();

            pHead->eCode        = CODE_ID_G711A;
            pHead->nChnx        = (uint16_t)_nChannel;
            pHead->nRvd1        = (uint16_t)_nSample;
            pHead->nRvd2        = (uint16_t)_nBitrate;

            pHead->eCmdMinor    = (int32_t)g_app.nAIState[0]
            | (int32_t)g_app.nAIState[1] << 8
                | (int32_t)g_app.nAIState[2] << 16
                | (int32_t)g_app.nAIState[3] << 24;

            pHead->nPkgLen      = STREAM_BEGIN - HEAD_LEN_STR;

            pHead->nPkgLen      += nPacket;
            m_cAdoSubj.Notify(PKG_ENC_STREAM, pPacket, pHead->nPkgLen+HEAD_LEN_STR, NULL);
            nPacket = 0;
            //printf("-------------------------------- %d-%lld.\n", pHead->nPkgLen+HEAD_LEN_STR, VTime::get_msec());
        }
        inc_living();
    }
    logi("########################### end %d.", VThread::get_pid());

    //if (pAecAdo) { delete[] pAecAdo; pAecAdo = NULL; }
    if (pPacket) { delete[] pPacket; pPacket = NULL; }
    if (pCapture) { delete[] pCapture; pCapture = NULL; }
}

int32_t CAlsaAudioEncode::Sgtl5000Init()
{
//#if HARD_VER < 200
    int fd = i2c_open("/dev/i2c-2", 0x0A);
    if (fd <= 0)
    {
        loge("sgtl5000 open i2c failed.");
        return RET_FOPEN_FAILED;
    }

    char sData[4] = {0};
    struct TAG_ADO_REG{

        int32_t nAddr;
        int32_t nValue;
    } aAdoReg[] = {
        {0x0010, 0x3c3c}, {0x0022, 0x1818},
        {0x002a, 0x0131}, {0x0020, 0x000a}
    };
    size_t nCount = sizeof(aAdoReg)/sizeof(TAG_ADO_REG);

    for (size_t i = 0; i < nCount; i++)
    {
        sData[0] = (aAdoReg[i].nAddr & 0xff00) >> 8;
        sData[1] = aAdoReg[i].nAddr & 0xff;
        sData[2] = (aAdoReg[i].nValue & 0xff00) >> 8;
        sData[3] = aAdoReg[i].nValue & 0xff;
        if (i2c_write(fd, sData, sizeof(sData))) 
        {
            //loge("----------write %d-%d failed.", aAdoReg[i].nAddr, aAdoReg[i].nValue);
            break;
        }
    }

    i2c_close(fd);
//#endif
    return RET_SUCCESS;
}

bool CAlsaAudioEncode::HaveSameHandle(string sName, bool bPcm)
{
    if (bPcm) {
        return m_cPcmSubj.Find(sName);
    }

    return m_cAdoSubj.Find(sName);
}

#endif // #ifdef IMX6Q
