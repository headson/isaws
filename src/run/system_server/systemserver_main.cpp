#include <signal.h>

#include "vzbase/helper/stdafx.h"

#include "systemserver/clistenmessage.h"

void SignalHandle(int n_sig) {
  CListenMessage::Instance()->Stop();

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

  bool b_ret = CListenMessage::Instance()->Start(
                 (uint8*)DEF_DP_SRV_IP, DEF_DP_SRV_PORT);
  while (b_ret) {
    CListenMessage::Instance()->RunLoop();
  }

  return 0;
}