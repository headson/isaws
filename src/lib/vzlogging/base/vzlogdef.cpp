#include "vzlogging/base/vzlogdef.h"

#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#include <process.h>
#else
#include <fcntl.h>
#include <unistd.h>

#include <sys/time.h>

#include <pthread.h>
#endif

#include "vzlogging/logging/vzlogging.h"

#ifdef WIN32
void gettimeofday(struct timeval *tp, struct timezone *tz) {
  FILETIME  ft;
  uint64_t  intervals;

  GetSystemTimeAsFileTime(&ft);
  intervals = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
  intervals -= 116444736000000000;

  tp->tv_sec = (uint32_t)(intervals / 10000000);
  tp->tv_usec = (uint32_t)((intervals % 10000000) / 10);
}
#endif

namespace vzlogging {

// 获取进程ID
unsigned int GetPid() {
#ifdef WIN32
  return (unsigned int)_getpid();
#else
  return getpid();
#endif
}

// 获取线程ID
unsigned int GetPpid() {
#ifndef WIN32
  return (unsigned int)(pthread_self() % 0XFFFF);
#else
  return (unsigned int)GetCurrentThreadId();
#endif
}

// 通过文件路径,获取文件名
const char *GetFileName(const char *filepath) {
  int   n_file_name = strlen(filepath);
  const char *res_pos = filepath + n_file_name;
  for (int i = n_file_name; i > 0; i--) {
    if (filepath[i] == '\\' || filepath[i] == '/') {
      res_pos = filepath + i + 1;
      break;
    }
  }
  if (res_pos != (filepath + n_file_name)) {
    return res_pos;
  }
  return filepath;
}

// HEAD
struct TAG_LOG_LEVEL {
  int   level;
  char  option;
} k_log_level[5] = {
  { L_DEBUG,      L_C_DEBUG },
  { L_INFO,       L_C_INFO },
  { L_WARNING,    L_C_WARNING },
  { L_ERROR,      L_C_ERROR },
  { L_HEARTBEAT,  L_C_HEARTBEAT },
};
int VzLogPackHead(unsigned int  level,
                  const char    *file,
                  int           line,
                  char*         slog,
                  int           nlog) {
  if (level > (sizeof(k_log_level)/sizeof(struct TAG_LOG_LEVEL))) {
    return -1;
  }

  struct timeval tv;
  gettimeofday(&tv, NULL);
  time_t tt = tv.tv_sec;
  struct tm *wtm = localtime(&tt);
  nlog = snprintf(slog, DEF_LOG_MAX_SIZE,
                  "%c %04d %04d %02d:%02d:%02d.%03d %s:%d] ",
                  k_log_level[level].option,
                  GetPid(), GetPpid(),
                  wtm->tm_hour, wtm->tm_min, wtm->tm_sec,
                  (int)(tv.tv_usec / 1000),
                  GetFileName(file), line);
  return nlog;
}

int VzLogPrintFailed(const char *file, int line, const char *fmt, ...) {
  int nlog = 0;
  char slog[DEF_LOG_MAX_SIZE+2] = {0};

  nlog = VzLogPackHead(L_DEBUG, file, line, slog, DEF_LOG_MAX_SIZE);
  if (nlog < 0) {
    return nlog;
  }

  // BODY
  va_list args;
  va_start(args, fmt);
  nlog += vsnprintf(slog + nlog, DEF_LOG_MAX_SIZE - nlog, fmt, args);
  va_end(args);

  if (nlog < DEF_LOG_MAX_SIZE) {
    slog[nlog] = '\0';
  }
  printf(slog);
  fflush(stdout);
  return 0;
}


#ifdef WIN32
BOOL SetConsoleColor(WORD wAttributes) {
  HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
  if (hConsole == INVALID_HANDLE_VALUE)
    return FALSE;

  return SetConsoleTextAttribute(hConsole, wAttributes);
}
#else
#define VZ_NONE                 "\e[0m"
#define VZ_NORMAL               "I"
#define VZ_RED                  "\e[0;31m"
#define VZ_YELLOW               "\e[1;33m"
#endif

void VzDumpLogging(const char* s_msg, int n_msg) {
#ifdef WIN32
  switch (s_msg[0]) {
  case L_C_WARNING:
    SetConsoleColor(FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
    break;
  case L_C_ERROR:
    SetConsoleColor(FOREGROUND_INTENSITY | FOREGROUND_RED);
    break;
  default:
    break;
  }

  fprintf(stdout, "%s", s_msg);
  fflush(stdout);
  if (s_msg[0] == L_C_WARNING || s_msg[0] == L_C_ERROR) {
    SetConsoleColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
  }
#else
  const char *color = NULL;
  switch (s_msg[0]) {
  case L_C_INFO:
  case L_C_DEBUG:
  case L_C_HEARTBEAT:
    color = VZ_NONE;
    break;
  case L_C_WARNING:
    color = VZ_YELLOW;
    break;
  case L_C_ERROR:
    color = VZ_RED;
    break;
  default:
    break;
  }
  fprintf(stdout, "%s%s%s", color, s_msg, VZ_NONE);
  fflush(stdout);
#endif
}

}  // namespace vzlogging

