#include "cacheserver/server/cachedserver.h"
#include "vzlogging/logging/vzlogging.h"

#include <signal.h>
#include "vzbase/helper/stdafx.h"
#include "dispatcher/sync/dpclient_c.h"

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif

  vzconn::EventService event_service;
  event_service.Start();

  unsigned int is_exit = 0;
  ExitSignalHandle(&event_service, "filecachedserver", &is_exit);

  cached::CachedServer cached_server(event_service);
  cached_server.StartCachedServer("0.0.0.0", 5320);

  while (0 == is_exit) {
    event_service.RunLoop(4 * 1000);

    static void *hdl_watchdog = NULL;
    if (hdl_watchdog == NULL) {
      hdl_watchdog = RegisterWatchDogKey(
                       "MAIN", 4, DEF_WATCHDOG_TIMEOUT);
    }

    static time_t old_time = time(NULL);
    time_t now_time = time(NULL);
    if (abs(now_time - old_time) >= DEF_FEEDDOG_TIME) {
      old_time = now_time;
      if (hdl_watchdog) {
        FeedDog(hdl_watchdog);
      }
    }
  }

  cached_server.StopCachedServer();

  return 0;
}