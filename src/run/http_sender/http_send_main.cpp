#include "vzbase/helper/stdafx.h"
#include "vzbase/thread/thread.h"

#include "dispatcher/sync/dpclient_c.h"
#include "http_sender/clistenmessage.h"

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif  // _WIN32

  vzconn::EventService *evt_srv =
    (vzconn::EventService *)vzbase::Thread::Current()->socketserver()->GetEvtService();

  unsigned int is_exit = 0;
  ExitSignalHandle(evt_srv, &is_exit);

  bool res = hs::CListenMessage::Instance()->Start();
  if (false == res) {
    LOG(L_ERROR) << "start failed.";
    return -1;
  }

  while(0 == is_exit) {
    try {
      hs::CListenMessage::Instance()->RunLoop();
    } catch(std::exception &e) {
      LOG(L_ERROR) << e.what();
    }
  }

  LOG(L_ERROR) << "sender server start failed.";
  return 0;
}