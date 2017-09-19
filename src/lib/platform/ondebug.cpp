/************************************************************************
*Author      : Sober.Peng 17-07-27
*Description :
************************************************************************/
#include "cpcalgctrl.h"

#include "clistenmessage.h"

#include "systemv/shm/vzshm_c.h"
#include "vzbase/helper/stdafx.h"

void CAlgCtrl::AlgDebugCallback(sdk_iva_debug_info *pDebug) {
  int nsize = pDebug->debug_image_width*pDebug->debug_image_height*3/2;
  if (platform::CListenMessage::Instance()->GetVdoCatch()) {
    platform::CListenMessage::Instance()->GetVdoCatch()->EncUsrImage(
      pDebug->debug_image, nsize,
      pDebug->debug_image_width, pDebug->debug_image_height);
  }

  //LOG_INFO("write buffer. %d %d",
  //         pDebug->debug_image_width, pDebug->debug_image_height);
}
