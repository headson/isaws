#include <stdio.h>
#include <signal.h>

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"
#include "dispatcher/sync/dpclient_c.h"

#include "alg_hs/clistenmessage.h"

void SignalHandle(void *usr_arg) {
  alg::CListenMessage::Instance()->Stop();
  exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif

  vzbase::Thread *main_thread = vzbase::Thread::Current();
  vzconn::EventService *event_service =
    (vzconn::EventService *)main_thread->socketserver()->GetEvtService();

  ExitSignalHandle(event_service, "dispatcher_server", SignalHandle, NULL);

  DpClient_Init(DEF_DP_SRV_IP, DEF_DP_SRV_PORT);
  Kvdb_Start(DEF_KVDB_SRV_IP, DEF_KVDB_SRV_PORT);

  bool res = alg::CListenMessage::Instance()->Start();

  while (res) {
    alg::CListenMessage::Instance()->RunLoop();
  }
  return 0;
}