/************************************************************************/
/* Author      : SoberPeng 2017-06-09
/* Description :
/************************************************************************/
#ifndef VDATETIME_H
#define VDATETIME_H

#include "inc/vtypes.h"

#ifdef WIN32
#include <winsock2.h>
#else
#include <time.h>
#include <sys/time.h>
#endif

#include <string>

#define ONE_SECOND_IN_USECS 1000000
class VDateTime
{
public:
    VDateTime();
    VDateTime(uint32_t s, uint32_t us=0);
    VDateTime(const struct timeval& tv);

    ~VDateTime();

    static VDateTime get();
    static VDateTime get(time_t s);
    // 只支持2016-12-30 13:33:11一种格式
    static VDateTime get(const std::string& sdt);

    // 返回自系统开机以来的毫秒数（tick）
    static int64_t   get_sys_tick();
    static bool      is_loop_yaer(int32_t ny);

    void set(int64_t ms);
    void set(uint32_t s, uint32_t us);
    void set(const struct timeval& tv);

    uint32_t    tsec() const;
    uint32_t    tusec() const;

    int64_t     to_msec() const;
    uint64_t    to_usec() const;
    std::string to_string() const;  // 2016-12-30 13:41:33只一种格式

    int32_t     year() const;
    int32_t     month() const;
    int32_t     day() const;
    int32_t     hour() const;
    int32_t     minute() const;
    int32_t     second() const;

public:
    VDateTime& operator =(const VDateTime& dt);
    VDateTime& operator =(const struct timeval& tv);

    VDateTime& operator +=(const VDateTime &dt);
    VDateTime& operator -=(const VDateTime& dt);

    friend bool operator==(const VDateTime& dt1, const VDateTime& dt2);
    friend bool operator!=(const VDateTime& dt1, const VDateTime dt2);
    friend bool operator<(const VDateTime& dt1, const VDateTime& dt2);
    friend bool operator<=(const VDateTime& dt1, const VDateTime& dt2);
    friend bool operator>(const VDateTime& dt1, const VDateTime& dt2);
    friend bool operator>=(const VDateTime& dt1, const VDateTime& dt2);

    friend VDateTime operator +(const VDateTime& t1, const VDateTime& t2);
    friend VDateTime operator -(const VDateTime& t1, const VDateTime& t2);

private:
    struct timeval  c_tv_;
    struct tm*      p_tm_;
};

#endif  // VDATETIME_H
