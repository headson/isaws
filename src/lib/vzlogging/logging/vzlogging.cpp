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

static vzlogging::TAG_SHM_ARG *k_arg = NULL;

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

  // ������
  for (int i = 0; i < argc; i++) {
    if (strlen(argv[i]) >= 2) {
      switch (argv[i][1]) {
      case 'v':           // ��ӡ���ʹ��
      case 'V':
        k_log_print = true;
        break;
      }
    }
  }

  // �򿪹����ڴ�
  bool res = k_shm_arg.Open(DEF_WDG_SHM_ARG, sizeof(*k_arg));
  if (!res) {
    VZ_ERROR("share memory failed. %d.\n", res);
    return -1;
  }
  k_arg = (vzlogging::TAG_SHM_ARG*)k_shm_arg.GetData();
  if (NULL == k_arg) {
    VZ_ERROR("get share arg failed. %d.\n", res);
    return -1;
  }
  if (k_arg != NULL) {
    VZ_PRINT("share memory success. level %d.\n", k_arg->snd_level);
  }
  return 0;
}

/***������־��***********************************************************/
void ExitVzLogging() {
#ifdef WIN32
  WSACleanup();
#endif
}

/***һֱ��ʾ��־*********************************************************/
void ShowVzLoggingAlways() {
  k_log_print = true;
}

int Write(vzlogging::CVzLogSock *ptls,
          int local_print, unsigned int level,
          const char *slog, int nlog) {
  if (k_arg &&
      (local_print == 0) &&
      (level >= k_arg->snd_level)) {
    return ptls->Write(&k_arg->sock_addr, slog, nlog);
  }
  return 0;
}

/************************************************************************
*Description : ��ӡ��־
*Parameters  : level ��־�ȼ�,
*              file  ���ô˺����ļ�,
*              line  ���ô˺����ļ���,
*              fmt   ��ʽ���ַ���,
*              ...   ����������
*Return      : 0 success,-1 failed
************************************************************************/
int VzLog(unsigned int  nlevel,
          int           local_print,
          const char   *pfile,
          int           nline,
          const char   *pfmt,
          ...) {
  vzlogging::CVzLogSock* ptls = GetVzLogSock();
  if (!k_log_print) {
    if (local_print == 1) {  // DLOGʱ,����ǵ���ģʽ����ӡ
#ifdef NDEBUG
      ptls = NULL;
#endif
    }
  }
  if (ptls) {
    char* slog = ptls->slog;
    int&  nlog = ptls->nlog;

    // HEAD
    nlog = vzlogging::VzLogHead(nlevel, pfile, nline,
                                slog, ptls->max_nlog);
    if (nlog < 0) {
      return nlog;
    }

    // BODY
    va_list args;
    va_start(args, pfmt);
    nlog += vsnprintf(slog + nlog, ptls->max_nlog - nlog, pfmt, args);
    va_end(args);

    if (nlog <= ptls->max_nlog) {     // ������־buffer����4�ֽ�,�ɴ�'\n\0'
      slog[nlog++] = '\n';
    }

    if (nlevel != L_HEARTBEAT) {
      if (local_print == 1 || k_log_print) {
        slog[nlog] = '\0';
        vzlogging::VzLogPrint(slog, nlog);
      }
    }

    /*if (k_arg &&
        (b_local_print == 0) &&
        (n_level >= k_arg->snd_level)) {
      p_tls->Write(&k_arg->sock_addr, slog, nlog);
    }*/
    return Write(ptls, local_print, nlevel, slog, nlog);
  }
  return -1;
}

int VzLogBin(unsigned int nlevel,
             int          local_print,
             const char  *pfile,
             int          nline,
             const char  *pdata,
             int          nsize) {
  static const char HEX_INDEX[] = {
    '0', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', 'A', 'B',
    'C', 'D', 'E', 'F'
  };
  vzlogging::CVzLogSock* ptls = GetVzLogSock();
  if (!k_log_print) {
    if (local_print == 1) {  // DLOGʱ,����ǵ���ģʽ����ӡ
#ifdef NDEBUG
      ptls = NULL;
#endif
    }
  }
  if (ptls) {
    char* slog = ptls->slog;
    int& nlog  = ptls->nlog;

    // HEAD
    nlog = vzlogging::VzLogHead(nlevel, pfile, nline,
                                slog, ptls->max_nlog);

    // BODY
    int data_index = 0;
    int buffer_index = 0;
    while (buffer_index < (ptls->max_nlog - nlog)
           && data_index < nsize) {
      unsigned char c = pdata[data_index];
      slog[nlog + buffer_index++] = HEX_INDEX[c & 0X0F];
      slog[nlog + buffer_index++] = HEX_INDEX[c >> 4];
      slog[nlog + buffer_index++] = ' ';
      data_index++;
    }
    nlog += buffer_index;
    if (nlog <= ptls->max_nlog) {      // ������־buffer����4�ֽ�,�ɴ�'\n\0'
      slog[nlog++] = '\n';
    }

    if (local_print == 1 || k_log_print) {
      slog[nlog] = '\0';
      vzlogging::VzLogPrint(slog, nlog);
    }

    /*if (k_arg &&
        (b_local_print == 0) &&
        (n_level >= k_arg->snd_level)) {
      return p_tls->Write(&k_arg->sock_addr, slog, nlog);
    }*/
    return Write(ptls, local_print, nlevel, slog, nlog);
  }
  return -1;
}
