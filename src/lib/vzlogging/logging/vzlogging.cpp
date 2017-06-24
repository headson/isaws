/************************************************************************/
/* Copyright@ 2008 vzenith.com
/* All rights reserved.
/* ----------------------------------------------------------------------
/* Author      : Sober.Peng
/* Date        : 19:5:2017
/* Description :
/************************************************************************/
#include "vzlogging/logging/vzlogging.h"
#include "vzlogging/logging/vzloggingcpp.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "vzlogging/base/vzlogdef.h"
#include "vzlogging/base/vzshmarg.h"
#include "vzlogging/logging/vzwatchdog.h"

#ifdef WIN32
// TLS
#include <process.h>
typedef DWORD               TlsKey;
#define TLS_NULL            0
#else
#include <fcntl.h>
#include <unistd.h>

#include <sys/time.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define INVALID_SOCKET -1

// TLS
#include <pthread.h>
typedef pthread_key_t       TlsKey;
#define TLS_NULL            -1
#endif

namespace vzlogging {

static int      k_total_log = 0;
static void*    k_tls_void[DEF_PER_PRO_THREAD_MAX];   // ˽������ָ��
/**VTls******************************************************************/
/**��־�߳�˽������******************************************************/
class CTlsLog {
 public:
  CTlsLog()
    : s_(INVALID_SOCKET)
    , nlog_max_(DEF_LOG_MAX_SIZE) {
    //VZ_PRINT("%s[%d].\n", __FUNCTION__, __LINE__);
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(DEF_SERVER_PORT);
    addr_.sin_addr.s_addr = inet_addr(DEF_SERVER_HOST);
  }

  ~CTlsLog() {
    if (s_ != INVALID_SOCKET) {
#ifdef WIN32
      closesocket(s_);
#else
      ::close(s_);
#endif
      s_ = INVALID_SOCKET;
    }
    //VZ_PRINT("%s[%d].\n", __FUNCTION__, __LINE__);
  }

  int InitSocket() {
    SOCKET s = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (s <= 0) {
      return -1;
    }
    // REUSE address
    int val = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char*)&val, sizeof(int));

    // nonblock
#ifdef WIN32
    int mode = 1;
    ::ioctlsocket(s, FIONBIO, (u_long FAR*)&mode);
#else
    int mode = fcntl(s, F_GETFL, 0);
    fcntl(s, F_SETFL, val | O_NONBLOCK);
#endif
    s_ = s;
    return 0;
  }

  void SetRemoteAddr(const char* ip, unsigned short port) {
    addr_.sin_family = AF_INET;
    addr_.sin_port = htons(port);
    addr_.sin_addr.s_addr = inet_addr(ip);
  }

  // ��ַ���Ϸ���������־
  int SetRemoteAddr(struct sockaddr_in* addr) {
    if (addr == NULL
        || addr->sin_port == 0
        || addr_.sin_addr.s_addr == 0) {
      return -1;
    }

    if (addr_.sin_port != addr->sin_port
        || addr_.sin_family != addr->sin_family
        || addr_.sin_addr.s_addr != addr->sin_addr.s_addr) {
      addr_ = *addr;
    }
    return 0;
  }

  int Write(struct sockaddr_in* addr, const char* msg, unsigned int size) {
    SetRemoteAddr(addr);

    if (addr_.sin_port == 0 || addr_.sin_addr.s_addr == 0) {
      return -1;
    }
    int ns = ::sendto(s_, msg, size, 0,
                      (sockaddr*)&addr_, sizeof(addr_));
    if (ns != size) {
      //VZ_PRINT("sendto log failed.\n");
    }

    k_total_log += ns;
    return ns;
  }

 private:
  SOCKET              s_;
  struct sockaddr_in  addr_;

 public:
  const int           nlog_max_;
  int                 nlog_;
  char                slog_[DEF_LOG_MAX_SIZE + 1];  // ��־ʹ��buffer
};

