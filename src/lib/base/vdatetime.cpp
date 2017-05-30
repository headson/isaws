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
#include "stdafx.h"
#include "vdatetime.h"

VDateTime::VDateTime()
{
    this->set(0, 0);
}

VDateTime::VDateTime(uint32_t s, uint32_t us)
{
    this->set(s, us);
}

VDateTime::VDateTime(const struct timeval& tv)
{
    this->set(tv);
}

VDateTime::~VDateTime()
{
}

VDateTime VDateTime::get()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv;
}

VDateTime VDateTime::get(time_t s)
{
    VDateTime dt(s);

    return dt;
}

VDateTime VDateTime::get(const string& sdt)
{
    VDateTime dt;
    if (!sdt.empty())
    {
        struct tm tt;
        sscanf(sdt.c_str(), "%04d-%02d-%02d %02d:%02d:%02d",
            &(tt.tm_year), &(tt.tm_mon), &(tt.tm_mday), &(tt.tm_hour), &(tt.tm_min), &(tt.tm_sec));
        tt.tm_year  -= 1900;
        tt.tm_mon   --;
        tt.tm_isdst = -1;
        dt.set(mktime(&tt), 0);
    }
    return dt;
}

int64_t VDateTime::get_sys_tick()
{
#ifdef WIN32
    return (int64_t)GetTickCount64();
    //return (int64_t)GetTickCount();
#else
    struct timespec ts; 
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ((int64_t)ts.tv_sec * 1000 +
        (int64_t)ts.tv_nsec / 1000000);
#endif
    return 0;
}

bool VDateTime::is_loop_yaer(int32_t ny)
{
    return (((0 == (ny % 4)) && (0 != (ny % 100))) || (0 == (ny % 400)));
}

void VDateTime::set(int64_t ms)
{
    int64_t secs = ms / 1000;
    set(secs, ((ms % 1000) * 1000));
}

void VDateTime::set(uint32_t s, uint32_t us)
{
    cTv_.tv_sec = static_cast<long>(s);
    cTv_.tv_usec = static_cast<long>(us);

    time_t tt = cTv_.tv_sec;
#if 1//选择无时区
    pTm_ = localtime(&tt);
#else
    pTm_ = gmtime(&tt);
#endif

    //printf("date time %d %lld %d %d %d %d %d %d.\n", 
    //    sec(),      msec(), 
    //    year(),     month(),
    //    day(),      hour(),
    //    minute(),   second());
}

void VDateTime::set(const struct timeval& tv)
{
    set(tv.tv_sec, tv.tv_usec);
}

uint32_t VDateTime::tsec() const
{
    return cTv_.tv_sec;
}

uint32_t VDateTime::tusec() const
{
    return cTv_.tv_usec;
}

int64_t VDateTime::to_msec() const
{
    int64_t ms = int64_t(this->cTv_.tv_sec);
    ms *= 1000;
    ms += (this->cTv_.tv_usec / 1000);

    return ms;
}

uint64_t VDateTime::to_usec() const
{
    uint64_t us = (uint64_t)(cTv_.tv_sec);
    us *= ONE_SECOND_IN_USECS;
    us += cTv_.tv_usec;

    return us;
}

int32_t VDateTime::year() const
{
    if (pTm_) {
        return (pTm_->tm_year+1900);
    }
    return 1900;
}

int32_t VDateTime::month() const
{
    if (pTm_) {
        return (pTm_->tm_mon+1);
    }
    return 1;
}

int32_t VDateTime::day() const
{
    if (pTm_) {
        return (pTm_->tm_mday);
    }
    return 1;
}

int32_t VDateTime::hour() const
{
    if (pTm_) {
        return (pTm_->tm_hour);
    }
    return 0;
}

int32_t VDateTime::minute() const
{
    if (pTm_) {
        return (pTm_->tm_min);
    }
    return 0;
}

int32_t VDateTime::second() const
{
    if (pTm_) {
        return (pTm_->tm_sec);
    }
    return 0;
}

VDateTime& VDateTime::operator=(const VDateTime& dt)
{
    this->cTv_ = dt.cTv_;
    return *this;
}

VDateTime& VDateTime::operator=(const struct timeval& tv)
{
    this->cTv_ = tv;
    return *this;
}

VDateTime& VDateTime::operator +=(const VDateTime &dt)
{
    uint64_t s  = this->tsec() + dt.tsec();
    uint64_t us = this->tusec() + this->tusec();

    s += us / ONE_SECOND_IN_USECS;
    us = us % ONE_SECOND_IN_USECS;
    set(s, us);

    return *this;
}

VDateTime& VDateTime::operator -=(const VDateTime& dt)
{
    if(this->to_usec() < dt.to_usec())
    {
        cTv_.tv_sec = 0;
        cTv_.tv_usec = 0;
    }
    else
    {
        uint64_t us = this->to_usec();
        uint64_t us2 = dt.to_usec();
        us = us - us2;

        cTv_.tv_sec = static_cast<long>(us / ONE_SECOND_IN_USECS);
        cTv_.tv_usec = static_cast<long>(us % ONE_SECOND_IN_USECS);
    }
    return *this;
}

std::string VDateTime::to_string() const
{
    char str[64] = {0};
    
    if (pTm_) {
        snprintf(str, 63, "%4d-%02d-%02d %02d:%02d:%02d",
            pTm_->tm_year+1900,
            pTm_->tm_mon+1,
            pTm_->tm_mday,
            pTm_->tm_hour,
            pTm_->tm_min,
            pTm_->tm_sec);
    }

    return str;
}

bool operator==(const VDateTime& dt1, const VDateTime& dt2)
{
    if(dt1.cTv_.tv_sec == dt2.cTv_.tv_sec
        && dt1.cTv_.tv_usec == dt2.cTv_.tv_usec)
        return true;
    else
        return false;
}

bool operator!=(const VDateTime& dt1, const VDateTime dt2)
{
    if(dt1 == dt2)
        return false;
    else
        return true;
}

bool operator<(const VDateTime& dt1, const VDateTime& dt2)
{
    if(dt1.to_usec() < dt2.to_usec())
        return true;
    else 
        return false;
}

bool operator<=(const VDateTime& dt1, const VDateTime& dt2)
{
    if(dt1.to_usec() <= dt2.to_usec())
        return true;
    else
        return false;
}

bool operator>(const VDateTime& dt1, const VDateTime& dt2)
{
    if(dt1.to_usec() > dt2.to_usec())
        return true;
    else 
        return false;
}

bool operator>=(const VDateTime& dt1, const VDateTime& dt2)
{
    if(dt1.to_usec() >= dt2.to_usec())
        return true;
    else
        return false;
}

VDateTime operator+(const VDateTime& dt1, const VDateTime& dt2)
{
    uint64_t us = dt1.to_usec() + dt2.to_usec();
    VDateTime timer(us / ONE_SECOND_IN_USECS, us % ONE_SECOND_IN_USECS);
    return timer;
}

VDateTime operator-(const VDateTime& dt1, const VDateTime& dt2)
{
    {
        uint64_t us = 0;
        if(dt1.to_usec() > dt2.to_usec())
        {
            us = dt1.to_usec() - dt2.to_usec();
        }

        VDateTime timer(us / ONE_SECOND_IN_USECS, us % ONE_SECOND_IN_USECS);
        return timer;
    }
}