#include <stdio.h>
#include <signal.h>

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"
#include "dispatcher/sync/dpclient_c.h"

#include "business/clistenmessage.h"

void SignalHandle(void *usr_arg) {
  bs::CListenMessage::Instance()->Stop();
  exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif

  vzconn::EventService *evt_service =
    (vzconn::EventService *)vzbase::Thread::Current()->socketserver()->GetEvtService();

  ExitSignalHandle(evt_service, "platform_app", SignalHandle, NULL);

  DpClient_Init(DEF_DP_SRV_IP, DEF_DP_SRV_PORT);
  Kvdb_Start(DEF_KVDB_SRV_IP, DEF_KVDB_SRV_PORT);

  bool res = bs::CListenMessage::Instance()->Start(DB_PCOUNT_FILEPATH);

  while (res) {
    bs::CListenMessage::Instance()->RunLoop();
  }
  return 0;
}