class VTls {
 public:
  VTls() : tls_key_(TLS_NULL) {
    //VZ_PRINT("%s[%d].\n", __FUNCTION__, __LINE__);
  }
  ~VTls() {
    KeyFree();
    for (int i = 0; i < DEF_PER_PRO_THREAD_MAX; i++) {
      if (k_tls_void[i] != NULL) {
        delete (CTlsLog*)k_tls_void[i];
        k_tls_void[i] = NULL;
      }
    }
    //VZ_PRINT("%s[%d].\n", __FUNCTION__, __LINE__);
  }

  // ��������ʱ����һ�Σ�����ؼ���
  TlsKey KeyAlloc() {
    if (!IsReady()) {
#ifdef WIN32
      tls_key_ = TlsAlloc();
#else
      pthread_key_create(&tls_key_, NULL);
#endif
    }
    return tls_key_;
  }

  void KeyFree() {
    if (tls_key_ != TLS_NULL) {
#ifdef WIN32
      TlsFree(tls_key_);
#else
      pthread_key_delete(tls_key_);
#endif
      tls_key_ = TLS_NULL;
    }
  }

  // true����,false������
  bool   IsReady() {
    if (TLS_NULL != tls_key_) {
      return true;
    }
    return false;
  }

  // �߳��е��ã������ؼ��ֺ�˽�����ݶ�Ӧָ��
  bool   SetValue(void* value) {
    if (!IsReady()) {
      return false;
    }
#ifdef WIN32
    return TlsSetValue(tls_key_, value) == TRUE;
#else
    return pthread_setspecific(tls_key_, value);
#endif
  }
  // �߳��е��ã���ȡ�ؼ��ֹ������߳�˽������
  void*  GetValue() {
    if (!IsReady()) {
      return NULL;
    }
#ifdef WIN32
    return TlsGetValue(tls_key_);
#else
    return pthread_getspecific(tls_key_);
#endif
  }

 private:
  TlsKey  tls_key_;
};

// ���Ź��ṹ
typedef struct {
  unsigned int n_mark;                                  // У����
  unsigned int n_max_timeout;                           // ���ʱʱ��
  char         s_descrebe[DEF_USER_DESCREBE_MAX+4];     // �û�����
  unsigned int n_descrebe_size;                         // �û�������С
} TAG_WATCHDOG;

/**static value**********************************************************/
static char           k_app_name[DEF_PROCESS_NAME_MAX] = { 0 };  // ������

static CVzShmArg      k_shm_arg;                      // �����ڴ�

static VTls           k_tls_log;                      // ��־�߳�˽������

static TAG_WATCHDOG   k_watchdog[DEF_PER_PRO_WATCHDOG_MAX];  // ���Ź��ṹ

static bool           k_en_stdout = false;            // ���ʹ��
static unsigned int   k_snd_level = L_ERROR;          // ���͵ȼ�

/**��̬����**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

// ��ȡ�߳�˽������
static vzlogging::CTlsLog* GetTlsLog() {
  k_tls_log.KeyAlloc();

  if (!k_tls_log.IsReady()) {
    VZ_ERROR("You can't get tls key.\n");
    return NULL;
  }

  CTlsLog* hdl = (CTlsLog*)k_tls_log.GetValue();
  if (NULL == hdl) {
    CTlsLog* hdl_log = new CTlsLog();
    if (hdl_log) {
      hdl_log->InitSocket();
      k_tls_log.SetValue((void*)hdl_log);

      for (int i = 0; i < DEF_PER_PRO_THREAD_MAX; i++) {
        if (k_tls_void[i] == NULL) {
          k_tls_void[i] = hdl_log;
          break;
        }
      }
    }

    hdl = (CTlsLog*)k_tls_log.GetValue();
  }
  return hdl;
}

#ifdef __cplusplus
}
#endif

/**��־����ʽ��**********************************************************/
CVzLogStream::CVzLogStream(int          n_level,
                           const char*  p_file,
                           int          n_line,
                           unsigned int b_print) {
b_local_print_  = b_print;
n_level_        = n_level;

p_tls_          = GetTlsLog();
if (!k_en_stdout) {
  if (b_local_print_ == 1) {
#ifdef NDEBUG
    p_tls_ = NULL;
#endif
  }
}

if (p_tls_) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  tls_log->nlog_ = VzLogPackHead(n_level, p_file, n_line,
                                 tls_log->slog_, tls_log->nlog_max_);
  if (tls_log->nlog_ < 0) {
    return;
  }
}
}

