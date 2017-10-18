#include <signal.h>

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"
#include "web_server/clistenmessage.h"

#include "dispatcher/sync/dpclient_c.h"

void SignalHandle(void *usr_arg) {
  web::CListenMessage::Instance()->Stop();
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif

  vzconn::EventService *evt_service =
    (vzconn::EventService*)vzbase::Thread::Current()->socketserver()->GetEvtService();

  ExitSignalHandle(evt_service, "dispatcher_server", SignalHandle, NULL);

  DpClient_Init(DEF_DP_SRV_IP, DEF_DP_SRV_PORT);
  Kvdb_Start(DEF_KVDB_SRV_IP, DEF_KVDB_SRV_PORT);

  bool res = web::CListenMessage::Instance()->Start(
               DEF_WEB_SRV_PORT, DEF_WEB_SRV_PATH);
  while (res) {
    web::CListenMessage::Instance()->RunLoop();
  }

  return 0;
}
