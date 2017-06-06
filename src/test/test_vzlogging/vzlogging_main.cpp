#include "vzlogging/logging/vzlogging.h"
#include "vzlogging/logging/vzloggingcpp.h"
#include "vzlogging/logging/vzwatchdog.h"

#ifdef WIN32
#include <windows.h>
#include <process.h>
#endif
#include <string.h>

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);
  //ShowVzLoggingAlways();

  VZLOG(L_INFO,     "message %d", 8);
  LOG_INFO("messag");

  VZLOG(L_WARNING,  "message %d", 8);
  VZLOG(L_ERROR,    "message %d", 8);
  VZDLOG(L_INFO,    "message %d", 8);
  VZDLOG(L_WARNING, "message %d", 8);
  VZDLOG(L_ERROR,   "message %d", 8);

  unsigned short n_sh = 1233;
  
  void *p_wdg = RegisterWatchDogKey("TEST", 4, 21);
  void *p_wdg1 = RegisterWatchDogKey("TEST", 4, 21);

  const char data[16] = "message";

  const char ddata[16] = "D message";
  
  std::string ss = "hello string.";
  LOG(L_WARNING) << ss;
  LOG(L_WARNING) << 'c';
  LOG(L_WARNING) << (unsigned char)254;
  LOG(L_WARNING) << true;
  LOG(L_INFO) << 12345;
  LOG(L_INFO) << (short)54321;
  LOG(L_INFO) << 1234567;
  LOG(L_INFO) << 3876543210;
  LOG(L_ERROR) << (long long)38765432100000LL;
  LOG(L_ERROR) << (unsigned long long)38765432100000LL;
  LOG(L_ERROR) << (float)1.1;
  LOG(L_ERROR) << (double)1.1;
  LOG(L_ERROR) << (long double)1.1;

  DLOG(L_WARNING) << 'D' << ss;
  DLOG(L_WARNING) << 'D' << 'c';
  DLOG(L_WARNING) << 'D' << (unsigned char)254;
  DLOG(L_WARNING) << 'D' << true;
  DLOG(L_INFO) << 'D' << 12345;
  DLOG(L_INFO) << 'D' << (short)54321;
  DLOG(L_INFO) << 'D' << 1234567;
  DLOG(L_INFO) << 'D' << 3876543210;
  DLOG(L_ERROR) << 'D' << (long long)38765432100000LL;
  DLOG(L_ERROR) << 'D' << (unsigned long long)38765432100000LL;
  DLOG(L_ERROR) << 'D' << (float)1.1;
  DLOG(L_ERROR) << 'D' << (double)1.1;
  DLOG(L_ERROR) << 'D' << (long double)1.1;

  int count = 0;
  int sa = 1000;
  int i = 0;
  for (i = 0; i < sa; i++) {
    LOG(L_INFO) << "Hello World!";
    LOG(L_WARNING) << "Hello World!";
    LOG(L_ERROR) << "Hello World!";

    DLOG(L_INFO) << "D Hello World!";
    DLOG(L_WARNING) << "D Hello World!";
    DLOG(L_ERROR) << "D Hello World!";

    VZLOG(L_INFO,     "message %d", 8);
    VZLOG(L_WARNING,  "message %d", 8);
    VZLOG(L_ERROR,    "message %d", 8);

    VZDLOG(L_INFO, "D message %d", 8);
    VZDLOG(L_WARNING, "D message %d", 8);
    VZDLOG(L_ERROR, "D message %d", 8);

    VZLOGB(L_INFO,    data, 16);
    VZLOGB(L_WARNING, data, 16);
    VZLOGB(L_ERROR,   data, 16);

    VZDLOGB(L_INFO, ddata, 16);
    VZDLOGB(L_WARNING, ddata, 16);
    VZDLOGB(L_ERROR, ddata, 16);
    
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