vzlogging::CVzLogStream::~CVzLogStream() {
if (p_tls_) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  if (tls_log->nlog_ < 0) {
    return;
  }

  if (tls_log->nlog_ < tls_log->nlog_max_) {
    tls_log->slog_[tls_log->nlog_++] = '\n';
  }

  if (k_en_stdout) {
    if (tls_log->nlog_ < tls_log->nlog_max_) {
      tls_log->slog_[tls_log->nlog_++] = '\0';
    }
    VzDumpLogging(tls_log->slog_, tls_log->nlog_);
  }

  if ((b_local_print_ == 0) &&
      (n_level_ >= k_shm_arg.GetSendLevel())) {
    tls_log->Write(k_shm_arg.GetSockAddr(), tls_log->slog_, tls_log->nlog_);
  }
}
}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(char val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
if (p_tls_) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  if (tls_log->nlog_ < 0) {
    return *this;
  }

  tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                             tls_log->nlog_max_ - tls_log->nlog_,
                             "%c", val);
}
return *this;
}


vzlogging::CVzLogStream&
CVzLogStream::operator<<(unsigned char val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
if (p_tls_) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  if (tls_log->nlog_ < 0) {
    return *this;
  }

  tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                             tls_log->nlog_max_ - tls_log->nlog_,
                             "%u", (int)val);
}
return *this;
}

vzlogging::CVzLogStream&
CVzLogStream::operator<<(bool val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
if (p_tls_) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  if (tls_log->nlog_ < 0) {
    return *this;
  }

  tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                             tls_log->nlog_max_ - tls_log->nlog_,
                             "%s", (val ? "true" : "false"));
}
return *this;
}

vzlogging::CVzLogStream&
CVzLogStream::operator<<(short val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
if (p_tls_) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  if (tls_log->nlog_ < 0) {
    return *this;
  }

  tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                             tls_log->nlog_max_ - tls_log->nlog_,
                             "%d", (int)val);
}
return *this;
}

vzlogging::CVzLogStream&
CVzLogStream::operator<<(unsigned short val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
if (p_tls_) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  if (tls_log->nlog_ < 0) {
    return *this;
  }

  tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                             tls_log->nlog_max_ - tls_log->nlog_,
                             "%u", (int)val);
}
return *this;
}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(int t) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
if (p_tls_) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  if (tls_log->nlog_ < 0) {
    return *this;
  }

  tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                             tls_log->nlog_max_ - tls_log->nlog_,
                             "%d", t);
}
return *this;
}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(unsigned int t) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
if (p_tls_) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  if (tls_log->nlog_ < 0) {
    return *this;
  }

  tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                             tls_log->nlog_max_ - tls_log->nlog_,
                             "%u", t);
}
return *this;
}

vzlogging::CVzLogStream&
CVzLogStream::operator<<(long val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
if (p_tls_) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  if (tls_log->nlog_ < 0) {
    return *this;
  }

  tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                             tls_log->nlog_max_ - tls_log->nlog_,
                             "%ld", val);
}
return *this;
}

vzlogging::CVzLogStream&
CVzLogStream::operator<<(unsigned long val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
if (p_tls_) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  if (tls_log->nlog_ < 0) {
    return *this;
  }

  tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                             tls_log->nlog_max_ - tls_log->nlog_,
                             "%lu", val);
}
return *this;
}

