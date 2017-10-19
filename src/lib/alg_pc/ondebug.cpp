/************************************************************************
*Author      : Sober.Peng 17-07-27
*Description :
************************************************************************/
#include "clistenmessage.h"

#include "systemv/shm/vzshm_c.h"
#include "vzbase/helper/stdafx.h"

void alg::CListenMessage::AlgDebugCallback(sdk_iva_debug_info *pDebug) {
  int nsize = pDebug->debug_image_width*pDebug->debug_image_height*3/2;
  if (nsize > SHM_IMAGE_1_SIZE) {
    return;
  }

  struct timeval tv;
  gettimeofday(&tv, NULL);
  CListenMessage::Instance()->shm_dbg_image_.Write(
    (char*)pDebug->debug_image, nsize, tv.tv_sec, tv.tv_usec);

  //LOG_INFO("write buffer. %d %d", 
  //         pDebug->debug_image_width, pDebug->debug_image_height);
}
