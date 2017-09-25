#include <signal.h>

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "systemserver/clistenmessage.h"

int SignalHandle(int n_signal, short events, const void *p_usr_arg) {
  if (n_signal == SIGINT) {
    LOG(L_WARNING) << "revive SIGINT,End of the sysyem server";
  } else if (n_signal == SIGTERM) {
    LOG(L_WARNING) << "revive SIGTERM,End of the sysyem server";
  } else if (n_signal == SIGSEGV) {
    LOG(L_ERROR) << "revive SIGSEGV,End of the sysyem server";
  } else if (n_signal == SIGABRT) {
    LOG(L_ERROR) << "revive SIGSEGV,End of the sysyem server";
  }
#ifdef POSIX
  else if (n_signal == SIGPIPE) {
    LOG(L_INFO) << "revive SIGSEGV, sysyem server";
  }
#endif
  if (n_signal == SIGINT ||
      n_signal == SIGTERM ||
      n_signal == SIGSEGV ||
      n_signal == SIGABRT) {

    sys::CListenMessage::Instance()->Stop();
    LOG(L_WARNING) << "End of the sysyem server";
    exit(EXIT_SUCCESS);
  }
  return 0;
}

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif

  DpEvtService evt_service =
    (DpEvtService)vzbase::Thread::Current()->socketserver()->GetEvtService();
  
  unsigned int is_exit = false;
  ExitSignalHandle((vzconn::EventService*)evt_service, &is_exit);

  DpClient_Init(DEF_DP_SRV_IP, DEF_DP_SRV_PORT);
  Kvdb_Start(DEF_KVDB_SRV_IP, DEF_KVDB_SRV_PORT);

  bool res = sys::CListenMessage::Instance()->Start();
  if (false == res) {
    LOG(L_ERROR) << "start failed.";
    return -1;
  }
  while (1 == is_exit) {
    sys::CListenMessage::Instance()->RunLoop();
  }

  return 0;
}