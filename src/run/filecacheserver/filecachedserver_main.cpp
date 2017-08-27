#include "cacheserver/server/cachedserver.h"
#include "vzlogging/logging/vzlogging.h"

#include <signal.h>
#include "vzbase/helper/stdafx.h"
#include "dispatcher/sync/dpclient_c.h"

int SignalHandle(int n_signal, short events, const void *p_usr_arg) {
  if (n_signal == SIGINT) {
    LOG(L_WARNING) << "revive SIGINT,End of the dispatcher server";
  } else if (n_signal == SIGTERM) {
    LOG(L_WARNING) << "revive SIGTERM,End of the dispatcher server";
  } else if (n_signal == SIGSEGV) {
    LOG(L_ERROR) << "revive SIGSEGV,End of the dispatcher server";
  } else if (n_signal == SIGABRT) {
    LOG(L_ERROR) << "revive SIGSEGV,End of the dispatcher server";
  }
#ifdef POSIX
  else if (n_signal == SIGPIPE) {
    LOG(L_INFO) << "revive SIGSEGV, dispatcher server";
  }
#endif
  if (n_signal == SIGINT ||
      n_signal == SIGTERM ||
      n_signal == SIGSEGV ||
      n_signal == SIGABRT) {
    LOG(L_WARNING) << "End of the dispatcher server";
    exit(EXIT_SUCCESS);
  }
  return 0;
}

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif

  vzconn::EventService event_service;
  event_service.Start();

//  Event_CreateSignalHandle(&event_service, SIGINT, SignalHandle, NULL);
//  Event_CreateSignalHandle(&event_service, SIGTERM, SignalHandle, NULL);
//  Event_CreateSignalHandle(&event_service, SIGSEGV, SignalHandle, NULL);
//  Event_CreateSignalHandle(&event_service, SIGABRT, SignalHandle, NULL);
//#ifdef POSIX
//  Event_CreateSignalHandle(&event_service, SIGPIPE, SignalHandle, NULL);
//#endif

  cached::CachedServer cached_server(event_service);
  cached_server.StartCachedServer("0.0.0.0", 5320);

  while (1) {
    event_service.RunLoop(4 * 1000);

    static void *watchdog = NULL;
    if (watchdog == NULL) {
      watchdog = RegisterWatchDogKey("MAIN", 4, 21);
    }
    if (watchdog) {
      FeedDog(watchdog);
    }
  }

  cached_server.StopCachedServer();

  return 0;
}