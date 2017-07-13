#include <signal.h>

#include "vzbase/helper/stdafx.h"
#include "vzbase/helper/vmessage.h"
#include "web_server/clistenmessage.h"

void SignalHandle(int n_sig) {
  web::CListenMessage::Instance()->Stop();

  ExitVzLogging();
  LOG(L_ERROR) << "applet terminal.";
}

int main(int argc, char *argv[]) {
  signal(SIGINT,  SignalHandle);
  signal(SIGTERM, SignalHandle);
  //signal(SIGKILL, SignalHandle);

  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif

  bool b_ret = web::CListenMessage::Instance()->Start(
                 DEF_DP_SRV_IP, DEF_DP_SRV_PORT,
                 DEF_WEB_SRV_PORT, DEF_WEB_SRV_PATH);
  while (b_ret) {
    web::CListenMessage::Instance()->RunLoop();
  }

  return 0;
}
