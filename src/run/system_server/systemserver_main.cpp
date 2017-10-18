#include <signal.h>

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "systemserver/clistenmessage.h"

void SignalHandle(void *usr_arg) {
  sys::CListenMessage::Instance()->Stop();
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif

  vzconn::EventService *evt_service =
    (vzconn::EventService *)vzbase::Thread::Current()->socketserver()->GetEvtService();

  ExitSignalHandle(evt_service, "platform_app", SignalHandle, NULL);
  
  DpClient_Init(DEF_DP_SRV_IP, DEF_DP_SRV_PORT);
  Kvdb_Start(DEF_KVDB_SRV_IP, DEF_KVDB_SRV_PORT);

  bool res = sys::CListenMessage::Instance()->Start();
  while (res) {
    sys::CListenMessage::Instance()->RunLoop();
  }

  return 0;
}