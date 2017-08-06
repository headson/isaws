#include <stdio.h>

#include "vzbase/helper/stdafx.h"

#include "vzbase/base/vmessage.h"

#include "dispatcher/sync/dpclient_c.h"

#include "platform/imx6q/cplatform.h"
#include "platform/imx6q/clistenmessage.h"

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);

  CPlatform::Initinal();

  DpClient_Init(DEF_DP_SRV_IP, DEF_DP_SRV_PORT);
  Kvdb_Start(DEF_KVDB_SRV_IP,  DEF_KVDB_SRV_PORT);

  if (argc == 2) {
    if (!imx6q::CListenMessage::Instance()->Start(argv[1])) {
      return -1;
    }
  } else {
    if (!imx6q::CListenMessage::Instance()->Start("/dev/video0")) {
      return -1;
    }
  }

  while (true) {
    imx6q::CListenMessage::Instance()->RunLoop();
  }

  CPlatform::Release();
  return 0;
}