vzlogging::CVzLogStream&
CVzLogStream::operator<<(long long val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
if (p_tls_) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  if (tls_log->nlog_ < 0) {
    return *this;
  }

  tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                             tls_log->nlog_max_ - tls_log->nlog_,
                             "%lld", val);
}
return *this;
}

vzlogging::CVzLogStream&
CVzLogStream::operator<<(unsigned long long val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
if (p_tls_) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  if (tls_log->nlog_ < 0) {
    return *this;
  }

  tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                             tls_log->nlog_max_ - tls_log->nlog_,
                             "%llu", val);
}
return *this;
}

vzlogging::CVzLogStream&
CVzLogStream::operator<<(float val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
if (p_tls_) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  if (tls_log->nlog_ < 0) {
    return *this;
  }

  tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                             tls_log->nlog_max_ - tls_log->nlog_,
                             "%f", val);
}
return *this;
}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(double val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
if (p_tls_) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  if (tls_log->nlog_ < 0) {
    return *this;
  }

  tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                             tls_log->nlog_max_ - tls_log->nlog_,
                             "%f", val);
}
return *this;
}


vzlogging::CVzLogStream&
CVzLogStream::operator<<(long double val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
if (p_tls_) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  if (tls_log->nlog_ < 0) {
    return *this;
  }

  tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                             tls_log->nlog_max_ - tls_log->nlog_,
                             "%lf", val);
}
return *this;
}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(const char* val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
if (p_tls_ && val) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  if (tls_log->nlog_ < 0) {
    return *this;
  }

  tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                             tls_log->nlog_max_ - tls_log->nlog_,
                             "%s", val);
}
return *this;
}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(std::string str) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
if (p_tls_ && str.size()>0) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  if (tls_log->nlog_ < 0) {
    return *this;
  }

  tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                             tls_log->nlog_max_ - tls_log->nlog_,
                             "%s", str.c_str());
}
return *this;
}

//vzlogging::CVzLogStream&
//vzlogging::CVzLogStream::operator<<(const time_t tt) {
//  //printf("%s[%d]\n", __FUNCTION__, __LINE__);
//  if (p_tls_) {
//    CTlsLog* tls_log = (CTlsLog*)p_tls_;
//    if (tls_log->nlog_ < 0) {
//      return *this;
//    }
//
//    tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
//                               tls_log->nlog_max_ - tls_log->nlog_,
//                               "%d", tt);
//  }
//  return *this;
//}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::write(const char* s_msg, int n_msg) {
if (p_tls_ && s_msg && n_msg > 0) {
  CTlsLog* tls_log = (CTlsLog*)p_tls_;
  if (tls_log->nlog_ < 0) {
    return *this;
  }

  int n_left = tls_log->nlog_max_ - tls_log->nlog_; // ʣ��ռ�
  int n_less = n_left > n_msg ? n_msg : n_left;     // ��Ϣ��ʣ���ַ����Ǹ�С

  memcpy(tls_log->slog_ + tls_log->nlog_, s_msg, n_less);
  tls_log->nlog_ += n_less;
}
return *this;
}

}  // namespace vzlogging

/**��־����ӿ�**********************************************************/
int InitVzLogging(int argc, char* argv[]) {
  #ifdef WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
  srand((unsigned int)time(NULL));
  #else
  srand((unsigned int)time(NULL));
  srandom((unsigned int)time(NULL));
  #endif
  memset(vzlogging::k_tls_void, 0, sizeof(vzlogging::k_tls_void));
  memset(vzlogging::k_watchdog, 0, sizeof(vzlogging::k_watchdog));

  // ������
  memcpy(vzlogging::k_app_name, vzlogging::GetFileName(argv[0]), 31);
  VZ_ERROR("%s, compile time %s %s.\n",
           vzlogging::k_app_name, __DATE__, __TIME__);

  // ������
  for (int i = 0; i < argc; i++) {
    if (strlen(argv[i]) >= 2) {
      switch (argv[i][1]) {
      case 'v':           // ��ӡ���ʹ��
      case 'V':
        vzlogging::k_en_stdout = true;
        break;
      }
    }
  }

  // �򿪹����ڴ�
  int ret = vzlogging::k_shm_arg.Open();
  if (ret != 0) {
    VZ_ERROR("share memory failed. %d.\n", ret);
  }
  VZ_PRINT("share memory success. level %d.\n",
           vzlogging::k_shm_arg.GetSendLevel());
  return ret;
}

