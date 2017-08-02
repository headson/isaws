/************************************************************************/
/* Author      : SoberPeng 2017-07-15
/* Description :
/************************************************************************/
#ifndef LIBSYSTEMSERVER_CHWCLOCK_H
#define LIBSYSTEMSERVER_CHWCLOCK_H

#include "vzbase/base/basictypes.h"

#include "json/json.h"
#include "vzbase/thread/thread.h"

namespace sys {

class CHwclock : public vzbase::MessageHandler {
 protected:
  CHwclock(vzbase::Thread *thread_slow);

 public:
  static CHwclock *Create(vzbase::Thread *thread_slow);

  ~CHwclock();

  bool Start(bool ntp_enable=false);
  void Stop();

  bool ResetHwclock(Json::Value &jbody);
  void UpdateNetServer();

 protected:
  virtual void OnMessage(vzbase::Message* msg);

 protected:
  vzbase::Thread *thread_slow_;   // 

  unsigned int    ntp_enable_;    // NTP获取时间
  std::string     ntp_srv_addr_;  // 
};

}  // namespace sys

#endif  // LIBSYSTEMSERVER_CHWCLOCK_H
