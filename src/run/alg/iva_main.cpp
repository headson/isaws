#include <stdio.h>

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"
#include "dispatcher/sync/dpclient_c.h"

#include "iva/clistenmessage.h"

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);

  DpClient_Init(DEF_DP_SRV_IP,
                DEF_DP_SRV_PORT);
  Kvdb_Start(DEF_KVDB_SRV_IP, 
             DEF_KVDB_SRV_PORT);

  if (!imx6q::CListenMessage::Instance()->Start()) {
    return -1;
  }

  while (true) {
    imx6q::CListenMessage::Instance()->RunLoop();
  }
  return 0;
}