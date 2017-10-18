/************************************************************************/
/* Author      : SoberPeng 2017-07-15
/* Description :
/************************************************************************/
#include "chwclock.h"

#include <time.h>

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "dispatcher/sync/dpclient_c.h"

namespace sys {

CHwclock::CHwclock(vzbase::Thread *thread_slow)
  : thread_slow_(thread_slow)
  , timezone_(8) {
  ntp_.enable = 0;
  ntp_.server = "cn.ntp.org.cn";
}

CHwclock * CHwclock::Create(vzbase::Thread *thread_slow) {
  if (NULL == thread_slow) {
    LOG(L_ERROR) << "param is null";
    return NULL;
  }

  return (new CHwclock(thread_slow));
}

CHwclock::~CHwclock() {
  Stop();

  if (thread_slow_) {
    thread_slow_ = NULL;
  }
}

bool CHwclock::Start() {
  Json::Value  jtime;
  Json::Reader jread;

  std::string time_info = "";
  Kvdb_GetKeyToString(KVDB_TIME_INFO,
                      strlen(KVDB_TIME_INFO),
                      &time_info);
  if (!time_info.empty() &&
      jread.parse(time_info, jtime)) {
  } else {
    LOG(L_WARNING) << "use default timeinfo.";
    jtime["timezone"] = 8;
    jtime["ntp"]["enable"] = 1;
    jtime["ntp"]["server"] = "cn.ntp.org.cn";
    jtime["ntp"]["timeout"] = 30 * 60 * 1000; // 30min
  }

  SetTimeInfo(jtime, false);
  if (0 < ntp_.enable) {
    if (thread_slow_) {
      thread_slow_->PostDelayed(1 * 1000, this);
    }
  }
  return true;
}

void CHwclock::Stop() {
}


bool CHwclock::SetDevTime(const Json::Value &jbody) {
  int year, month, day, hour, min, sec;
  if (jbody["datetime"].isString()) {
    std::string str = jbody["datetime"].asString();
    int res = sscanf(str.c_str(),
                     "%4d-%2d-%2d %2d:%2d:%2d",
                     &year, &month, &day, &hour, &min, &sec);
    if (6 == res) {
      LOG(L_INFO) << str;

      time_t now = time(NULL);
      struct tm *tm  = localtime(&now);
      year = (year > 1900) ? year-1900 : 0;
      tm->tm_year = year;
      month = (month > 0) ? month-1 : 0;
      tm->tm_mon  = month;
      tm->tm_mday = day;
      tm->tm_hour = hour;
      tm->tm_min  = min;
      tm->tm_sec  = sec;

      if (abs(now - mktime(tm)) > 5) {
#ifdef _LINUX
        char scmd[1024] = {0};
        snprintf(scmd, 1023,
                 "date -s \"%s\";clock -w",
                 str.c_str());
        LOG(L_INFO) << scmd;
        system(scmd);
#endif  // _WIN32

        DpClient_SendDpMessage(MSG_TIME_CHANGE, 0,
                               (const char*)&now, sizeof(time_t));
        return true;
      }
    }
  }
  return false;
}

bool CHwclock::SetTimeZone(int timezone) {
  timezone_ = timezone;
  // 设置系统时区

  return true;
}

bool CHwclock::GetTimeInfo(Json::Value &jbody) {
  jbody["datetime"] = vzbase::SecToString(time(NULL));

  jbody["timezone"]       = timezone_;
  jbody["ntp"]["enable"]  = ntp_.enable;
  jbody["ntp"]["server"]  = ntp_.server;
  jbody["ntp"]["timeout"] = ntp_.timeout;
  return true;
}

bool CHwclock::SetTimeInfo(const Json::Value &jbody, bool bsave) {
  if (jbody.isMember("timezone") &&
      jbody.isMember("ntp") &&
      jbody["ntp"].isMember("enable") &&
      jbody["ntp"].isMember("server")) {

    timezone_ = jbody["timezone"].asInt();
    SetTimeZone(timezone_);

    ntp_.enable = jbody["ntp"]["enable"].asInt();
    ntp_.server = jbody["ntp"]["server"].asString();
    ntp_.timeout= jbody["ntp"]["timeout"].asInt();

    if (bsave) {
      Json::FastWriter jfw;
      std::string stime = jfw.write(jbody);
      int res = Kvdb_SetKey(KVDB_TIME_INFO,
                            strlen(KVDB_TIME_INFO),
                            stime.c_str(), stime.size());
      return (res == KVDB_RET_SUCCEED);
    }
    return true;
  }
  return false;
}

void CHwclock::OnMessage(vzbase::Message* msg) {
  // ntp
  char cmd[256] = {0};
  snprintf(cmd, 255,
           "ntpclient -h %s -c 1 -i 10 -s",
           ntp_.server.c_str());
  LOG(L_INFO) << cmd;
  clock_t c_begin = clock();
  time_t t_begin = time(NULL);
  int ret = system(cmd);
  LOG(L_INFO) << "ret=" << ret
              <<",t_begin=" << t_begin
              <<",c_begin=" << c_begin;
  if (ret >= 0 && ret != 127) {
    long diff_t = time(NULL) - t_begin;
    long diff_c = (clock() - c_begin)/CLOCKS_PER_SEC;
    LOG(L_INFO) << "diff_t=" << diff_t << ",diff_c=" << diff_c;
    if (diff_t != diff_c) {
      LOG(L_INFO) << "SendDpMessage,t_begin=" << t_begin;
      DpClient_SendDpMessage(MSG_TIME_CHANGE, 0,
                             (const char*)&t_begin, sizeof(time_t));
    }
  }

  if (thread_slow_ && ntp_.enable > 0) {
    thread_slow_->PostDelayed(30*60*1000, this);  // 30min
  }
}

}  // namespace sys
