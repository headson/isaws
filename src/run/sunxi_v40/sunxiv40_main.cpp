#include <stdio.h>

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "dev_sunxi_v40/clistenmessage.h"

void SignalHandle(void *usr_arg) {
  platform::CListenMessage::Instance()->Stop();
  exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);

  vzbase::Thread *main_thread = vzbase::Thread::Current();
  vzconn::EventService *event_service =
    (vzconn::EventService *)main_thread->socketserver()->GetEvtService();

  ExitSignalHandle(event_service, "platform_app", SignalHandle, NULL);

  DpClient_Init(DEF_DP_SRV_IP, DEF_DP_SRV_PORT);
  Kvdb_Start(DEF_KVDB_SRV_IP, DEF_KVDB_SRV_PORT);

  bool res = platform::CListenMessage::Instance()->Start();

  while (res) {
    try {
      platform::CListenMessage::Instance()->RunLoop();
    }
    catch (...) {

    }
  }
  
  return 0;
}