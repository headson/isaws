#include <signal.h>

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"
#include "web_server/clistenmessage.h"

#include "dispatcher/sync/dpclient_c.h"

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif

  DpEvtService evt_service =
    (DpEvtService)vzbase::Thread::Current()->socketserver()->GetEvtService();

  unsigned int is_exit = 0;
  ExitSignalHandle((vzconn::EventService*)evt_service, &is_exit);

  DpClient_Init(DEF_DP_SRV_IP, DEF_DP_SRV_PORT);
  Kvdb_Start(DEF_KVDB_SRV_IP, DEF_KVDB_SRV_PORT);

  bool res = web::CListenMessage::Instance()->Start(
               DEF_WEB_SRV_PORT, DEF_WEB_SRV_PATH);
  if (false == res) {
    LOG(L_ERROR) << "start failed.";
    return -1;
  }
  while (1 == is_exit) {
    web::CListenMessage::Instance()->RunLoop();
  }

  return 0;
}
