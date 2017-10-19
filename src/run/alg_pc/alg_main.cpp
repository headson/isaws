#include <stdio.h>
#include <signal.h>

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"
#include "dispatcher/sync/dpclient_c.h"

#include "alg/clistenmessage.h"

int SignalHandle(int n_signal, short events, const void *p_usr_arg) {
  if (n_signal == SIGINT) {
    LOG(L_WARNING) << "revive SIGINT,End of the alg app";
  }
  else if (n_signal == SIGTERM) {
    LOG(L_WARNING) << "revive SIGTERM,End of the alg app";
  }
  else if (n_signal == SIGSEGV) {
    LOG(L_ERROR) << "revive SIGSEGV,End of the alg app";
  }
  else if (n_signal == SIGABRT) {
    LOG(L_ERROR) << "revive SIGSEGV,End of the alg app";
  }
#ifdef POSIX
  else if (n_signal == SIGPIPE) {
    LOG(L_INFO) << "revive SIGSEGV, alg app";
  }
#endif
  if (n_signal == SIGINT ||
      n_signal == SIGTERM ||
      n_signal == SIGSEGV ||
      n_signal == SIGABRT) {

    alg::CListenMessage::Instance()->Stop();
    LOG(L_WARNING) << "End of the alg app";
    exit(EXIT_SUCCESS);
  }
  return 0;
}

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif

  DpEvtService evt_service =
    (DpEvtService)vzbase::Thread::Current()->socketserver()->GetEvtService();

  Event_CreateSignalHandle((vzconn::EventService*)evt_service, SIGINT, SignalHandle, NULL);
  Event_CreateSignalHandle((vzconn::EventService*)evt_service, SIGTERM, SignalHandle, NULL);
  Event_CreateSignalHandle((vzconn::EventService*)evt_service, SIGSEGV, SignalHandle, NULL);
  Event_CreateSignalHandle((vzconn::EventService*)evt_service, SIGABRT, SignalHandle, NULL);
#ifdef POSIX
  Event_CreateSignalHandle((vzconn::EventService*)evt_service, SIGPIPE, SignalHandle, NULL);
#endif

  DpClient_Init(DEF_DP_SRV_IP, DEF_DP_SRV_PORT);
  Kvdb_Start(DEF_KVDB_SRV_IP, DEF_KVDB_SRV_PORT);

  if (!alg::CListenMessage::Instance()->Start()) {
    return -1;
  }

  while (true) {
    alg::CListenMessage::Instance()->RunLoop();
  }
  return 0;
}