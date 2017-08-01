#include <signal.h>

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "systemserver/clistenmessage.h"

void SignalHandle(int n_sig) {
  sys::CListenMessage::Instance()->Stop();

  ExitVzLogging();
  LOG(L_ERROR) << "applet terminal.";
}

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif

  DpClient_Init(DEF_DP_SRV_IP, DEF_DP_SRV_PORT);
  Kvdb_Start(DEF_KVDB_SRV_IP, DEF_KVDB_SRV_PORT);

  bool b_ret = sys::CListenMessage::Instance()->Start();
  while (b_ret) {
    sys::CListenMessage::Instance()->RunLoop();
  }

  return 0;
}