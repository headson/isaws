#include <signal.h>

#include "vzbase/helper/stdafx.h"
#include "web_for_dp/clistenmessage.h"

int SignalHandle(int n_signal, short events, const void *p_usr_arg) {
  web::CListenMessage::Instance()->Stop();

  ExitVzLogging();
  LOG(L_ERROR) << "applet terminal.";
  return 0;
}

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif

  vzconn::EventService *p_evt_service =
    (vzconn::EventService *)vzbase::Thread::Current()->socketserver()->GetEvtService();

  Event_CreateSignalHandle(p_evt_service, SIGINT,  SignalHandle, NULL);
  Event_CreateSignalHandle(p_evt_service, SIGTERM, SignalHandle, NULL);
  Event_CreateSignalHandle(p_evt_service, SIGSEGV, SignalHandle, NULL);

  bool b_ret = false;
  if (argc == 3) {
    b_ret = web::CListenMessage::Instance()->Start(
      "127.0.0.1", 5291, atoi(argv[1]), argv[2]);
  } else {
    printf("-----------------------------------------------------------\n"\
           "app_name listen_port web_html_path.\n");
  }
  while (b_ret) {
    web::CListenMessage::Instance()->RunLoop();
  }

  return 0;
}
