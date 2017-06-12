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

#include "inc/vdefine.h"

#include "asc8.h"
#include "yuv420.h"

#include "shm/vshmvideo.h"

CVideoEncode::CVideoEncode()
  : m_pPkgBuffer(NULL) 
  , m_cV4l2()
  , m_cVpu() {
}

CVideoEncode::~CVideoEncode() {
  Stop();
}

int32_t CVideoEncode::Start() {
  int32_t nRet = 0;

  if (m_cVpu.enc_fd == 0) {
    nRet = m_cVpu.vdo_restart();
    if (nRet != 0) {
      LOG_ERROR("CFslVideoEncode::init(): failed %d.\n", nRet);
      return nRet;
    }
  }
  return nRet;
}

void CVideoEncode::Stop() {
  m_cVpu.vdo_stop();  // 关闭VPU
}

void CVideoEncode::Process() {
  int32_t nRet = 0;
  int32_t nScale = 1;
  uint32_t nTmOld = 0, nColor = 0;
  if (m_cVpu.nWidth==720)      nScale = 2;
  else if(m_cVpu.nWidth==1280) nScale = 3;
  else if(m_cVpu.nWidth>=1600) nScale = 4;

  // start capture
  struct v4l2_buffer v4l_buf;
  int32_t nFrmSeqs=0, nSeqs=0;
  FrameBuffer* pSrcFrm = NULL;                        // 帧源数据
  uint32_t nImgSize = m_cVpu.nWidth*m_cVpu.nHeight;   // 照片大小
  if (!m_pPkgBuffer) {
    m_pPkgBuffer = new int8_t[nImgSize];
  }
  bool    b_runing = true;
  VShmVideo v_shm_video;
  v_shm_video.Open(sizeof(TAG_SHM_VIDEO));

  v_shm_video.Signal();
  printf("%s[%d].\n", __FUNCTION__, __LINE__);

REOPEN:
  // 打开V4L2
  nRet = m_cV4l2.v4l_open();
  if (nRet != RET_SUCCESS) {
    LOG_ERROR("################## CFslVideoEncode::Process: open v4l2 %s failed.", m_cV4l2.sVideo.c_str());
    goto ERROR_;
  }
  nRet = m_cV4l2.v4l_start_capturing();
  if (nRet != RET_SUCCESS) {
    LOG_ERROR("################## CFslVideoEncode::Process: capture v4l2 %s failed.", m_cV4l2.sVideo.c_str());
    goto ERROR_;
  }
  printf("%s[%d].\n", __FUNCTION__, __LINE__);
  pSrcFrm = &m_cVpu.pFrmBufs[m_cVpu.nFrmNums-1];

  // 采集编码
  while (b_runing) {
    nRet = m_cV4l2.v4l_get_capture_data(&v4l_buf);
    if (nRet != RET_SUCCESS) {
      LOG_ERROR("video %s restart result %d.", m_cV4l2.sVideo.c_str(), nRet);

      goto ERROR_;
    }
    // 视频处理
    //uint32_t nTmNow     = vdt.tsec();
    //if (nTmOld != nTmNow) {
    //  nTmOld = nTmNow;
    //  nColor = nColor ? 0 : 255;
    //}
    //yuv_osd(nColor, (uint8_t*)m_cV4l2.sBuffer[v4l_buf.index].start,
    //        m_cVpu.nWidth, m_cVpu.nHeight, (char*)vdt.to_string().c_str(), nScale, asc8, 10, 10);

    // 编码
    pSrcFrm->myIndex = m_cVpu.nFrmNums-1+v4l_buf.index;
    pSrcFrm->bufY    = m_cV4l2.sBuffer[v4l_buf.index].offset;
    pSrcFrm->bufCb   = pSrcFrm->bufY  + nImgSize;
    pSrcFrm->bufCr   = pSrcFrm->bufCb + (nImgSize >> 2);
    pSrcFrm->strideY = m_cVpu.nWidth;
    pSrcFrm->strideC = m_cVpu.nHeight >> 1;
    nRet = m_cVpu.enc_process(pSrcFrm, m_pPkgBuffer, nImgSize*3/2, b_runing);
    if (nRet) {
      m_cVpu.bGetIFrame = false;

      struct timeval tv;
      gettimeofday(&tv, NULL);
      v_shm_video.Write(m_pPkgBuffer, nRet, &tv);
    }
    m_cV4l2.v4l_put_capture_data(&v4l_buf);
  }
  printf("%s[%d].\n", __FUNCTION__, __LINE__);
ERROR_:
  if (m_pPkgBuffer) {
    delete[] m_pPkgBuffer;
    m_pPkgBuffer = NULL;
  }
// 关闭V4L2
  m_cV4l2.v4l_stop_capturing();
  m_cV4l2.v4l_close();
}

