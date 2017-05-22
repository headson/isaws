/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : cfslvideoencode.cpp
* Author        : Sober.Peng
* Date          : 9:2:2017
* Description   :
*-----------------------------------------------------------------------------
* Modify        : 
*-----------------------------------------------------------------------------
******************************************************************************/
#include "cvideoencode.h"

#include "asc8.h"
#include "yuv420.h"

CVideoEncode::CVideoEncode(int32_t nChn)
    : n_chn_(nChn)
    , m_i2c(0)
    , m_pPkgBuffer(NULL)
    , m_nStartTime(0)
    , m_bResetCamera(false)
{
    m_cImage.pVideo = NULL;
    m_cImage.nVideo = 0;
}

CVideoEncode::~CVideoEncode()
{
    Stop();
}

int32_t CVideoEncode::Start()
{
    int32_t nRet = RET_SUCCESS;

    // 打开VPU
    if (m_cVpu.enc_fd == 0)
    {
        nRet = m_cVpu.vdo_restart();
        if (nRet != RET_SUCCESS)
        {
            loge("CFslVideoEncode::init(): failed %d.", nRet);
            return nRet;
        }
    }

    nRet = VExThread::Start();
    logd("CFslVideoEncode::init(): chn %d, start time %d, thread start %d.", _nChnx, m_nStartTime, nRet);

    nRet = m_cImage.vThread.Start(ThreadImage, this);
    return nRet;
}

void CVideoEncode::Stop()
{
    VExThread::Stop();
    m_cImage.vThread.Stop();

    m_cVpu.vdo_stop();  // 关闭VPU

    if (m_cImage.pVideo) { delete[] m_cImage.pVideo; m_cImage.pVideo = NULL; }

    m_cVdoSubj.Clear();
    m_cYuvSubj.Clear();
}

bool CVideoEncode::HaveSameHandle(string sName, int32_t nChn)
{
    return m_cVdoSubj.Find(sName);
}

// 发送用户数据
int32_t CVideoEncode::SendUserData(int8_t* pData, int32_t nData)
{
    return m_cVdoSubj.Notify(CMD_USER_CMD, pData, nData, NULL);
}

// 插入观察者
int32_t CVideoEncode::Attach(VOBS_SP& cObs, int32_t nChn, bool bYuv)
{
    int32_t nRet = 0;

    if (bYuv)
    {
        nRet = m_cYuvSubj.Attach(cObs);
        return nRet;
    }

    nRet = m_cVdoSubj.Attach(cObs);
    m_cVpu.bGetIFrame = true;
    return nRet;
}

// 释放观察者
int32_t CVideoEncode::Detach(VOBS_SP& cObs, int32_t nChn, bool bYuv)
{
    int32_t nRet = 0;

    if (bYuv)
    {
        nRet = m_cYuvSubj.Detach(cObs);
        return nRet;
    }

    nRet = m_cVdoSubj.Detach(cObs);
    return nRet;
}

void CVideoEncode::ResetCamera()
{
    m_bResetCamera = true;
}

