#include <signal.h>

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "platform/clistenmessage.h"

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);

  DpEvtService evt_service =
    (DpEvtService)vzbase::Thread::Current()->socketserver()->GetEvtService();

  unsigned int is_exit = false;
  ExitSignalHandle((vzconn::EventService*)evt_service, &is_exit);

  DpClient_Init(DEF_DP_SRV_IP, DEF_DP_SRV_PORT);
  Kvdb_Start(DEF_KVDB_SRV_IP, DEF_KVDB_SRV_PORT);

  bool res = platform::CListenMessage::Instance()->Start();
  if (false == res) {
    LOG(L_ERROR) << "start failed.";
    return -1;
  }

  while (1 == is_exit) {
    platform::CListenMessage::Instance()->RunLoop();
  }

  return 0;
}