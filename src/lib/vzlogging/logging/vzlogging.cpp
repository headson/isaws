/************************************************************************/
/* Copyright@ 2008 vzenith.com
/* All rights reserved.
/* ----------------------------------------------------------------------
/* Author      : Sober.Peng
/* Date        : 19:5:2017
/* Description :
/************************************************************************/
#include "vzlogging/include/vzlogging.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "vzlogging/base/vzlogdef.h"
#include "vzlogging/base/vzshmarg.h"
#include "vzlogging/include/vzwatchdog.h"

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
static void*    k_tls_void[DEF_PER_PRO_THREAD_MAX];   // 私有数据指针
/**VTls******************************************************************/
/**日志线程私有数据******************************************************/
class CTlsLog {
 public:
  CTlsLog()
    : s_(INVALID_SOCKET)
    , nlog_max_(DEF_LOG_MAX_SIZE) {
    VZ_PRINT("%s[%d].\n", __FUNCTION__, __LINE__);
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
    VZ_PRINT("%s[%d].\n", __FUNCTION__, __LINE__);
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

  // 地址不合法不发送日志
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
      VZ_PRINT("sendto log failed.\n");
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
  char                slog_[DEF_LOG_MAX_SIZE + 1];  // 日志使用buffer
};

class VTls {
 public:
  VTls() : tls_key_(TLS_NULL) {
    VZ_PRINT("%s[%d].\n", __FUNCTION__, __LINE__);
  }
  ~VTls() {
    KeyFree();
    for (int i = 0; i < DEF_PER_PRO_THREAD_MAX; i++) {
      if (k_tls_void[i] != NULL) {
        delete (CTlsLog*)k_tls_void[i];
        k_tls_void[i] = NULL;
      }
    }
    VZ_PRINT("%s[%d].\n", __FUNCTION__, __LINE__);
  }

  // 启动进程时调用一次，分配关键字
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

  // true可用,false不可用
  bool   IsReady() {
    if (TLS_NULL != tls_key_) {
      return true;
    }
    return false;
  }

  // 线程中调用，关联关键字和私有数据对应指针
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
  // 线程中调用，获取关键字关联的线程私有数据
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

// 看门狗结构
typedef struct {
  unsigned int n_mark;                                  // 校验字
  unsigned int n_max_timeout;                           // 最大超时时长
  char         s_descrebe[DEF_USER_DESCREBE_MAX+4];     // 用户描述
  unsigned int n_descrebe_size;                         // 用户描述大小
} TAG_WATCHDOG;

/**static value**********************************************************/
static char           k_app_name[DEF_PROCESS_NAME_MAX] = { 0 };  // 进程名

static CVzShmArg      k_shm_arg;                      // 共享内存

static VTls           k_tls_log;                      // 日志线程私有数据

static TAG_WATCHDOG   k_watchdog[DEF_PER_PRO_WATCHDOG_MAX];  // 看门狗结构

static bool           k_en_stdout = false;            // 输出使能
static unsigned int   k_snd_level = L_ERROR;          // 发送等级

/**静态函数**************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

  // 获取线程私有数据
  static vzlogging::CTlsLog* GetTlsLog() {
    k_tls_log.KeyAlloc();

    if (!k_tls_log.IsReady()) {
      VZ_PRINT("You can't get tls key.\n");
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

/**日志流格式化**********************************************************/
CVzLogStream::CVzLogStream(int          n_level,
                           const char*  p_file,
                           int          n_line,
                           bool         b_print) {
  b_print_ = b_print;
  n_level_ = n_level;
  p_tls_ = GetTlsLog();

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
    // 使能输出
    if ((b_print_ == true) || k_en_stdout) {
      if (tls_log->nlog_ < tls_log->nlog_max_) {
        tls_log->slog_[tls_log->nlog_++] = '\0';
      }
      printf(tls_log->slog_);
    }

    if ((b_print_ == false) &&
        (n_level_ >= k_shm_arg.GetSendLevel())) {
      tls_log->Write(k_shm_arg.GetSockAddr(), tls_log->slog_, tls_log->nlog_);
    }
  }
}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(const char* t) {
  if (p_tls_ && t) {
    CTlsLog* tls_log = (CTlsLog*)p_tls_;
    if (tls_log->nlog_ < 0) {
      return *this;
    }

    tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                               tls_log->nlog_max_ - tls_log->nlog_,
                               "%s", t);
  }
  return *this;
}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(const char t) {
  if (p_tls_) {
    CTlsLog* tls_log = (CTlsLog*)p_tls_;
    if (tls_log->nlog_ < 0) {
      return *this;
    }

    tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                               tls_log->nlog_max_ - tls_log->nlog_,
                               "%c", t);
  }
  return *this;
}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(const double t) {
  if (p_tls_) {
    CTlsLog* tls_log = (CTlsLog*)p_tls_;
    if (tls_log->nlog_ < 0) {
      return *this;
    }

    tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                               tls_log->nlog_max_ - tls_log->nlog_,
                               "%f", t);
  }
  return *this;
}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(const unsigned long long t) {
  if (p_tls_) {
    CTlsLog* tls_log = (CTlsLog*)p_tls_;
    if (tls_log->nlog_ < 0) {
      return *this;
    }

    tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                               tls_log->nlog_max_ - tls_log->nlog_,
                               "%llu", t);
  }
  return *this;
}

