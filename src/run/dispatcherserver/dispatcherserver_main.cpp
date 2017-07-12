#include <signal.h>

#include "vzbase/helper/stdafx.h"

#include "dispatcher/dpserver/dpserver.h"
#include "dispatcher/kvdbserver/kvdbserver.h"

void SignalHandle(int n_sig) {
  ExitVzLogging();
  LOG(L_ERROR) << "applet terminal.";
}

int main(int argc, char *argv[]) {
  signal(SIGINT,  SignalHandle);
  signal(SIGTERM, SignalHandle);
  //signal(SIGKILL, SignalHandle);

  InitVzLogging(argc, argv);
#ifdef WIN32
  ShowVzLoggingAlways();
#endif

  vzconn::EventService event_service;
  event_service.Start();

  dp::DpServer dpserver(event_service);
  dpserver.StartDpServer("0.0.0.0", DEF_DP_SRV_PORT);

  kvdb::KvdbServer kvdb_server(event_service);
  kvdb_server.StartKvdbServer("0.0.0.0", DEF_KVDB_SRV_PORT,
                              "./kvdb.db", "./kvdb_backup.db");

  while (1) {
    event_service.RunLoop();
  }

  dpserver.StopDpServer();
  kvdb_server.StopKvdbServer();
  return 0;
}