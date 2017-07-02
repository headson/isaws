#include "vzbase/helper/stdafx.h"

#include "web_server/clistenmessage.h"

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif

  CListenMessage c_listen;
  bool b_ret = c_listen.Start((uint8*)"127.0.0.1", 5291, 
    (uint8*)"8000", (uint8*)"C:/tools/web");
  while (b_ret) {
    c_listen.RunLoop();
  }

  return 0;
}
