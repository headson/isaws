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
    FrameBuffer  stFrmBuff;  // �洢ʹ�õ�ַ
    vpu_mem_desc stFrmDesc;
  } TAG_VPU_SRC;

 public:
  CVpu();
  ~CVpu();

  // ���봴��
  bool vpu_setup();

  // ����ر�
  void vpu_close();

  // ����buffer
  int  vpu_alloc_encbuffs();
  // �ͷ�buffer
  void vpu_free_encbuffs();

  //////////////////////////////////////////////////////////////////////////
  // ����Դ����
  int  vpu_alloc_srcbuffs(TAG_VPU_SRC *pSrc, int w, int h);

  // ����Դ�ͷ�
  void vpu_free_srcbuffs(TAG_VPU_SRC *pSrc);

  // ��Ƶ\��Ƭ����
  int enc_process(FrameBuffer *pSrcFrm, char *pDstData, int nDstSize);

  // ��Ƶ\��Ƭ����
  int enc_h264(FrameBuffer *pSrcFrm, CShareBuffer *shm_vdo);

 public:
  bool vdo_restart();
  void vdo_stop();

 public:
  EncHandle       enc_fd;
  unsigned int    nWidth, nHeight;    //

  unsigned int    nBitrate;       // ����
  unsigned int    nIGopSize;      // I֡���
  CodStd          eEncFormat;     // �����ʽ
  bool            bGetIFrame;     // ��ȡI֡

 public:
  vpu_mem_desc    cMemDesc;       // �ڴ�����

  unsigned int    nFrmNums;       // ����FB����
  FrameBuffer*    pFrmBufs;       // �洢ʹ�õ�ַ
  vpu_mem_desc*   pFrmDesc;       // �洢ʹ�õ�ַ

  FILE           *file;

 public:
  int   sps_size_;    //
  int   pps_size_;    //
  int   jpeg_size_;   //

  char  head_[1024];  // 
};

#endif  // LIBPLATFORM_VPU_H