/***������־��***********************************************************/
void ExitVzLogging() {
  vzlogging::k_shm_arg.Close();
  vzlogging::k_tls_log.KeyFree();
  VZ_PRINT("send message size %d.\n", vzlogging::k_total_log);

#ifdef WIN32
  WSACleanup();
#endif
}

/***һֱ��ʾ��־*********************************************************/
void ShowVzLoggingAlways() {
  vzlogging::k_en_stdout = true;
}

/************************************************************************/
/* Description : ��ӡ��־
/* Parameters  : level ��־�ȼ�,
file  ���ô˺����ļ�,
line  ���ô˺����ļ���,
fmt   ��ʽ���ַ���,
...   ����������
/* Return      : 0 success,-1 failed
/************************************************************************/
int VzLog(unsigned int  n_level,
          int           b_local_print,
          const char    *p_file,
          int           n_line,
          const char    *p_fmt,
          ...) {
  vzlogging::CTlsLog* p_tls = vzlogging::GetTlsLog();
  //printf("%s[%d] %d.\n", __FUNCTION__, __LINE__, vzlogging::k_en_stdout);
  if (!vzlogging::k_en_stdout) {
    if (b_local_print == 1) {  // DLOGʱ,����ǵ���ģʽ����ӡ
#ifdef NDEBUG
      p_tls = NULL;
#endif
    }
  }
  if (p_tls) {
    char* slog = p_tls->slog_;
    int&  nlog = p_tls->nlog_;

    // HEAD
    nlog = vzlogging::VzLogPackHead(n_level, p_file, n_line,
                                    slog, p_tls->nlog_max_);
    if (nlog < 0) {
      return nlog;
    }

    // BODY
    va_list args;
    va_start(args, p_fmt);
    nlog += vsnprintf(slog + nlog, p_tls->nlog_max_ - nlog, p_fmt, args);
    va_end(args);

    if (nlog < p_tls->nlog_max_) {
      slog[nlog++] = '\n';
    }

    if (n_level != L_HEARTBEAT) {
      if (b_local_print == 1 || vzlogging::k_en_stdout) {
        if (nlog < p_tls->nlog_max_) {
          slog[nlog++] = '\0';
        }
        vzlogging::VzDumpLogging(slog, nlog);
      }
    }

    if ((b_local_print == 0) &&
        n_level >= vzlogging::k_shm_arg.GetSendLevel()) {
      p_tls->Write(vzlogging::k_shm_arg.GetSockAddr(), slog, nlog);
    }
    return 0;
  }
  return -1;
}

