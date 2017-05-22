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
        FrameBuffer  stFrmBuff;  // 存储使用地址
        vpu_mem_desc stFrmDesc;
    } TAG_VPU_SRC;

public:
    CVpu();

    ~CVpu();

    // 编码创建
    int32_t vpu_setup();

    // 编码关闭
    void    vpu_close();

    // 分配buffer
    int32_t vpu_alloc_encbuffs();
    // 释放buffer
    void    vpu_free_encbuffs();

    //////////////////////////////////////////////////////////////////////////
    // 编码源分配
    int32_t vpu_alloc_srcbuffs(TAG_VPU_SRC *pSrc, int w, int h);

    // 编码源释放
    void    vpu_free_srcbuffs(TAG_VPU_SRC *pSrc);

    // 视频\照片编码
    int32_t enc_process(FrameBuffer *pSrcFrm, int8_t *pDstData, int32_t nDstSize, volatile bool& bRuning);

public:
    int32_t vdo_restart();
    void    vdo_stop();
    
public:
    EncHandle       enc_fd;
    uint16_t        nWidth, nHeight;    // 

    uint32_t        nBitrate;       // 码流
    uint32_t        nIGopSize;      // I帧间隔
    CodStd          eEncFormat;     // 编码格式
    bool            bGetIFrame;     // 获取I帧

public:
    vpu_mem_desc    cMemDesc;       // 内存描述

    uint32_t        nFrmNums;       // 分配FB个数
    FrameBuffer*    pFrmBufs;       // 存储使用地址
    vpu_mem_desc*   pFrmDesc;       // 存储使用地址

public:
    uint32_t        nHeadData;          // 
    uint8_t         sHeadData[2*SZ_1K]; // 
};

#endif