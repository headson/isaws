/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : vosinc.h
* Author        : Sober.Peng
* Date          : 28:12:2016
* Description   :
*-----------------------------------------------------------------------------
* Modify        : 
*-----------------------------------------------------------------------------
******************************************************************************/
#pragma once
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

// 操作2G文件读写
#ifndef __USE_FILE_OFFSET64
#define __USE_FILE_OFFSET64
#endif
#ifndef __USE_LARGEFILE64
#define __USE_LARGEFILE64
#endif
#ifndef _LARGEFILE64_SOURCE
#define _LARGEFILE64_SOURCE
#endif

#define SZ_1K       (1024)              // 
#define SZ_4K       (4 * SZ_1K)         // 
#define SZ_1M       (SZ_1K * SZ_1K)     // 
#define SZ_1G       (SZ_1K * SZ_1M)     // 

//#ifdef WIN32
//#define GTEST_ENABLE    1
//#endif

#ifdef WIN32
#define WIN32_EN    1
#endif
#if !defined(WIN32_EN) && defined(WIN32)
#pragma comment(lib, "sdk/pthread/pthreadVC2.lib")
#endif // WIN32

#include <time.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/timeb.h>

#define INVALID_HANDLER     -1

#undef FD_SETSIZE
#define FD_SETSIZE          512

#ifdef WIN32
#define NOMINMAX // warning C4003: not enough actual parameters for macro min, max

//#if (_WIN32_WINNT < 0x0603) // VS2013编译出错
#include <winsock2.h>
#include <ws2tcpip.h>
#include <mswsock.h>
//#endif // _WIN32_WINNT
#include <windows.h>
#include <process.h>
#include <io.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <direct.h>
#include <winbase.h>

#pragma comment(lib, "ws2_32.lib")

typedef SOCKET VHANDLER;

#ifndef snprintf
#define snprintf    _snprintf
#endif // snprintf

#define i_sync()  // sync;

inline int set_socket_blocking(VHANDLER vHdl)
{
    int mode = 0;
    ::ioctlsocket(vHdl, FIONBIO, (u_long FAR*)&mode);

    return 0;
}

inline int set_socket_nonblocking(VHANDLER vHdl)
{
    int mode = 1;
    ::ioctlsocket(vHdl, FIONBIO, (u_long FAR*)&mode);

    return 0;
}

//inline int close(BASE_HANDLER socket)
//{
//  return closesocket(socket);
//}

inline int error_no()
{
    return ::WSAGetLastError();
}

inline void usleep(uint32_t us)
{
    if(us < 1000) 
    {
        ::Sleep(1);
    }
    else 
    {
        ::Sleep(us / 1000);
    }
}

inline int access(const char *path, int mode)
{
    return _access(path, mode);
}

inline int32_t gettimeofday(struct timeval *tp, void *tzp)
{
    time_t clock;
    struct tm tm;
    SYSTEMTIME wtm;

    GetLocalTime(&wtm);
    tm.tm_year   = wtm.wYear - 1900;
    tm.tm_mon    = wtm.wMonth - 1;
    tm.tm_mday   = wtm.wDay;
    tm.tm_hour   = wtm.wHour;
    tm.tm_min    = wtm.wMinute;
    tm.tm_sec    = wtm.wSecond;
    tm. tm_isdst = -1;
    clock        = mktime(&tm);
    tp->tv_sec   = (long)clock;
    tp->tv_usec  = wtm.wMilliseconds * 1000;
    return (0);
}

#define F_OK            0
#define X_OK            1
#define W_OK            2
#define R_OK            4

#define XEAGAIN         WSAEWOULDBLOCK
#define XEINPROGRESS    WSAEWOULDBLOCK
#define XECONNABORTED   WSAECONNABORTED
#define XEINTR          WSAEWOULDBLOCK

#else //LINUX GCC
#include <sched.h>
#include <pthread.h>
#include <semaphore.h>

