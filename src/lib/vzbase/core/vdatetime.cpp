/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : vdatetime.cpp
* Author        : Sober.Peng
* Date          : 30:12:2016
* Description   :
*-----------------------------------------------------------------------------
* Modify        :
*-----------------------------------------------------------------------------
******************************************************************************/
#include "vdatetime.h"
#include "stdafx.h"

#include <time.h>
#include <stdio.h>

//#ifdef WIN32
//inline int32 gettimeofday(struct timeval *tp, void *tzp)
//{
//  time_t clock;
//  struct tm tm;
//  SYSTEMTIME wtm;
//
//  GetLocalTime(&wtm);
//  tm.tm_year   = wtm.wYear - 1900;
//  tm.tm_mon    = wtm.wMonth - 1;
//  tm.tm_mday   = wtm.wDay;
//  tm.tm_hour   = wtm.wHour;
//  tm.tm_min    = wtm.wMinute;
//  tm.tm_sec    = wtm.wSecond;
//  tm. tm_isdst = -1;
//  clock        = mktime(&tm);
//  tp->tv_sec   = (long)clock;
//  tp->tv_usec  = wtm.wMilliseconds * 1000;
//  return (0);
//}
//#endif

VDateTime::VDateTime() {
  this->set(0, 0);
}

VDateTime::VDateTime(uint32 s, uint32 us) {
  this->set(s, us);
}

VDateTime::VDateTime(const struct timeval& tv) {
  this->set(tv);
}

VDateTime::~VDateTime() {
}

VDateTime VDateTime::get() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv;
}

VDateTime VDateTime::get(time_t s) {
  VDateTime dt((uint32)s);

  return dt;
}

VDateTime VDateTime::get(const std::string& sdt) {
  VDateTime dt;
  if (!sdt.empty()) {
    struct tm tt;
    sscanf(sdt.c_str(), "%04d-%02d-%02d %02d:%02d:%02d",
           &(tt.tm_year), &(tt.tm_mon), &(tt.tm_mday), &(tt.tm_hour), &(tt.tm_min), &(tt.tm_sec));
    tt.tm_year  -= 1900;
    tt.tm_mon   --;
    tt.tm_isdst = -1;
    dt.set((uint32)mktime(&tt), 0);
  }
  return dt;
}

int64_t VDateTime::get_sys_tick() {
#ifdef WIN32
  //return (int64_t)GetTickCount64();
  return (int64_t)GetTickCount();
#else
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return ((int64_t)ts.tv_sec * 1000 +
          (int64_t)ts.tv_nsec / 1000000);
#endif
  return 0;
}

bool VDateTime::is_loop_yaer(int32 ny) {
  return (((0 == (ny % 4)) && (0 != (ny % 100))) || (0 == (ny % 400)));
}

void VDateTime::set(int64_t ms) {
  int64_t secs = ms / 1000;
  set((uint32)secs, ((ms % 1000) * 1000));
}

void VDateTime::set(uint32 s, uint32 us) {
  c_tv_.tv_sec = static_cast<long>(s);
  c_tv_.tv_usec = static_cast<long>(us);

  time_t tt = c_tv_.tv_sec;
#if 1//选择无时区
  p_tm_ = localtime(&tt);
#else
  p_tm_ = gmtime(&tt);
#endif

  //printf("date time %d %lld %d %d %d %d %d %d.\n",
  //    sec(),      msec(),
  //    year(),     month(),
  //    day(),      hour(),
  //    minute(),   second());
}

void VDateTime::set(const struct timeval& tv) {
  set(tv.tv_sec, tv.tv_usec);
}

uint32 VDateTime::tsec() const {
  return c_tv_.tv_sec;
}

uint32 VDateTime::tusec() const {
  return c_tv_.tv_usec;
}

int64_t VDateTime::to_msec() const {
  int64_t ms = int64_t(this->c_tv_.tv_sec);
  ms *= 1000;
  ms += (this->c_tv_.tv_usec / 1000);

  return ms;
}

uint64_t VDateTime::to_usec() const {
  uint64_t us = (uint64_t)(c_tv_.tv_sec);
  us *= ONE_SECOND_IN_USECS;
  us += c_tv_.tv_usec;

  return us;
}

