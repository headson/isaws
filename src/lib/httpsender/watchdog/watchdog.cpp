
#include "httpsender/watchdog/watchdog.h"
#include "vzbase/base/loggingsettings.h"
#include "vzlogging/logging/vzwatchdog.h"

namespace hs {

WatchDog::WatchDog() 
  : p_watchdog_(NULL) {
}

WatchDog::~WatchDog() {
}

int WatchDog::Init() {
  LOG(L_INFO) << "Init WatchDog Start";

#if 0
#ifndef WIN32
  watchdog_wmid_ = vzShareMemInit(WATCHDOG_SHARE_KEY);
  if (watchdog_wmid_ < 0)	{
    return -1;
  }
  watchdog_semid_ = vzSemInit(WATCHDOG_SEM_KEY);
  if (watchdog_semid_ < 0) {
    return -1;
  }
  httpsender_watchdog_count_ = 0;
  cftp_mt_watchdog_count_ = 0;
#endif  // WIN32
#else   // 0

  p_watchdog_ = RegisterWatchDogKey("MAIN", 4, DEF_MAX_TIMEOUT_SEC);
  if (p_watchdog_ == NULL) {
    return -1;
  }
#endif  // 0

  LOG(L_INFO) << "Init WatchDog Success";
  return 0;
}

void WatchDog::WatchDogFeed() {
  LOG(L_INFO) << "Feed WatchDog";
#if 0
#ifndef WIN32
  httpsender_watchdog_count_++;
  vzSemLock(watchdog_semid_);
  vzShareMemWrite(watchdog_wmid_, HTTPSENDER_OFFSET, &httpsender_watchdog_count_, sizeof(int));
  vzSemUnLock(watchdog_semid_);
  cftp_mt_watchdog_count_++;
  vzSemLock(watchdog_semid_);
  vzShareMemWrite(watchdog_wmid_, CFTP_OFFSET, &cftp_mt_watchdog_count_, sizeof(int));
  vzSemUnLock(watchdog_semid_);
#endif

#else  // 0

  if (p_watchdog_) {
    FeedDog(p_watchdog_);
  }

#endif  // 0
}
};