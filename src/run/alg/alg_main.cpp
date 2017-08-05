#include <stdio.h>

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"
#include "dispatcher/sync/dpclient_c.h"

#include "alg/clistenmessage.h"

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif

  DpClient_Init(DEF_DP_SRV_IP,
                DEF_DP_SRV_PORT);
  Kvdb_Start(DEF_KVDB_SRV_IP, 
             DEF_KVDB_SRV_PORT);

  if (!alg::CListenMessage::Instance()->Start()) {
    return -1;
  }

  while (true) {
    alg::CListenMessage::Instance()->RunLoop();
  }
  return 0;
}