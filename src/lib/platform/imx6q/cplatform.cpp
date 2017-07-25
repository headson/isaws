/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : cplatform.cpp
* Author        : Sober.Peng
* Date          : 7:2:2017
* Description   :
*-----------------------------------------------------------------------------
* Modify        :
*-----------------------------------------------------------------------------
******************************************************************************/
#include "cplatform.h"
#include "cvideoencode.h"
#include "vzlogging/logging/vzlogging.h"

void CPlatform::Initinal() {
  RetCode ret;
  vpu_versioninfo ver;
  ret = vpu_Init(NULL);
  if (ret) {
    LOG_ERROR("VPU Init Failure.");
    return;
  }

  ret = vpu_GetVersionInfo(&ver);
  if (ret) {
    LOG_ERROR("Cannot get version info, err:%d", ret);
    vpu_UnInit();
    return;
  }

  LOG_INFO("VPU firmware version: %d.%d.%d_r%d.\n",
         ver.fw_major, ver.fw_minor, ver.fw_release, ver.fw_code);
  LOG_INFO("VPU library version: %d.%d.%d.\n", 
         ver.lib_major, ver.lib_minor, ver.lib_release);
}

void CPlatform::Release() {
  vpu_UnInit();
}
