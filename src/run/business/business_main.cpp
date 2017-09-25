#include <stdio.h>
#include <signal.h>

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"
#include "dispatcher/sync/dpclient_c.h"

#include "business/clistenmessage.h"

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif

  DpEvtService evt_service =
    (DpEvtService)vzbase::Thread::Current()->socketserver()->GetEvtService();

  unsigned int is_exit = false;
  ExitSignalHandle((vzconn::EventService*)evt_service, &is_exit);

  DpClient_Init(DEF_DP_SRV_IP, DEF_DP_SRV_PORT);
  Kvdb_Start(DEF_KVDB_SRV_IP, DEF_KVDB_SRV_PORT);

  if (!bs::CListenMessage::Instance()->Start(DB_PCOUNT_FILEPATH)) {
    return -1;
  }

  while (1 == is_exit) {
    bs::CListenMessage::Instance()->RunLoop();
  }
  return 0;
}