void CVideoEncode::Process()
{
    if (_nChnx == 1)
        VThread::set_cpu(0);
    VThread::set_cpu(2);

    int32_t nRet = 0;
#if (HARD_VER != DEV_TYPE_SW1)
    int32_t nScale = 1;
    uint32_t nTmOld = 0, nColor = 0;
    if (m_cVpu.nWidth==720)      nScale = 2;
    else if(m_cVpu.nWidth==1280) nScale = 3;
    else if(m_cVpu.nWidth>=1600) nScale = 4;
#endif
    // start capture    
    struct v4l2_buffer v4l_buf;
    int32_t nFrmSeqs=0, nSeqs=0;
    FrameBuffer* pSrcFrm = NULL;                        // 帧源数据
    uint32_t nImgSize = m_cVpu.nWidth*m_cVpu.nHeight;   // 照片大小
    if (!m_pPkgBuffer) { m_pPkgBuffer = new int8_t[nImgSize]; }

    int32_t nOpenTimes = 0;

REOPEN:
    // 打开V4L2
    nRet = m_cV4l2.v4l_open();
    if (nRet != RET_SUCCESS)
    {
        loge("################## CFslVideoEncode::Process: open v4l2 %s failed.", m_cV4l2.sVideo.c_str());
        goto err;
    }
    nRet = m_cV4l2.v4l_start_capturing();
    if (nRet != RET_SUCCESS)
    {
        loge("################## CFslVideoEncode::Process: capture v4l2 %s failed.", m_cV4l2.sVideo.c_str());
        goto err;
    }
    //VTime::msleep(1*1000);
    CamISPInit();

    pSrcFrm = &m_cVpu.pFrmBufs[m_cVpu.nFrmNums-1];
    logi("################## start %d-%d, %d.", _nChnx, m_nYuvScale, VThread::get_pid());

    // 采集编码
    while (is_runing())
    {
        nRet = m_cV4l2.v4l_get_capture_data(&v4l_buf);
        if (m_bResetCamera || nRet != RET_SUCCESS)
        {
            logw("video %s restart result %d, and open times %d.", 
                m_cV4l2.sVideo.c_str(), nRet, nOpenTimes);

            // 关闭V4L2
            m_cV4l2.v4l_stop_capturing();
            m_cV4l2.v4l_close();

            if (m_bResetCamera) {
                nOpenTimes = 0;
                m_bResetCamera = false;
            }

            VThread::msleep(1000);
            if ((nOpenTimes++) > 5) {  
                goto err; 
            } else { 
                goto REOPEN; 
            }
        }
        nFrmSeqs++;     // 采集到的帧累加
        int nW=m_cV4l2.nWidth, nH=m_cV4l2.nHeight;  // 处理后YUV数据宽、高
        if ((_nChnx == VI_1) && (m_cV4l2.nWidth==1024) && (m_cV4l2.nHeight==768))
        {
            nW=720; nH=576;
            if (m_cImage.pVideo == NULL) {
                m_cImage.pVideo = new int8_t[nW * nH * 3 / 2];
            }

            YUV1024x768toD1((uint8_t*)m_cV4l2.sBuffer[v4l_buf.index].start, m_nYuvScale);
            if (m_cImage.pVideo)
            {
                m_cImage.nVideo = nW * nH;
                memcpy(m_cImage.pVideo, m_cV4l2.sBuffer[v4l_buf.index].start, m_cImage.nVideo);

                m_cImage.vEvent.notify();
            }
        }
        else
        {
            if (m_cImage.pVideo == NULL)
                m_cImage.pVideo = new int8_t[nW * nH * 3 / 2];

            if (m_cImage.pVideo)
            {
                if (_nChnx == VI_2){
                    m_cImage.nVideo = nW * nH * 3 / 2;
                } else {
                    m_cImage.nVideo = nW * nH;
                }
                memcpy(m_cImage.pVideo, m_cV4l2.sBuffer[v4l_buf.index].start, m_cImage.nVideo);
                m_cImage.vEvent.notify();
            }
        }

        if (m_cVdoSubj.Size() > 0 && ((nFrmSeqs%m_cV4l2.nFrmLost)==0)) // 丢帧
        {
            if ((nW != m_cVpu.nWidth) && (nH != m_cVpu.nHeight))
            {
                ImageResizeNN((uint8_t*)m_cV4l2.sBuffer[v4l_buf.index].start, nW, nH, m_cVpu.nWidth, m_cVpu.nHeight);
                memset((void*)((int8_t*)m_cV4l2.sBuffer[v4l_buf.index].start+m_cVpu.nWidth*m_cVpu.nHeight), 128, m_cVpu.nWidth*m_cVpu.nHeight/2);
            }

            // 视频处理
            VDateTime vdt = VDateTime::get();
            TAG_PkgHeadStream* pHead = (TAG_PkgHeadStream*)m_pPkgBuffer; // 组包头
            PkgHeadStream((int8_t*)pHead, PKG_ENC_STREAM);
            pHead->nSeq         = (uint16_t)(nSeqs++);
            pHead->eFrm         = (uint8_t)EFRM_P;
            pHead->nLast        = (uint8_t)0;
            pHead->nTime        = vdt.to_msec();

            pHead->eCode        = (uint16_t)m_eCodec;
            pHead->nChnx        = (uint16_t)_nChnx;
            pHead->nRvd1        = (uint16_t)m_cVpu.nWidth;
            pHead->nRvd2        = (uint16_t)m_cVpu.nHeight;
            pHead->nPkgLen      = STREAM_BEGIN - HEAD_LEN_STR;

            // OSD
#if (HARD_VER != DEV_TYPE_SW1)// OC 图像需要90度旋转
            uint32_t nTmNow     = vdt.tsec();
            if (nTmOld != nTmNow) { nTmOld = nTmNow; nColor = nColor ? 0 : 255; }
            yuv_osd(nColor, (uint8_t*)m_cV4l2.sBuffer[v4l_buf.index].start, 
                m_cVpu.nWidth, m_cVpu.nHeight, (char*)vdt.to_string().c_str(), nScale, asc8, 10, 10);
#endif
            // 编码
            pSrcFrm->myIndex = m_cVpu.nFrmNums-1+v4l_buf.index;
            pSrcFrm->bufY    = m_cV4l2.sBuffer[v4l_buf.index].offset;
            pSrcFrm->bufCb   = pSrcFrm->bufY  + nImgSize;
            pSrcFrm->bufCr   = pSrcFrm->bufCb + (nImgSize >> 2);
            pSrcFrm->strideY = m_cVpu.nWidth;
            pSrcFrm->strideC = m_cVpu.nHeight >> 1;
            nRet = m_cVpu.enc_process(pSrcFrm, m_pPkgBuffer+STREAM_BEGIN, nImgSize*3/2-STREAM_BEGIN, _isRuning);
            if (nRet)
            {
                pHead->nPkgLen += nRet;
                if (m_cVpu.bGetIFrame) { pHead->eFrm = (uint8_t)EFRM_I; }

                m_cVdoSubj.Notify(PKG_ENC_STREAM, m_pPkgBuffer, pHead->nPkgLen+HEAD_LEN_STR, NULL);  // 通知其它
                m_cVpu.bGetIFrame = false;
            }
        }
        m_cV4l2.v4l_put_capture_data(&v4l_buf);

        inc_living();
    }
    rel_living();

err:
    m_cVdoSubj.Clear();
    if (m_pPkgBuffer) { delete[] m_pPkgBuffer; m_pPkgBuffer = NULL; }

    logi("end %d, %d.", _nChnx, VThread::get_pid());
    // 关闭V4L2
    m_cV4l2.v4l_stop_capturing();
    m_cV4l2.v4l_close();
}

