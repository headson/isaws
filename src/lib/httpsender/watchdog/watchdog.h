
#ifndef _WATCH_DOG_H_
#define _WATCH_DOG_H_

#ifndef WIN32
#include "vz_sharemem.h"
#endif
#include "vzbase/base/boostsettings.h"

namespace hs {

class WatchDog : public boost::noncopyable,
  public boost::enable_shared_from_this <WatchDog> {

 public:

  typedef boost::shared_ptr<WatchDog> Ptr;

  WatchDog();
  ~WatchDog();

  int Init();

  void WatchDogFeed();

 private:
  void *p_watchdog_;
#if 0
  int watchdog_wmid_;
  int watchdog_semid_;
  int httpsender_watchdog_count_;
  int cftp_mt_watchdog_count_;
#endif
};
};

#endif