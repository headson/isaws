#include "dispatcher/dpserver/dpserver.h"
#include "dispatcher/kvdbserver/kvdbserver.h"
#include "vzlogging/logging/vzlogging.h"

#include <signal.h>
//#include <execinfo.h>
#include "vzbase/helper/stdafx.h"
#include "dispatcher/sync/dpclient_c.h"
#include "vzbase/thread/thread.h"

void SignalHandle(void *usr_arg) {
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
#ifdef WIN32
  ShowVzLoggingAlways();
#endif
  
  vzbase::Thread *main_thread = vzbase::Thread::Current();
  vzconn::EventService *event_service =
    (vzconn::EventService *)main_thread->socketserver()->GetEvtService();

  ExitSignalHandle(event_service, "dispatcher_server", SignalHandle, NULL);

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

  kvdb::KvdbServer skvdb_server(main_thread);
#ifdef WIN32
  skvdb_server.StartKvdbServer("0.0.0.0", 5499,
                               "c:\\tools\\secret_kvdb.db",
                               "c:\\tools\\secret_kvdb_backup.db");
#else
  skvdb_server.StartKvdbServer("0.0.0.0", 5499,
                               "/mnt/usr/secret_kvdb.db",
                               "/mnt/usr/secret_kvdb_backup.db");
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
  skvdb_server.StopKvdbServer();
  return 0;
}