int32 VDateTime::year() const {
  if (p_tm_) {
    return (p_tm_->tm_year+1900);
  }
  return 1900;
}

int32 VDateTime::month() const {
  if (p_tm_) {
    return (p_tm_->tm_mon+1);
  }
  return 1;
}

int32 VDateTime::day() const {
  if (p_tm_) {
    return (p_tm_->tm_mday);
  }
  return 1;
}

int32 VDateTime::hour() const {
  if (p_tm_) {
    return (p_tm_->tm_hour);
  }
  return 0;
}

int32 VDateTime::minute() const {
  if (p_tm_) {
    return (p_tm_->tm_min);
  }
  return 0;
}

int32 VDateTime::second() const {
  if (p_tm_) {
    return (p_tm_->tm_sec);
  }
  return 0;
}

VDateTime& VDateTime::operator=(const VDateTime& dt) {
  this->c_tv_ = dt.c_tv_;
  return *this;
}

VDateTime& VDateTime::operator=(const struct timeval& tv) {
  this->c_tv_ = tv;
  return *this;
}

VDateTime& VDateTime::operator +=(const VDateTime &dt) {
  uint64_t s  = this->tsec() + dt.tsec();
  uint64_t us = this->tusec() + this->tusec();

  s += us / ONE_SECOND_IN_USECS;
  us = us % ONE_SECOND_IN_USECS;
  set((uint32)s, (uint32)us);

  return *this;
}

VDateTime& VDateTime::operator -=(const VDateTime& dt) {
  if(this->to_usec() < dt.to_usec()) {
    c_tv_.tv_sec = 0;
    c_tv_.tv_usec = 0;
  } else {
    uint64_t us = this->to_usec();
    uint64_t us2 = dt.to_usec();
    us = us - us2;

    c_tv_.tv_sec = static_cast<long>(us / ONE_SECOND_IN_USECS);
    c_tv_.tv_usec = static_cast<long>(us % ONE_SECOND_IN_USECS);
  }
  return *this;
}

std::string VDateTime::to_string() const {
  char str[64] = {0};

  if (p_tm_) {
    snprintf(str, 63, "%4d-%02d-%02d %02d:%02d:%02d",
             p_tm_->tm_year+1900,
             p_tm_->tm_mon+1,
             p_tm_->tm_mday,
             p_tm_->tm_hour,
             p_tm_->tm_min,
             p_tm_->tm_sec);
  }

  return str;
}

bool operator==(const VDateTime& dt1, const VDateTime& dt2) {
  if(dt1.c_tv_.tv_sec == dt2.c_tv_.tv_sec
      && dt1.c_tv_.tv_usec == dt2.c_tv_.tv_usec)
    return true;
  else
    return false;
}

bool operator!=(const VDateTime& dt1, const VDateTime dt2) {
  if(dt1 == dt2)
    return false;
  else
    return true;
}

bool operator<(const VDateTime& dt1, const VDateTime& dt2) {
  if(dt1.to_usec() < dt2.to_usec())
    return true;
  else
    return false;
}

bool operator<=(const VDateTime& dt1, const VDateTime& dt2) {
  if(dt1.to_usec() <= dt2.to_usec())
    return true;
  else
    return false;
}

bool operator>(const VDateTime& dt1, const VDateTime& dt2) {
  if(dt1.to_usec() > dt2.to_usec())
    return true;
  else
    return false;
}

bool operator>=(const VDateTime& dt1, const VDateTime& dt2) {
  if(dt1.to_usec() >= dt2.to_usec())
    return true;
  else
    return false;
}

VDateTime operator+(const VDateTime& dt1, const VDateTime& dt2) {
  uint64_t us = dt1.to_usec() + dt2.to_usec();
  VDateTime timer((uint32)us / ONE_SECOND_IN_USECS, 
    (uint32)us % ONE_SECOND_IN_USECS);
  return timer;
}

VDateTime operator-(const VDateTime& dt1, const VDateTime& dt2) {
  {
    uint64_t us = 0;
    if(dt1.to_usec() > dt2.to_usec()) {
      us = dt1.to_usec() - dt2.to_usec();
    }

    VDateTime timer((uint32)us / ONE_SECOND_IN_USECS, 
      (uint32)us % ONE_SECOND_IN_USECS);
    return timer;
  }
}