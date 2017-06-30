#include "cacheserver/server/cachedserver.h"
#include "vzlogging/logging/vzlogging.h"

int main(int argc, char *argv[]) {

  InitVzLogging(argc, argv);
  ShowVzLoggingAlways();

  vzconn::EventService event_service;
  event_service.Start();

  cached::CachedServer cached_server(event_service);
  cached_server.StartCachedServer("0.0.0.0", 5499);

  while (1) {
    event_service.RunLoop();
  }

  cached_server.StopCachedServer();

  return 0;
}