void CVideoEncode::OpenISPAddr(std::string sAddr, int32_t nSlave) 
{
    m_i2c = i2c_open(sAddr.c_str(), nSlave);
    logd("--------------- open i2c %s, slave %d.", sAddr.c_str(), nSlave);
}

void CVideoEncode::SetISPValue(int32_t nReg, int32_t nValue) 
{
    if (nReg == -1)
    {
        m_nYuvScale = nValue;
        return;
    }
    else if (nReg == 1)
    {
        m_nMaxExp = nValue;
        SetISPValue(0x3502, (nValue & 0xff));
        SetISPValue(0x3501, ((nValue & 0xff00) >> 8));
    }
    else if (nReg == 2)
    {
        m_nMaxGain = nValue;
        SetISPValue(0x350b, (nValue & 0xff));
        SetISPValue(0x350a, ((nValue & 0xff00) >> 8));
    }

    if (m_i2c > 0)
    {
        char buf[3] = {0};
        buf[0] = (nReg & 0xffff) >> 8;
        buf[1] = nReg & 0xff;
        buf[2] = nValue & 0xff;
        if (i2c_write(m_i2c, buf, 3) != RET_SUCCESS)
            logd("chn %d-0x%x i2c write 0x%x-%d failed.", _nChnx, m_i2c, nReg, nValue);
    }
}

int32_t CVideoEncode::GetISPValue(int32_t nReg, int32_t& nValue)
{
    char buf[3] = {0};
    buf[0] = (nReg & 0xffff) >> 8;
    buf[1] = nReg  & 0xff;

    if (i2c_write(m_i2c, buf, 2) == RET_SUCCESS)
    {
        int32_t nRead = i2c_read(m_i2c, (char*)&nValue, 1);
        if (nRead != RET_SUCCESS)
        {
            logd("chn %d-0x%x i2c read 0x%x-%d failed.", _nChnx, m_i2c, nReg, nValue);
            return RET_EIOCTL_FAILED;
        }
        return RET_SUCCESS;
    }
    
    return RET_EIOCTL_FAILED;
}

void CVideoEncode::CamISPInit()
{
    typedef struct {
        int32_t nReg;
        int32_t nVal;
    } TAG_VDO_ISP;
    TAG_VDO_ISP isp_0x3c[] = {
        // AGC,AEC
        {0x3503, 0x03},
        // 增益
        {0x350a, ((m_nMaxGain & 0xff00) >> 8)}, 
        {0x350b, (m_nMaxGain & 0xff)},
        // 快门
        {0x3500, 0x00}, 
        {0x3501, ((m_nMaxExp & 0xff00) >> 8)}, 
        {0x3502, (m_nMaxExp & 0xff)},
        // 黑白
        {0x3212, 0x03}, {0x5580, 0x1e}, {0x5583, 0x80}, {0x5584, 0x80},
        {0x5003, 0x08}, {0x3212, 0x13}, {0x3212, 0xa3}
    };
    uint32_t nCount = sizeof(isp_0x3c)/sizeof(TAG_VDO_ISP);

    if ((m_i2c > 0) && (nCount > 0))
    {
        for (uint32_t i = 0; i < nCount; i++)
        {
            SetISPValue(isp_0x3c[i].nReg, isp_0x3c[i].nVal);
        }
    }
    logd("Camera %d ISP initinal.", _nChnx);
}

void* CVideoEncode::ThreadImage(void* pArg)
{
    if (pArg)
        ((CVideoEncode*)pArg)->NotifyImage();

    return NULL;
}

void CVideoEncode::NotifyImage()
{
    while (is_runing())
    {
        if (m_cImage.nVideo)
        {
            m_cYuvSubj.Notify(PKG_YUV_STREAM, (int8_t*)m_cImage.pVideo, m_cImage.nVideo, &_nChnx);
            m_cImage.nVideo = 0;
        }
        else
        {
            m_cImage.vEvent.waits(100);
        }
    }
}
#endif // IMX6Q
