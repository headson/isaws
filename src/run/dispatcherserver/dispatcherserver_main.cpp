#include "dispatcher/dpserver/dpserver.h"
#include "dispatcher/kvdbserver/kvdbserver.h"
#include "vzlogging/logging/vzlogging.h"

int main(int argc, char *argv[]) {

  InitVzLogging(argc, argv);
#ifdef WIN32
  ShowVzLoggingAlways();
#endif

  vzconn::EventService event_service;
  event_service.Start();

  dp::DpServer dpserver(event_service);
  dpserver.StartDpServer("0.0.0.0", 5291);

  kvdb::KvdbServer kvdb_server(event_service);
  kvdb_server.StartKvdbServer("0.0.0.0", 5299,
                              "./kvdb.db",
                              "./kvdb_backup.db");

  while (1) {
    event_service.RunLoop(-1);
  }

  dpserver.StopDpServer();
  kvdb_server.StopKvdbServer();
  return 0;
}