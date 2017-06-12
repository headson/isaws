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

void CPlatform::Initinal() {
  RetCode ret;
  vpu_versioninfo ver;
  ret = vpu_Init(NULL);
  if (ret) {
    printf("VPU Init Failure.");
    return;
  }

  ret = vpu_GetVersionInfo(&ver);
  if (ret) {
    printf("Cannot get version info, err:%d", ret);
    vpu_UnInit();
    return;
  }

  printf("VPU firmware version: %d.%d.%d_r%d.\n", ver.fw_major, ver.fw_minor, ver.fw_release, ver.fw_code);
  printf("VPU library version: %d.%d.%d.\n", ver.lib_major, ver.lib_minor, ver.lib_release);
}

void CPlatform::Release() {
  vpu_UnInit();
}
