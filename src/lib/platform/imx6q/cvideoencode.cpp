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

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/timeutils.h"

#include "systemv/shm/vzshm_c.h"

CVideoEncode::CVideoEncode()
  : v4l2_()
  , vpu_() {
}

CVideoEncode::~CVideoEncode() {
  Stop();
}

bool CVideoEncode::Start(const char* s_vdo_key, unsigned int n_vdo_size,
                         const char *s_img_key, unsigned int n_img_size) {
  bool b_ret = false;

  if (vpu_.enc_fd == 0) {
    b_ret = vpu_.vdo_restart();
    if (!b_ret) {
      LOG_ERROR("video start failed.\n");
      return false;
    }
  }

  b_ret = shm_vdo_.Create(s_vdo_key, n_vdo_size);
  if (!b_ret) {
    LOG_ERROR("create share video failed.");
    return false;
  }

  b_ret = shm_img_.Create(s_img_key, n_img_size);
  if (!b_ret) {
    LOG_ERROR("create share image failed.");
    return false;
  }
  return true;
}

void CVideoEncode::Stop() {
  vpu_.vdo_stop();  // 关闭VPU
}

void CVideoEncode::SetViSize(unsigned int nWidth, unsigned int nHeight) {
  v4l2_.nWidth = nWidth;
  v4l2_.nHeight = nHeight;
}

void CVideoEncode::Run(vzbase::Thread* thread) {
  int nRet = 0;
  int nScale = 1;
  time_t nTmOld = 0;
  unsigned int nColor = 0;
  if (vpu_.nWidth==720)
    nScale = 2;
  else if(vpu_.nWidth==1280)
    nScale = 3;
  else if(vpu_.nWidth>=1600)
    nScale = 4;

  int nseqs = 0;

  // start capture
  struct v4l2_buffer v4l_buf;
  FrameBuffer* pSrcFrm = NULL;                        // 帧源数据
  unsigned int nImgSize = vpu_.nWidth*vpu_.nHeight;   // 照片大小

  shm_vdo_.WriteSps(vpu_.head_, vpu_.sps_size_);
  shm_vdo_.WritePps(vpu_.head_+vpu_.sps_size_, vpu_.pps_size_);

  shm_vdo_.SetWidth(vpu_.nWidth);
  shm_vdo_.SetHeight(vpu_.nHeight);

  shm_img_.SetWidth(v4l2_.nWidth);
  shm_img_.SetHeight(v4l2_.nHeight);

REOPEN:
  // 打开V4L2
  nRet = v4l2_.v4l_open();
  if (nRet != 0) {
    LOG_ERROR("open v4l2 %s failed.", v4l2_.sVideo.c_str());
    goto ERROR_;
  }
  nRet = v4l2_.v4l_start_capturing();
  if (nRet != 0) {
    LOG_ERROR("capture v4l2 %s failed.", v4l2_.sVideo.c_str());
    goto ERROR_;
  }
  printf("%s[%d].\n", __FUNCTION__, __LINE__);
  pSrcFrm = &vpu_.pFrmBufs[vpu_.nFrmNums-1];

  // 采集编码
  while (true) {
    nRet = v4l2_.v4l_get_capture_data(&v4l_buf);
    if (nRet != 0) {
      LOG_ERROR("video %s restart result %d.", v4l2_.sVideo.c_str(), nRet);
      goto ERROR_;
    }

    // share image
    struct timeval tv;
    gettimeofday(&tv, NULL);
    shm_img_.Write((char*)v4l2_.sBuffer[v4l_buf.index].start,
                   v4l2_.nWidth * v4l2_.nHeight * 3 / 2, 
                   tv.tv_usec, tv.tv_usec);

    // add time osd
    time_t nTmNow = time(NULL);
    if (nTmOld != nTmNow) {
      nTmOld = nTmNow;
      nColor = nColor ? 0 : 255;
    }
    yuv_osd(nColor, (unsigned char*)v4l2_.sBuffer[v4l_buf.index].start,
            vpu_.nWidth, vpu_.nHeight,
            (char*)vzbase::SecToString(nTmNow).c_str(),
            nScale, asc8, 10, 10);

    // resize image
    if ((v4l2_.nWidth != vpu_.nWidth) || (v4l2_.nHeight != vpu_.nHeight)) {
      ImageResizeNN((uint8*)v4l2_.sBuffer[v4l_buf.index].start,
                    v4l2_.nWidth, v4l2_.nHeight,
                    vpu_.nWidth, vpu_.nHeight);
    }

    // encode
    pSrcFrm->myIndex = vpu_.nFrmNums-1+v4l_buf.index;
    pSrcFrm->bufY    = v4l2_.sBuffer[v4l_buf.index].offset;
    pSrcFrm->bufCb   = pSrcFrm->bufY  + nImgSize;
    pSrcFrm->bufCr   = pSrcFrm->bufCb + (nImgSize >> 2);
    pSrcFrm->strideY = vpu_.nWidth;
    pSrcFrm->strideC = vpu_.nHeight >> 1;
    nRet = vpu_.enc_h264(pSrcFrm, &shm_vdo_);
    if ((nseqs++ % 10) == 1) {
      vpu_.bGetIFrame = true;
    } else {
      vpu_.bGetIFrame = false;
    }
    v4l2_.v4l_put_capture_data(&v4l_buf);
  }

ERROR_:
  // 关闭V4L2
  v4l2_.v4l_stop_capturing();
  v4l2_.v4l_close();
}

