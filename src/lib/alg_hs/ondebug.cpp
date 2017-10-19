/************************************************************************
*Author      : Sober.Peng 17-07-27
*Description :
************************************************************************/
#include "clistenmessage.h"

#include "vzbase/helper/stdafx.h"

void alg::CListenMessage::AlgDebugCallback(IVA_DEBUG_OUTPUT *pDebug) {
  if (!pDebug || !pDebug->debug_img) {
    LOG(L_ERROR) << "param is null.";
    return;
  }

#ifdef _LINUX
  if (alg::CListenMessage::Instance()->vdo_codec_) {
    alg::CListenMessage::Instance()->vdo_codec_->EncodeFrame(
      (char*)pDebug->debug_img, pDebug->img_w, pDebug->img_h,
      alg::CListenMessage::video_callback, (void*)alg::CListenMessage::Instance());
  }
#endif
  //LOG_INFO("write buffer. %d %d",
  //         pDebug->debug_image_width, pDebug->debug_image_height);
}

#ifdef _LINUX
int alg::CListenMessage::OnVideo(VencOutputBuffer *output_buffer) {
  if (shm_vdo_ptr_) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    shm_vdo_ptr_->wsec = tv.tv_sec;
    shm_vdo_ptr_->wusec = tv.tv_usec;

    memcpy(shm_vdo_ptr_->pdata,
           output_buffer->pData0, output_buffer->nSize0);
    shm_vdo_ptr_->ndata = output_buffer->nSize0;
    if (output_buffer->nSize1) {
      memcpy(shm_vdo_ptr_->pdata + shm_vdo_ptr_->ndata,
             output_buffer->pData1, output_buffer->nSize1);
      shm_vdo_ptr_->ndata += output_buffer->nSize0;
    }
    return shm_vdo_ptr_->ndata;
  }
  return 0;
}
#endif