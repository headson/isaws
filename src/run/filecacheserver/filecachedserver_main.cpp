#include "cacheserver/server/cachedserver.h"
#include "vzlogging/logging/vzlogging.h"

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif

  vzconn::EventService event_service;
  event_service.Start();

  cached::CachedServer cached_server(event_service);
  cached_server.StartCachedServer("0.0.0.0", 5320);

  while (1) {
    event_service.RunLoop(-1);
  }

  cached_server.StopCachedServer();

  return 0;
}