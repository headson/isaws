/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* �ļ�����: cdevphoto.h
* ��Ҫ˵��: ��Ƭ����
* �������:
* ��д����: Sober.Peng
* �������: 2013-11-14
*-----------------------------------------------------------------------------
* �޶�����:
* �޶�ʱ��:
*-----------------------------------------------------------------------------
******************************************************************************/
#include "cphotoencode.h"

#include "yuv420.h"

CPhotoEncode::CPhotoEncode(int32_t eCodec)
    : CPhotoEncode(eCodec)
    , m_bInited(false)
{
    m_cVpu.eEncFormat = STD_MJPG;
    if (eCodec == CODE_ID_H264)
        m_cVpu.eEncFormat = STD_AVC;
}

CPhotoEncode::~CPhotoEncode()
{
    if (m_bInited)
    {
        m_bInited = false;

        m_cVpu.vdo_stop();

        m_cVpu.vpu_free_srcbuffs(&m_cVpuSrc);
    }
}

/*============================================================================
��������: ��Ƭ����
��������: pDst[OUT] ���������
          nDst[IN]  ��������ݴ�С
          pSrc[IN]  ����ǰͼ������
          nW[IN]    ����ǰͼ���
          nH[IN]    ����ǰͼ���
����˵��: �����ͼ�����ݴ�С
��д����: Sober
���ʱ��: 2014-9-9 14:01:54
=============================================================================*/
int32_t CPhotoEncode::Encode(int8_t* pDst, int32_t nDst, int8_t* pSrc, int32_t nW, int32_t nH)
{
    int32_t nRet = -1;
    if (m_bInited == false)
    {
        m_cVpu.enc_fd     = NULL;
        m_cVpu.nWidth     = nW;
        m_cVpu.nHeight    = nH;
        m_cVpu.nBitrate   = 0;
        m_cVpu.nIGopSize  = 3;
        if (m_cVpu.vpu_alloc_srcbuffs(&m_cVpuSrc, m_cVpu.nWidth, m_cVpu.nHeight) < 0)
        {
            loge("%s[%d] vpu source alloc failed.", __FILE__, __LINE__);
            return nRet;
        }

        nRet = m_cVpu.vdo_restart();
        if (nRet != RET_SUCCESS)
        {
            loge("vpu restart failed.");
            m_cVpu.vpu_free_srcbuffs(&m_cVpuSrc);
            return nRet;
        }

        logd("photo encode %d-%d create success.", nW, nH);
        m_bInited = true;
    }

    if (m_cVpu.nWidth != nW || m_cVpu.nHeight != nH)
    {
        loge("New image width %d-%d, height %d-%d.", m_cVpu.nWidth, nW, m_cVpu.nHeight, nH);
        return nRet;
    }
    if (m_bInited && pDst && nDst)
    {
        volatile bool bRuning = true;
        int32_t  nImgSize = m_cVpu.nWidth*m_cVpu.nHeight*3/2;

        /*FILE *file = fopen("test.yuv", "wb+");
        fwrite(pSrc, 1, nImgSize, file);
        fclose(file);*/
        memcpy((char*)m_cVpuSrc.stFrmDesc.virt_uaddr, pSrc, nImgSize);
        nRet = m_cVpu.enc_process(&m_cVpuSrc.stFrmBuff, pDst, nDst, bRuning);

        _isIFrame = m_cVpu.bGetIFrame;
        m_cVpu.bGetIFrame = false;

    }
    return nRet;
}




