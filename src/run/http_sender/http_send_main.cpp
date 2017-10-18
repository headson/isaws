#include "vzbase/helper/stdafx.h"
#include "vzbase/thread/thread.h"

#include "dispatcher/sync/dpclient_c.h"
#include "http_sender/clistenmessage.h"

void SignalHandle(void *usr_arg) {
  hs::CListenMessage::Instance()->Stop();
  exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif  // _WIN32

  vzconn::EventService *evt_service =
    (vzconn::EventService *)vzbase::Thread::Current()->socketserver()->GetEvtService();

  ExitSignalHandle(evt_service, "platform_app", SignalHandle, NULL);

  bool res = hs::CListenMessage::Instance()->Start();

  while(res) {
    try {
      hs::CListenMessage::Instance()->RunLoop();
    } catch(std::exception &e) {
      LOG(L_ERROR) << e.what();
    }
  }

  LOG(L_ERROR) << "sender server start failed.";
  return 0;
}