int VzLogBin(unsigned int n_level,
             int            b_local_print,
             const char     *p_file,
             int            n_line,
             const char     *p_data,
             int            n_size) {
  static const char HEX_INDEX[] = {
    '0', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', 'A', 'B',
    'C', 'D', 'E', 'F'
  };
  vzlogging::CTlsLog* p_tls = vzlogging::GetTlsLog();
  if (!vzlogging::k_en_stdout) {
    if (b_local_print == 1) {  // DLOGʱ,����ǵ���ģʽ����ӡ
#ifdef NDEBUG
      p_tls = NULL;
#endif
    }
  }
  if (p_tls) {
    char* slog = p_tls->slog_;
    int& nlog  = p_tls->nlog_;

    // HEAD
    nlog = vzlogging::VzLogPackHead(n_level, p_file, n_line,
                                    slog, p_tls->nlog_max_);

    // BODY
    int data_index = 0;
    int buffer_index = 0;
    while (buffer_index < (DEF_LOG_MAX_SIZE - nlog)
           && data_index < n_size) {
      unsigned char c = p_data[data_index];
      slog[nlog + buffer_index++] = HEX_INDEX[c & 0X0F];
      slog[nlog + buffer_index++] = HEX_INDEX[c >> 4];
      slog[nlog + buffer_index++] = ' ';
      data_index++;
    }
    nlog += data_index;
    if (nlog < p_tls->nlog_max_) {
      slog[nlog++] = '\n';
    }

    if (b_local_print == 1 ||
        vzlogging::k_en_stdout) {
      if (nlog < p_tls->nlog_max_) {
        slog[nlog++] = '\0';
      }
      vzlogging::VzDumpLogging(slog, nlog);
    }

    if ((b_local_print == 0) &&
        n_level >= vzlogging::k_shm_arg.GetSendLevel()) {
      return p_tls->Write(vzlogging::k_shm_arg.GetSockAddr(), slog, nlog);
    }
    return 0;
  }
  return -1;
}

/************************************************************************/
/* Description : ע��һ��ι��KEY,������۲������,��������
/* Parameters  : key ������+KEY�γ�Ψһ����,
                     ���Ź�ͨ�����������������ж��Ƿ�ҵ�
                 max_timeout    ���ʱʱ��
                 descrebe       �û�����[MAX:8Byte]
                 descrebe_size  �û�������С
/* Return      : !=NULL ע��ɹ�,==NULL ע��ʧ��
/************************************************************************/
void *RegisterWatchDogKey(const char   *s_descrebe,
                          unsigned int n_descrebe_size,
                          unsigned int n_max_timeout) {
  int n_empty = 0;

  // �ж�ע������
  for (n_empty = 0 ; n_empty < DEF_PER_PRO_WATCHDOG_MAX; n_empty++) {
    if (vzlogging::k_watchdog[n_empty].n_mark == DEF_TAG_MARK &&
        strncmp(vzlogging::k_watchdog[n_empty].s_descrebe, s_descrebe, 8) == 0) {
      return &vzlogging::k_watchdog[n_empty];
    }
  }

  // �����µ�
  for (n_empty = 0 ; n_empty < DEF_PER_PRO_WATCHDOG_MAX; n_empty++) {
    if (vzlogging::k_watchdog[n_empty].n_mark == 0) {
      vzlogging::k_watchdog[n_empty].n_mark = DEF_TAG_MARK;
      vzlogging::k_watchdog[n_empty].n_max_timeout = n_max_timeout;
      vzlogging::k_watchdog[n_empty].n_descrebe_size =
        (n_descrebe_size > DEF_USER_DESCREBE_MAX) ? \
        DEF_USER_DESCREBE_MAX : n_descrebe_size;
      memcpy(vzlogging::k_watchdog[n_empty].s_descrebe, s_descrebe,
             vzlogging::k_watchdog[n_empty].n_descrebe_size);
      return &vzlogging::k_watchdog[n_empty];
    }
  }
  return NULL;
}

/************************************************************************/
/* Description : ι���ӿ�,��ʱ����,�����Ź������жϴ�key����̹߳ҵ�
/* Parameters  : key[IN] ע�ῴ�Ź�ʱʹ�ô����keyֵ
/* Return      : true ι���ɹ�,false ι��ʧ��
/************************************************************************/
int FeedDog(const void *p_arg) {
  vzlogging::TAG_WATCHDOG* p_wdg = (vzlogging::TAG_WATCHDOG*)p_arg;
  if (p_wdg && p_wdg->n_mark == DEF_TAG_MARK) {
    int n_ret = ::VzLog(L_HEARTBEAT, 0, __FILE__, __LINE__,
                        "%s %d %s",
                        vzlogging::k_app_name,
                        p_wdg->n_max_timeout,
                        p_wdg->s_descrebe);
    if (n_ret == 0) {
      return 0;
    }
  }
  return -1;
}
