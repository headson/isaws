/************************************************************************
*Author      : Sober.Peng 17-07-25
*Description : 
************************************************************************/
#ifndef LIBPLATFORM_VPU_H
#define LIBPLATFORM_VPU_H

#ifdef __cplusplus
extern "C" {
#endif
#include "sdk/inc/ipu.h"
#include "sdk/inc/vpu_lib.h"
#include "sdk/inc/vpu_io.h"
#include "sdk/inc/videodev2.h"
#ifdef __cplusplus
}
#endif

#include <stdio.h>

#include "systemv/shm/vzshm_c.h"

class CVpu {
 public:
#define VPU_BUFFER  0x200000// 2M
  typedef struct {
    FrameBuffer  stFrmBuff;  // 存储使用地址
    vpu_mem_desc stFrmDesc;
  } TAG_VPU_SRC;

 public:
  CVpu();
  ~CVpu();

  // 编码创建
  bool vpu_setup();

  // 编码关闭
  void vpu_close();

  // 分配buffer
  int  vpu_alloc_encbuffs();
  // 释放buffer
  void vpu_free_encbuffs();

  //////////////////////////////////////////////////////////////////////////
  // 编码源分配
  int  vpu_alloc_srcbuffs(TAG_VPU_SRC *pSrc, int w, int h);

  // 编码源释放
  void vpu_free_srcbuffs(TAG_VPU_SRC *pSrc);

  // 视频\照片编码
  int enc_process(FrameBuffer *pSrcFrm, char *pDstData, int nDstSize);

  // 视频\照片编码
  int enc_h264(FrameBuffer *pSrcFrm, CShareBuffer *shm_vdo);

 public:
  bool vdo_restart();
  void vdo_stop();

 public:
  EncHandle       enc_fd;
  unsigned int    nWidth, nHeight;    //

  unsigned int    nBitrate;       // 码流
  unsigned int    nIGopSize;      // I帧间隔
  CodStd          eEncFormat;     // 编码格式
  bool            bGetIFrame;     // 获取I帧

 public:
  vpu_mem_desc    cMemDesc;       // 内存描述

  unsigned int    nFrmNums;       // 分配FB个数
  FrameBuffer*    pFrmBufs;       // 存储使用地址
  vpu_mem_desc*   pFrmDesc;       // 存储使用地址

  FILE           *file;

 public:
  int   sps_size_;    //
  int   pps_size_;    //
  int   jpeg_size_;   //

  char  head_[1024];  // 
};

#endif  // LIBPLATFORM_VPU_H
