/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* 文件名称: cdevphoto.h
* 简要说明: 照片编码
* 相关描述:
* 编写作者: Sober.Peng
* 完成日期: 2013-11-14
*-----------------------------------------------------------------------------
* 修订作者:
* 修订时间:
*-----------------------------------------------------------------------------
******************************************************************************/
#include "cimageencode.h"

#include <string.h>

#include "yuv420.h"

#include "vzlogging/logging/vzlogging.h"

CImageEncode::CImageEncode(int eCodec)
  : bInited(false) {
  vpu_.eEncFormat = (CodStd)eCodec;
}

CImageEncode::~CImageEncode() {
  if (bInited) {
    bInited = false;

    vpu_.vdo_stop();
    vpu_.vpu_free_srcbuffs(&vpu_src_);
  }
}

/*============================================================================
函数功能: 照片编码
参数描述: pDst[OUT] 编码后数据
          nDst[IN]  编码后数据大小
          pSrc[IN]  编码前图像数据
          nW[IN]    编码前图像宽
          nH[IN]    编码前图像高
返回说明: 编码后图像数据大小
编写作者: Sober
完成时间: 2014-9-9 14:01:54
=============================================================================*/
int CImageEncode::Encode(char* pDst, int nDst, char* pSrc, int nW, int nH) {
  int nRet = -1;
  if (bInited == false) {
    vpu_.enc_fd     = NULL;
    vpu_.nWidth     = nW;
    vpu_.nHeight    = nH;
    vpu_.nBitrate   = 0;
    vpu_.nIGopSize  = 3;
    if (vpu_.vpu_alloc_srcbuffs(&vpu_src_, vpu_.nWidth, vpu_.nHeight) < 0) {
      LOG_ERROR("%s[%d] vpu source alloc failed.", __FILE__, __LINE__);
      return nRet;
    }

    if (false == vpu_.vdo_restart()) {
      LOG_ERROR("vpu restart failed.");
      vpu_.vpu_free_srcbuffs(&vpu_src_);
      return nRet;
    }

    LOG_INFO("photo encode %d-%d create success.", nW, nH);
    bInited = true;
  }

  if (vpu_.nWidth != nW || vpu_.nHeight != nH) {
    LOG_ERROR("New image width %d-%d, height %d-%d.", vpu_.nWidth, nW, vpu_.nHeight, nH);
    return nRet;
  }
  if (bInited && pDst && nDst) {
    volatile bool bRuning = true;
    int  nImgSize = vpu_.nWidth*vpu_.nHeight*3/2;

    /*FILE *file = fopen("test.yuv", "wb+");
    fwrite(pSrc, 1, nImgSize, file);
    fclose(file);*/
    memcpy((char*)vpu_src_.stFrmDesc.virt_uaddr,
           pSrc, nImgSize);
    nRet = vpu_.enc_process(&vpu_src_.stFrmBuff, pDst, nDst);

    vpu_.bGetIFrame = false;
  }
  return nRet;
}




