/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "vzlogging/logging/vzlogging.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "vzlogging/base/vzbases.h"
#include "vzlogging/base/vzcommon.h"

int InitVzLogging(int argc, char* argv[]) {
#ifdef WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
  srand((unsigned int)time(NULL));
#else
  srand((unsigned int)time(NULL));
  srandom((unsigned int)time(NULL));
#endif
  strncpy(k_app_name, vzlogging::GetFileName(argv[0]), LEN_APP_NAME);

  // 传参数
  for (int i = 0; i < argc; i++) {
    if (strlen(argv[i]) >= 2) {
      switch (argv[i][1]) {
      case 'v':           // 打印输出使能
      case 'V':
        k_log_print = true;
        break;
      }
    }
  }

  // 打开共享内存
  int ret = k_shm_arg.Open();
  if (ret != 0) {
    VZ_ERROR("share memory failed. %d.\n", ret);
  }
  VZ_PRINT("share memory success. level %d.\n",
           k_shm_arg.GetLevel());
  return ret;
}

/***销毁日志库***********************************************************/
void ExitVzLogging() {
#ifdef WIN32
  WSACleanup();
#endif
}

/***一直显示日志*********************************************************/
void ShowVzLoggingAlways() {
  k_log_print = true;
}

/************************************************************************
*Description : 打印日志
*Parameters  : level 日志等级,
*              file  调用此函数文件,
*              line  调用此函数文件行,
*              fmt   格式化字符串,
*              ...   不定长参数
*Return      : 0 success,-1 failed
************************************************************************/
int VzLog(unsigned int  n_level,
          int           b_local_print,
          const char    *p_file,
          int           n_line,
          const char    *p_fmt,
          ...) {
  vzlogging::CVzLogSock* p_tls = GetVzLogSock();
  if (!k_log_print) {
    if (b_local_print == 1) {  // DLOG时,如果是调试模式不打印
#ifdef NDEBUG
      p_tls = NULL;
#endif
    }
  }
  if (p_tls) {
    char* slog = p_tls->slog;
    int&  nlog = p_tls->nlog;

    // HEAD
    nlog = vzlogging::VzLogHead(n_level, p_file, n_line,
                                slog, p_tls->max_nlog);
    if (nlog < 0) {
      return nlog;
    }

    // BODY
    va_list args;
    va_start(args, p_fmt);
    nlog += vsnprintf(slog + nlog, p_tls->max_nlog - nlog, p_fmt, args);
    va_end(args);

    if (nlog <= p_tls->max_nlog) {     // 分配日志buffer多了4字节,可存'\n\0'
      slog[nlog++] = '\n';
      slog[nlog++] = '\0';
    }

    if (n_level != L_HEARTBEAT) {
      if (b_local_print == 1 || k_log_print) {
        vzlogging::VzLogPrint(slog, nlog);
      }
    }

    if ((b_local_print == 0) &&
        n_level >= k_shm_arg.GetLevel()) {
      p_tls->Write(k_shm_arg.GetSockAddr(), slog, nlog);
    }
    return 0;
  }
  return -1;
}

int VzLogBin(unsigned int n_level,
             int          b_local_print,
             const char  *p_file,
             int          n_line,
             const char  *p_data,
             int          n_size) {
  static const char HEX_INDEX[] = {
    '0', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', 'A', 'B',
    'C', 'D', 'E', 'F'
  };
  vzlogging::CVzLogSock* p_tls = GetVzLogSock();
  if (!k_log_print) {
    if (b_local_print == 1) {  // DLOG时,如果是调试模式不打印
#ifdef NDEBUG
      p_tls = NULL;
#endif
    }
  }
  if (p_tls) {
    char* slog = p_tls->slog;
    int& nlog  = p_tls->nlog;

    // HEAD
    nlog = vzlogging::VzLogHead(n_level, p_file, n_line,
                                slog, p_tls->max_nlog);

    // BODY
    int data_index = 0;
    int buffer_index = 0;
    while (buffer_index < (p_tls->max_nlog - nlog)
           && data_index < n_size) {
      unsigned char c = p_data[data_index];
      slog[nlog + buffer_index++] = HEX_INDEX[c & 0X0F];
      slog[nlog + buffer_index++] = HEX_INDEX[c >> 4];
      slog[nlog + buffer_index++] = ' ';
      data_index++;
    }
    nlog += buffer_index;
    if (nlog <= p_tls->max_nlog) {      // 分配日志buffer多了4字节,可存'\n\0'
      slog[nlog++] = '\n';
      slog[nlog++] = '\0';
    }

    if (b_local_print == 1 || k_log_print) {
      vzlogging::VzLogPrint(slog, nlog);
    }

    if ((b_local_print == 0) &&
        n_level >= k_shm_arg.GetLevel()) {
      return p_tls->Write(k_shm_arg.GetSockAddr(), slog, nlog);
    }
    return 0;
  }
  return -1;
}