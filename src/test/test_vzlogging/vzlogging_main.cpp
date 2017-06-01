#include "vzlogging/include/vzlogging.h"
//#include "vzlogging/include/vzloggingcpp.h"
#include "vzlogging/include/vzwatchdog.h"

#ifdef WIN32
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#endif
#include <string.h>

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);
  
  VZLOG(L_INFO,     "message %d", 8);
  VZLOG(L_WARNING,  "message %d", 8);
  VZLOG(L_ERROR,    "message %d", 8);

  unsigned short n_sh = 1233;
  //VZLOGSTR(L_INFO)    << "Hello World!";

  void *p_wdg = RegisterWatchDogKey("TEST", 4, 21);
  void *p_wdg1 = RegisterWatchDogKey("TEST", 4, 21);

  const char data[16] = "message";
  
  int count = 0;
  int sa = 1000;
  int i = 0;
  for (i = 0; i < sa; i++) {
    //VZLOGSTR(L_INFO)    << "Hello World!";
    //VZLOGSTR(L_WARNING) << "Hello World!";
    //VZLOGSTR(L_ERROR)   << "Hello World!";

    VZLOG(L_INFO,     "message %d", 8);
    VZLOG(L_WARNING,  "message %d", 8);
    VZLOG(L_ERROR,    "message %d", 8);
    //VZLOG(L_DEBUG,    "message %d %d\n", 8, vzlogging::GetPid());

    VZLOGB(L_INFO,    data, 16);
    VZLOGB(L_WARNING, data, 16);
    VZLOGB(L_ERROR,   data, 16);
    
    if (p_wdg) {
      FeedDog(p_wdg);
    }
#ifdef WIN32
    _sleep(1);
#else
    usleep(1*1000);
#endif
  }

  ExitVzLogging();
  return 0;
}