#ifndef WIN32
vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(const long long t) {
  if (p_tls_) {
    CTlsLog* tls_log = (CTlsLog*)p_tls_;
    if (tls_log->nlog_ < 0) {
      return *this;
    }

    tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                               tls_log->nlog_max_ - tls_log->nlog_,
                               "%lld", t);
  }
  return *this;
}
#endif  // WIN32

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(const unsigned int t) {
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
vzlogging::CVzLogStream::operator<<(const int t) {
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
  vzlogging::CVzLogStream::operator<<(const time_t tt) {
  if (p_tls_) {
    CTlsLog* tls_log = (CTlsLog*)p_tls_;
    if (tls_log->nlog_ < 0) {
      return *this;
    }

    tls_log->nlog_ += snprintf(tls_log->slog_ + tls_log->nlog_,
                               tls_log->nlog_max_ - tls_log->nlog_,
                               "%d", tt);
  }
  return *this;
}

vzlogging::CVzLogStream&
  vzlogging::CVzLogStream::operator<<(const std::string str) {
  if (p_tls_) {
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

vzlogging::CVzLogStream& 
  vzlogging::CVzLogStream::write(const char* s_msg, int n_msg) {
  if (p_tls_ && s_msg && n_msg > 0) {
    CTlsLog* tls_log = (CTlsLog*)p_tls_;
    if (tls_log->nlog_ < 0) {
      return *this;
    }

    int n_left = tls_log->nlog_max_ - tls_log->nlog_; // 剩余空间
    int n_less = n_left > n_msg ? n_msg : n_left;     // 消息与剩余字符串那个小

    memcpy(tls_log->slog_ + tls_log->nlog_, s_msg, n_less);
    tls_log->nlog_ += n_less;
  }
  return *this;
}

/**日志对外接口**********************************************************/
int InitVzLogging(int argc, char* argv[]) {
#ifdef WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
  srand((unsigned int)time(NULL));
#else
  srand((unsigned int)time(NULL));
  srandom((unsigned int)time(NULL));
#endif
  memset(k_tls_void, 0, sizeof(k_tls_void));
  memset(k_watchdog, 0, sizeof(k_watchdog));

  // 进程名
  memcpy(k_app_name, GetFileName(argv[0]), 31);
  VZ_PRINT("this applet name is: %s.\n", k_app_name);

  // 传参数
  for (int i = 0; i < argc; i++) {
    if (strlen(argv[i]) >= 2) {
      switch (argv[i][1]) {
      case 'v':           // 打印输出使能
      case 'V':
        k_en_stdout = true;
        break;
      }
    }
  }

  // 打开共享内存
  int ret = k_shm_arg.Open();
  if (ret != 0) {
    VZ_PRINT("share memory failed. %d.\n", ret);
  }
  VZ_PRINT("share memory success. level %d.\n", k_shm_arg.GetSendLevel());
  return ret;
}

/***销毁日志库***********************************************************/
void ExitVzLogging() {
#ifdef WIN32
  WSACleanup();
#endif

  k_shm_arg.Close();
  k_tls_log.KeyFree();
  VZ_PRINT("send message size %d.\n", k_total_log);
}

/************************************************************************/
/* Description : 打印日志
/* Parameters  : level 日志等级,
file  调用此函数文件,
line  调用此函数文件行,
fmt   格式化字符串,
...   不定长参数
/* Return      : 0 success,-1 failed
/************************************************************************/
int VzLog(unsigned int  level,
          bool          b_print,
          const char    *file,
          int           line,
          const char    *fmt,
          ...) {
  CTlsLog* tls_log = GetTlsLog();
  if (tls_log) {
    char* slog = tls_log->slog_;
    int&  nlog = tls_log->nlog_;

    // HEAD
    nlog = VzLogPackHead(level, file, line, slog, tls_log->nlog_max_);
    if (nlog < 0) {
      return nlog;
    }

    // BODY
    va_list args;
    va_start(args, fmt);
    nlog += vsnprintf(slog + nlog, tls_log->nlog_max_ - nlog, fmt, args);
    va_end(args);

    if (nlog < tls_log->nlog_max_) {
      slog[nlog++] = '\n';
    }

    // 使能输出
    if (b_print || k_en_stdout) {
      if (nlog < tls_log->nlog_max_) {
        slog[nlog] = '\0';
      }
      VzDumpLogging(slog, nlog);
    }

    if ((b_print == false) &&
        level >= k_shm_arg.GetSendLevel()) {
      return tls_log->Write(k_shm_arg.GetSockAddr(), slog, nlog);
    }
    return 0;
  }
  return -1;
}

int VzLogBin(unsigned int level,
           bool         b_print,
           const char   *file,
           int          line,
           const char   *data,
           int          size) {
  static const char HEX_INDEX[] = {
    '0', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', 'A', 'B',
    'C', 'D', 'E', 'F'
  };
  CTlsLog* tls_log = GetTlsLog();
  if (tls_log) {
    char* slog = tls_log->slog_;
    int& nlog  = tls_log->nlog_;

    // HEAD
    nlog = VzLogPackHead(level, file, line, slog, tls_log->nlog_max_);

    // BODY
    int data_index = 0;
    int buffer_index = 0;
    while (buffer_index < (DEF_LOG_MAX_SIZE - nlog)
           && data_index < size) {
      unsigned char c = data[data_index];
      slog[nlog + buffer_index++] = HEX_INDEX[c & 0X0F];
      slog[nlog + buffer_index++] = HEX_INDEX[c >> 4];
      slog[nlog + buffer_index++] = ' ';
      data_index++;
    }
    nlog += data_index;
    if (nlog < tls_log->nlog_max_) {
      slog[nlog++] = '\n';
    }

    if (b_print || k_en_stdout) {
      if (nlog < tls_log->nlog_max_) {
        slog[nlog++] = '\0';
      }
      VzDumpLogging(slog, nlog);
    }

    if ((b_print == false) &&
        level >= k_shm_arg.GetSendLevel()) {
      return tls_log->Write(k_shm_arg.GetSockAddr(), slog, nlog);
    }
    return 0;
  }
  return -1;
}

/************************************************************************/
/* Description : 注册一个喂狗KEY,并传入观察进程名,进程描述
/* Parameters  : key 进程名+KEY形成唯一主键, 
                     看门狗通过监听此主键心跳判断是否挂掉
                 max_timeout    最大超时时间
                 descrebe       用户描述[MAX:8Byte]
                 descrebe_size  用户描述大小
/* Return      : !=NULL 注册成功,==NULL 注册失败
/************************************************************************/
void *RegisterWatchDogKey(const char   *s_descrebe,
                          unsigned int n_descrebe_size,
                          unsigned int n_max_timeout) {
  int n_empty = 0;
  for ( ; n_empty < DEF_PER_PRO_WATCHDOG_MAX; n_empty++) {
    if (k_watchdog[n_empty].n_mark == 0) {
      k_watchdog[n_empty].n_mark = DEF_TAG_MARK;
      k_watchdog[n_empty].n_max_timeout = n_max_timeout;
      k_watchdog[n_empty].n_descrebe_size =
        (n_descrebe_size > DEF_USER_DESCREBE_MAX) ? \
          DEF_USER_DESCREBE_MAX : n_descrebe_size;
      memcpy(k_watchdog[n_empty].s_descrebe, s_descrebe,
             k_watchdog[n_empty].n_descrebe_size);
      return &k_watchdog[n_empty];
    }
  }
  return NULL;
}

/************************************************************************/
/* Description : 喂狗接口,定时调用,否则看门狗服务判断此key相关线程挂掉
/* Parameters  : key[IN] 注册看门狗时使用传入的key值
/* Return      : true 喂狗成功,false 喂狗失败
/************************************************************************/
bool FeedDog(const void *p_arg) {
  TAG_WATCHDOG* p_wdg = (TAG_WATCHDOG*)p_arg;
  if (p_wdg && p_wdg->n_mark == DEF_TAG_MARK) {
    int n_ret = VzLog(L_HEARTBEAT, false, __FILE__, __LINE__,
                      "%s %d %s",
                      k_app_name,
                      p_wdg->n_max_timeout,
                      p_wdg->s_descrebe);
    if (n_ret == 0) {
      return true;
    }
  }
  return false;
}

}  // namespace vzlogging
