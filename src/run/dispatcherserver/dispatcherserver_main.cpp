#include "dispatcher/dpserver/dpserver.h"
#include "dispatcher/kvdbserver/kvdbserver.h"
#include "vzlogging/logging/vzlogging.h"

#include <signal.h>
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


int main(int argc, char *argv[]) {

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
  kvdb_server.StartKvdbServer("0.0.0.0", 5299, "./kvdb.db",
                              "./kvdb_backup.db");
#else
  kvdb_server.StartKvdbServer("0.0.0.0", 5299, "/mnt/usr/kvdb.db",
                              "/mnt/usr/kvdb_backup.db");
#endif

  kvdb::KvdbServer skvdb_server(main_thread);
#ifdef WIN32
  skvdb_server.StartKvdbServer("0.0.0.0", 5499,
                               "./secret_kvdb.db",
                               "./secret_kvdb_backup.db");
#else
  skvdb_server.StartKvdbServer("0.0.0.0", 5499,
                               "/mnt/usr/secret_kvdb.db",
                               "/mnt/usr/secret_kvdb_backup.db");
#endif
  while (true) {
    main_thread->ProcessMessages(4 * 1000);

    static void *watchdog = NULL;
    if (watchdog == NULL) {
      watchdog = RegisterWatchDogKey("MAIN", 4, 21);
    }
    if (watchdog) {
      FeedDog(watchdog);
    }
  }

  dpserver.StopDpServer();
  kvdb_server.StopKvdbServer();
  skvdb_server.StopKvdbServer();
  return 0;
}