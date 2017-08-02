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

  bool Start();
  void Stop();

  // ntp
  bool SetDevTime(const Json::Value &jbody);

  bool SetTimeZone(int timezone);

  // timezone/ntp
  bool GetTimeInfo(Json::Value &jbody);
  // timezone/ntp
  bool SetTimeInfo(const Json::Value &jbody, bool bsave=true);

 protected:
  virtual void OnMessage(vzbase::Message* msg);

 protected:
  vzbase::Thread *thread_slow_;   // 

  int             timezone_;

  struct {
    unsigned int  enable;         // NTP获取时间
    std::string   server;         // ntp服务器
    unsigned int  timeout;        // 超时MS
  } ntp_;
};

}  // namespace sys

#endif  // LIBSYSTEMSERVER_CHWCLOCK_H
