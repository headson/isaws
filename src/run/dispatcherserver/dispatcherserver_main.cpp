#include "dispatcher/dpserver/dpserver.h"
#include "dispatcher/kvdbserver/kvdbserver.h"
#include "vzlogging/logging/vzlogging.h"

#include <signal.h>
//#include <execinfo.h>
#include "vzbase/helper/stdafx.h"
#include "dispatcher/sync/dpclient_c.h"
#include "vzbase/thread/thread.h"

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

//void dump(int signo) {
//  char **strings = NULL;
//  void *array[10] ={NULL};
//  size_t size = backtrace(array, 10);
//  strings = backtrace_symbols(array, size);
//  printf("Obtained %zd stack frames.\n", size);
//  for (size_t i = 0; i < size; i++) {
//    printf("%s\n", strings[i]);
//  }
//  free(strings);
//  exit(0);
//}

int main(int argc, char *argv[]) {
  //signal(SIGSEGV, &dump);

  InitVzLogging(argc, argv);
#ifdef WIN32
  ShowVzLoggingAlways();
#endif

  vzbase::Thread *main_thread = vzbase::Thread::Current();
  vzconn::EventService *event_service =
    (vzconn::EventService *)main_thread->socketserver()->GetEvtService();

//  Event_CreateSignalHandle(&event_service, SIGINT, SignalHandle, NULL);
//  Event_CreateSignalHandle(&event_service, SIGTERM, SignalHandle, NULL);
//  Event_CreateSignalHandle(&event_service, SIGSEGV, SignalHandle, NULL);
//  Event_CreateSignalHandle(&event_service, SIGABRT, SignalHandle, NULL);
//#ifdef POSIX
//  Event_CreateSignalHandle(&event_service, SIGPIPE, SignalHandle, NULL);
//#endif

  dp::DpServer dpserver(*event_service);
  dpserver.StartDpServer("0.0.0.0", 5291);


  kvdb::KvdbServer kvdb_server(main_thread);
#ifdef WIN32
  kvdb_server.StartKvdbServer("0.0.0.0", 5299, "c:\\tools\\kvdb.db",
                              "c:\\tools\\kvdb_backup.db");
#else
  kvdb_server.StartKvdbServer("0.0.0.0", 5299, "/mnt/usr/kvdb.db",
                              "/mnt/usr/kvdb_backup.db");
#endif

  while (true) {
    main_thread->ProcessMessages(4 * 1000);

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

  dpserver.StopDpServer();
  kvdb_server.StopKvdbServer();
  return 0;
}