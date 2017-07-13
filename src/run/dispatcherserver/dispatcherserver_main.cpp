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
#ifdef WIN32
  kvdb_server.StartKvdbServer("0.0.0.0", 5299,
                              "./kvdb.db",
                              "./kvdb_backup.db");
#else
  kvdb_server.StartKvdbServer("0.0.0.0", 5299,
                              "/mnt/usr/kvdb.db",
                              "/mnt/usr/kvdb_backup.db");
#endif

  kvdb::KvdbServer skvdb_server(event_service);
#ifdef WIN32
  skvdb_server.StartKvdbServer("0.0.0.0", 5499,
                              "./secret_kvdb.db",
                              "./secret_kvdb_backup.db");
#else
  skvdb_server.StartKvdbServer("0.0.0.0", 5499,
                               "/mnt/usr/secret_kvdb.db",
                               "/mnt/usr/secret_kvdb_backup.db");
#endif

  while (1) {
    event_service.RunLoop();
  }

  dpserver.StopDpServer();
  kvdb_server.StopKvdbServer();

  return 0;
}