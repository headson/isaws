/************************************************************************/
/* Author      : SoberPeng 2017-06-09
/* Description :
/************************************************************************/
#ifndef VDATETIME_H
#define VDATETIME_H

#include "vzbase/base/basictypes.h"

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
    VDateTime(uint32 s, uint32 us=0);
    VDateTime(const struct timeval& tv);

    ~VDateTime();

    static VDateTime get();
    static VDateTime get(time_t s);
    // 只支持2016-12-30 13:33:11一种格式
    static VDateTime get(const std::string& sdt);

    // 返回自系统开机以来的毫秒数（tick）
    static int64_t   get_sys_tick();
    static bool      is_loop_yaer(int32 ny);

    void set(int64_t ms);
    void set(uint32 s, uint32 us);
    void set(const struct timeval& tv);

    uint32    tsec() const;
    uint32    tusec() const;

    int64_t     to_msec() const;
    uint64_t    to_usec() const;
    std::string to_string() const;  // 2016-12-30 13:41:33只一种格式

    int32     year() const;
    int32     month() const;
    int32     day() const;
    int32     hour() const;
    int32     minute() const;
    int32     second() const;

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
