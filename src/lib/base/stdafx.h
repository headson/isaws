/************************************************************************/
/* ����: SoberPeng 17-05-23
/* ����:
/************************************************************************/
#ifndef _VDEFINE_H
#define _VDEFINE_H

#include <time.h>
#include <stdio.h>
#include <stdlib.h>

#include "vtypes.h"

#ifdef WIN32
#define NOMINMAX // warning C4003: not enough actual parameters for macro min, max

#include <windows.h>
#include <winsock2.h>

#include <io.h>

#ifndef snprintf
#define snprintf    _snprintf
#endif // snprintf

#define i_sync()  // sync;

inline int set_socket_blocking(SOCKET vHdl) {
  int mode = 0;
  ::ioctlsocket(vHdl, FIONBIO, (u_long FAR*)&mode);
  return 0;
}

inline int set_socket_nonblocking(SOCKET vHdl) {
  int mode = 1;
  ::ioctlsocket(vHdl, FIONBIO, (u_long FAR*)&mode);
  return 0;
}

//inline int close(BASE_HANDLER socket) {
//  return closesocket(socket);
//}

inline int error_no() {
  return ::WSAGetLastError();
}

inline void usleep(uint32_t us) {
  if(us < 1000) {
    ::Sleep(1);
  } else {
    ::Sleep(us / 1000);
  }
}

inline int access(const char *path, int mode) {
  return _access(path, mode);
}

inline int32_t gettimeofday(struct timeval *tp, void *tzp) {
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

#else  // LINUX GCC
#include <unistd.h>
#include <fcntl.h>

#define i_sync()        sync()

#define XEAGAIN         EAGAIN
#define XEINPROGRESS    EINPROGRESS
#define XECONNABORTED   ECONNABORTED
#define XEINTR          EINTR

inline int set_socket_blocking(SOCKET socket) {
  int val = fcntl(socket, F_GETFL, 0);
  fcntl(socket, F_SETFL, val & ~O_NONBLOCK);
  return 0;
}

inline int set_socket_nonblocking(SOCKET socket) {
  int val = fcntl(socket, F_GETFL, 0);
  fcntl(socket, F_SETFL, val | O_NONBLOCK);
  return 0;
}

inline int error_no() {
  return errno;
}

inline int closesocket(SOCKET socket) {
  return close(socket);
}

#endif  // WIN32

#include "verror.h"
#include "vmessage.h"
#include "vzlogging/logging/vzlogging.h"
#include "vzlogging/logging/vzloggingcpp.h"

void app_init();      // 
void app_destroy();   // 

// ��־��ʼ��
int InitLogging(int argc, char* argv[]);

#endif  // _VDEFINE_H



