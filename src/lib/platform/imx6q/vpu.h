/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : vpu.h
* Author        : Sober.Peng
* Date          : 7:2:2017
* Description   :
*-----------------------------------------------------------------------------
* Modify        : 
*-----------------------------------------------------------------------------
******************************************************************************/
#pragma once
#ifdef IMX6Q
#ifdef __cplusplus
extern "C" {
#endif
#include "sdk/imx6q/inc/ipu.h"
#include "sdk/imx6q/inc/vpu_lib.h"
#include "sdk/imx6q/inc/vpu_io.h"
#include "sdk/imx6q/inc/videodev2.h"
#ifdef __cplusplus
}
#endif

class CVpu
{
public:
    #define VPU_BUFFER  0x200000// 2M
    typedef struct  
    {
        FrameBuffer  stFrmBuff;  // �洢ʹ�õ�ַ
        vpu_mem_desc stFrmDesc;
    } TAG_VPU_SRC;

public:
    CVpu();

    ~CVpu();

    // ���봴��
    int32_t vpu_setup();

    // ����ر�
    void    vpu_close();

    // ����buffer
    int32_t vpu_alloc_encbuffs();
    // �ͷ�buffer
    void    vpu_free_encbuffs();

    //////////////////////////////////////////////////////////////////////////
    // ����Դ����
    int32_t vpu_alloc_srcbuffs(TAG_VPU_SRC *pSrc, int w, int h);

    // ����Դ�ͷ�
    void    vpu_free_srcbuffs(TAG_VPU_SRC *pSrc);

    // ��Ƶ\��Ƭ����
    int32_t enc_process(FrameBuffer *pSrcFrm, int8_t *pDstData, int32_t nDstSize, volatile bool& bRuning);

public:
    int32_t vdo_restart();
    void    vdo_stop();
    
public:
    EncHandle       enc_fd;
    uint16_t        nWidth, nHeight;    // 

    uint32_t        nBitrate;       // ����
    uint32_t        nIGopSize;      // I֡���
    CodStd          eEncFormat;     // �����ʽ
    bool            bGetIFrame;     // ��ȡI֡

public:
    vpu_mem_desc    cMemDesc;       // �ڴ�����

    uint32_t        nFrmNums;       // ����FB����
    FrameBuffer*    pFrmBufs;       // �洢ʹ�õ�ַ
    vpu_mem_desc*   pFrmDesc;       // �洢ʹ�õ�ַ

public:
    uint32_t        nHeadData;          // 
    uint8_t         sHeadData[2*SZ_1K]; // 
};

#endif