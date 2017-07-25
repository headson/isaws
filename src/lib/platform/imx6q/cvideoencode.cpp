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

#include "systemv/shm/vzshm_c.h"

CVideoEncode::CVideoEncode()
  : v4l2_()
  , vpu_() {
}

CVideoEncode::~CVideoEncode() {
  Stop();
}

bool CVideoEncode::Start(const char* s_key, unsigned int n_size) {
  bool b_ret = false;

  if (vpu_.enc_fd == 0) {
    b_ret = vpu_.vdo_restart();
    if (!b_ret) {
      LOG_ERROR("video start failed.\n");
      return false;
    }
  }

  b_ret = shm_vdo_.Create(s_key, n_size);
  if (!b_ret) {
    LOG_ERROR("create share video failed.");
    return false;
  }
  
  return true;
}

void CVideoEncode::Stop() {
  vpu_.vdo_stop();  // 关闭VPU
}

void CVideoEncode::Process() {
  int nRet = 0;
  int nScale = 1;
  unsigned int nTmOld = 0, nColor = 0;
  if (vpu_.nWidth==720)
    nScale = 2;
  else if(vpu_.nWidth==1280)
    nScale = 3;
  else if(vpu_.nWidth>=1600)
    nScale = 4;

  // start capture
  struct v4l2_buffer v4l_buf;
  int nFrmSeqs=0, nSeqs=0;
  FrameBuffer* pSrcFrm = NULL;                        // 帧源数据
  unsigned int nImgSize = vpu_.nWidth*vpu_.nHeight;   // 照片大小

  CShareVideo::ShmVdo *p_shm_vdo = shm_vdo_.GetData();
  if (p_shm_vdo != NULL) {
    memcpy(p_shm_vdo->sps_pps, 
           vpu_.head_, vpu_.sps_size_+vpu_.pps_size_);
    p_shm_vdo->n_sps = vpu_.sps_size_;
    p_shm_vdo->n_pps = vpu_.pps_size_;

    p_shm_vdo->n_width  = vpu_.nWidth;
    p_shm_vdo->n_height = vpu_.nHeight;
  }

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
  while (p_shm_vdo) {
    nRet = v4l2_.v4l_get_capture_data(&v4l_buf);
    if (nRet != 0) {
      LOG_ERROR("video %s restart result %d.", v4l2_.sVideo.c_str(), nRet);
      goto ERROR_;
    }
    // 视频处理
    //unsigned int nTmNow     = vdt.tsec();
    //if (nTmOld != nTmNow) {
    //  nTmOld = nTmNow;
    //  nColor = nColor ? 0 : 255;
    //}
    //yuv_osd(nColor, (unsigned char*)m_cV4l2.sBuffer[v4l_buf.index].start,
    //        m_cVpu.nWidth, m_cVpu.nHeight, (char*)vdt.to_string().c_str(), nScale, asc8, 10, 10);

    // 编码
    pSrcFrm->myIndex = vpu_.nFrmNums-1+v4l_buf.index;
    pSrcFrm->bufY    = v4l2_.sBuffer[v4l_buf.index].offset;
    pSrcFrm->bufCb   = pSrcFrm->bufY  + nImgSize;
    pSrcFrm->bufCr   = pSrcFrm->bufCb + (nImgSize >> 2);
    pSrcFrm->strideY = vpu_.nWidth;
    pSrcFrm->strideC = vpu_.nHeight >> 1;
    nRet = vpu_.enc_process(pSrcFrm,
                            (char*)p_shm_vdo->p_data,
                            p_shm_vdo->n_size);
    if (nRet) {
      vpu_.bGetIFrame = false;

      struct timeval tv;
      gettimeofday(&tv, NULL);
      p_shm_vdo->n_data = nRet;
      p_shm_vdo->n_w_sec = tv.tv_sec;
      p_shm_vdo->n_w_usec = tv.tv_usec;
    }
    v4l2_.v4l_put_capture_data(&v4l_buf);
  }

ERROR_:
  // 关闭V4L2
  v4l2_.v4l_stop_capturing();
  v4l2_.v4l_close();
}