#include <netdb.h>
#include <getopt.h>
#include <unistd.h>     /*Unix 标准函数定义*/
#include <dirent.h>     /*opendir*/
#include <net/if.h>
#include <termios.h>    /*PPSIX 终端控制定义*/
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/vfs.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <linux/rtc.h>
#include <sys/select.h>
#include <sys/sysinfo.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <linux/wait.h>
#include <netinet/tcp.h>
#include <linux/types.h>
#include <linux/sockios.h>
#include <linux/netlink.h>
#include <linux/watchdog.h>

typedef int             VHANDLER;

#define i_sync()        sync()

#define XEAGAIN         EAGAIN
#define XEINPROGRESS    EINPROGRESS
#define XECONNABORTED   ECONNABORTED
#define XEINTR          EINTR

inline int set_socket_blocking(VHANDLER socket)
{
    int val = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, val & ~O_NONBLOCK);

    return 0;
}

inline int set_socket_nonblocking(VHANDLER socket)
{
    int val = fcntl(socket, F_GETFL, 0);
    fcntl(socket, F_SETFL, val | O_NONBLOCK);

    return 0;
}

inline int error_no()
{
    return errno;
}

inline int closesocket(VHANDLER socket)
{
    return close(socket);
}

#endif

#include <set>
#include <map>
#include <list>
#include <queue>
#include <deque>
#include <stack>
#include <string>
#include <vector>
#include <bitset>
#include <fstream>
#include <sstream>
#include <ostream>
#include <istream>
#include <iostream>
#include <algorithm>
#include <exception>
using namespace std;

#include "verror.h"     // 错误报告

///程序内部断言//////////////////////////////////////////////////////////
// 未使用变量
#define UNUSED_ARG(a)   do{/* null */} while (&a == 0)

// 无返回断言
#define ASSERT_NO(_exp) if(!(_exp))     { printf("assert failed: "#_exp); printf("%s[%d] \n", __FUNCTION__, __LINE__); }

// 返回值断言
#define ASSERT_RET(_exp, r) if(!(_exp)) { printf("assert failed: "#_exp); printf("%s[%d] \n", __FUNCTION__, __LINE__); return (r);}

//////////////////////////////////////////////////////////////////////////
// 基础类
class VObject
{
public:
    enum {
        OBJ_CREATE  = 1,            // 创建
        OBJ_INITED,                 // 已初始化
        OBJ_RUNING,                 // 运行

        OBJ_CLOSE   = 0xffffffff    // 关闭
    };

public:
    VObject() 
        : _sName("")
        , _nIndex(0)
        , _nState(OBJ_CREATE)
        , _nLiving(NULL) 
    { }
    ~VObject() {
        _sName = "";
    }

    // 类名
    const string& get_name() const { return _sName; }
    void          set_name(const string &sName) { _sName = sName; }
    // 类序号
    uint32_t      get_index() const { return _nIndex; }
    void          set_index(uint32_t nIndex) { _nIndex = nIndex; }

    // 状态
    virtual uint32_t    get_state() const {
        return _nState;
    }
    virtual void        set_state(uint32_t nState) {
        if (is_close()) { 
            return; 
        }

        _nState = nState;
        if (is_close()) {
            if (_nLiving) { *_nLiving = -111111; }
        }
    }
    virtual bool        is_close() {
        /*return ((OBJ_CLOSE == _nState) 
            || (OBJ_CREATE == _nState));*/

        return (OBJ_CLOSE == _nState); 
    }

    // 存活计数
    void     set_living(int32_t *nLiving) { _nLiving = nLiving; }
    uint32_t get_living() const { if (_nLiving) { return *_nLiving; } return 0; }
    void     rel_living() { 
        if (_nLiving) { 
            if ((*_nLiving) >= 0) { (*_nLiving) = -1; 
            } 
        } 
    }

    void     inc_living() { if (_nLiving) { if ((*_nLiving) >= 0) { (*_nLiving) ++; } } }

protected:
    string              _sName;     // 类名
    uint32_t            _nIndex;    // 类序号
    volatile uint32_t   _nState;    // 状态
    int32_t*            _nLiving;   // 存活计数
};

inline void app_init()
{
#ifdef WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    srand((uint32_t)time(NULL));
#else
    srand((uint32_t)time(NULL));
    srandom((uint32_t)time(NULL));
#endif
}

inline void app_destroy()
{
#ifdef WIN32
    WSACleanup();
#endif
}
