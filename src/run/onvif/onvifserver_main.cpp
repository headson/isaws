/***************************************************************************************
 *
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *  By downloading, copying, installing or using the software you agree to this license.
 *  If you do not agree to this license, do not download, install,
 *  copy or use the software.
 *
 *  Copyright (C) 2010-2014, Happytimesoft Corporation, all rights reserved.
 *
 *  Redistribution and use in binary forms, with or without modification, are permitted.
 *
 *  Unless required by applicable law or agreed to in writing, software distributed
 *  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *  CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
 *  language governing permissions and limitations under the License.
 *
****************************************************************************************/

#ifdef WIN32
#include "onvif/cfgpro/stdafx.h"
#else
#include "sys_msg_drv.h"
#endif
#include "onvif/bm/sys_inc.h"
#include "onvif/onvif/onvif_api.h"
#include "dispatcher/sync/dpclient_c.h"
#include <signal.h>
#include "onvif/cfgpro/onvif_dispatcher.h"
#include "onvif/cfgpro/onvif_cfgfile_mng.h"
#include "cacheserver/client/cachedclient_c.h"
#include "onvif/cfgpro/onvif_pro_msg.h"
#include "vzbase/base/loggingsettings.h"

int main(int argc, char * argv[]) {
#if __WIN32_OS__
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
  vzbase::InitLoggingSystem(argc, argv);
  Kvdb_Start("127.0.0.1",5299);
  int ret = onvif_init_cfg();

  printf("onvif_init_cfg ret=%d\n",ret);

  if(OnvifDisp_Start()<0) {
    printf("OnvifDisp_Start error!!!! \n");
    return 0;
  }

  onvif_start();
  Cached_Start("127.0.0.1", 5320);

  DPPollHandle poll_handle = DpClient_CreatePollHandle(
                               OnDpMessage,
                               NULL,
                               OnDpState,
                               NULL);

  for (;;) {
    DpClient_PollDpMessage(poll_handle, 100);
    Onvif_Info* pOnvifInfo = GetOnvifInfo();
    if (pOnvifInfo->ip_chg_flag) {
      printf("ip_chg_flag!!!! \n");
      pOnvifInfo->ip_chg_flag = 0;
    }
  }

  onvif_stop();
  OnvifDisp_Stop();//DpClient_Stop();
  Cached_Stop();
  Kvdb_Stop();
  return 0;
}



