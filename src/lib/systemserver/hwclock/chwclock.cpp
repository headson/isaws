/************************************************************************/
/* Author      : SoberPeng 2017-07-15
/* Description :
/************************************************************************/
#include "chwclock.h"

#include <time.h>

#include "vzbase/helper/stdafx.h"
#include "vzbase/core/vmessage.h"
#include "vzbase/core/vdatetime.h"

#include "dispatcher/sync/dpclient_c.h"

namespace sys {

CHwclock::CHwclock(vzbase::Thread *thread_slow)
  : thread_slow_(thread_slow)
  , ntp_enable_(0)
  , ntp_srv_addr_("cn.ntp.org.cn") {
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

bool CHwclock::Start(bool ntp_enable) {
  if (ntp_enable) {
    ntp_enable_ = 1;

    UpdateNetServer();

    if (thread_slow_) {
      thread_slow_->PostDelayed(1*1000, this);
    }
  }
  return true;
}

void CHwclock::Stop() {
}

bool CHwclock::ResetHwclock(Json::Value &jbody) {
  int year, month, day, hour, min, sec;
  if (!jbody["datetime"].isString()) {
    std::string str = jbody["datetime"].asString();
    if (6 == sscanf(str.c_str(),
                    "%4d-%2d-%2d %2d:%2d:%2d",
                    &year, &month, &day, &hour, &min, &sec)) {
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

      if ((now = mktime(tm)) >= 0) {
#ifndef _WIN32
        stime(&now);
        system("hwclock -uw");  // hwclock 还不能正常运行
#endif  // _WIN32

        DpClient_SendDpMessage(MSG_TIME_CHANGE, 0,
                               (const char*)&now, sizeof(time_t));
        return true;
      }
    }
  }
  return false;
}

void CHwclock::UpdateNetServer() {
}

void CHwclock::OnMessage(vzbase::Message* msg) {
  // ntp
  char cmd[256] = {0};
  snprintf(cmd, 255, "ntpclient -h %s -c 1 -i 10 -s", ntp_srv_addr_.c_str());

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

  if (thread_slow_) {
    thread_slow_->PostDelayed(30*60*1000, this);  // 30min
  }
}

}  // namespace sys
