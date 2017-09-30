/************************************************************************/
/* Copyright@ 2008 vzenith.com                                          */
/* All rights reserved.                                                 */
/* -------------------------------------------------------------------- */
/* Author      : Sober.Peng                                             */
/* Date        : 19:5:2017                                              */
/* Description : 通过共享内存实现进程间日志参数传递                       */
/************************************************************************/
#include "vzlogging/base/vzcommon.h"

#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <io.h>
#include <fcntl.h>
#include <process.h>

#include <windows.h>
#define HDL_NULL  NULL
#else  // WIN32
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef int       HANDLE;
#define HDL_NULL  -1
#endif  // WIN32

bool k_log_print = false;             // 输出使能
char k_app_name[LEN_APP_NAME] = {0};  // 进程名
vzlogging::VShm           k_shm_arg;
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include <process.h>
typedef DWORD             TlsKey;
#define TLS_NULL          0
#else
#include <pthread.h>
typedef pthread_key_t     TlsKey;
#define TLS_NULL          (pthread_key_t)-1
#endif
class VTls {
 public:
  VTls() : tls_key_(TLS_NULL) {
    //VZ_PRINT("%s[%d].\n", __FUNCTION__, __LINE__);
  }
  ~VTls() {
    KeyFree();
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

// 获取线程私有数据
vzlogging::CVzLogSock* GetVzLogSock() {
  static VTls k_tls; // 日志线程私有数据

  k_tls.KeyAlloc();
  if (!k_tls.IsReady()) {
    VZ_ERROR("You can't get tls key.\n");
    return NULL;
  }

  // 从tls中获取私有数据
  vzlogging::CVzLogSock* hdl = (vzlogging::CVzLogSock*)k_tls.GetValue();
  if (NULL == hdl) {
    // 未获取到私有数据,创建之
    vzlogging::CVzLogSock* hdl_log = new vzlogging::CVzLogSock();
    if (hdl_log) {
      hdl_log->InitSocket();
      k_tls.SetValue((void*)hdl_log);
    }

    hdl = (vzlogging::CVzLogSock*)k_tls.GetValue();
  }
  return hdl;
}

namespace vzlogging {

/**VShm******************************************************************/
#ifdef _WIN32
bool VShm::Open(const char* name, int size) {
  shm_id_ = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, (LPCSTR)name);
  if (shm_id_ == NULL) {
    shm_id_ = ::CreateFileMapping(INVALID_HANDLE_VALUE,
                                  NULL,
                                  PAGE_READWRITE,
                                  0,
                                  size,
                                  (LPCSTR)name);
  }
  if (shm_id_ == NULL) {
    perror("OpenFileMapping failed.\n");
    return false;
  }
  printf("shm_id = 0x%x\n", shm_id_);

  shm_ptr_ = vzShmAt();
  if (shm_ptr_ != NULL) {
    shm_size_ = size;
    return true;
  }

  shm_id_ = HDL_NULL;
  return false;
}

void *VShm::vzShmAt() {
  if (shm_id_ == HDL_NULL) {
    return NULL;
  }
  void *ptr = ::MapViewOfFile(shm_id_, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  if (ptr == NULL) {
    printf("MapViewOfFile failed.\n");
    return NULL;
  }
  return ptr;
}

void VShm::vzShmDt(void *ptr) {
  if (ptr) {
    if (FALSE == ::UnmapViewOfFile(ptr)) {
      printf("UnmapViewOfFile failed.\n");
    }
    ptr = NULL;
  }
}

#else

int VShm::create_file(const char *filename) {
  if (access(filename, 0) == 0) {
    return 0;
  }

  if (creat(filename, 0755) < 0) {
    printf("create file %s failure!\n", filename);
    return -1;
  }

  printf("create file %s success!\n", filename);
  return 0;
}

bool VShm::Open(const char* name, int size) {
  create_file(name);
  key_t k = ftok(name, 1);
  if (k < 0) {
    return false;
  }

  shm_id_ = shmget(k, size, IPC_CREAT | 0660);
  if (shm_id_ < 0) {
    shm_id_ = shmget(k, 0, 0);
  }
  if (shm_id_ < 0) {
    return false;
  }

  printf("shm_id = %d, %d\n", shm_id_, k);
  shm_ptr_ = vzShmAt();
  if (shm_ptr_ != NULL) {
    shm_size_ = size;
    return true;
  }

  shm_id_ = HDL_NULL;
  return false;
}

void *VShm::vzShmAt() {
  if (shm_id_ == HDL_NULL) {
    return NULL;
  }
  void *ptr = shmat(shm_id_, 0, 0);
  if (ptr == NULL) {
    printf("shmat failed.\n");
    return NULL;
  }
  return ptr;
}

void VShm::vzShmDt(void *ptr) {
  if (ptr) {
    shmdt(ptr);
    ptr = NULL;
  }
}
#endif

int VShm::GetData(void *pdata, int ndata) {
  void *pshm = vzShmAt();
  if (pshm) {
    int nn = shm_size_ > ndata ? ndata : shm_size_;
    memcpy(pdata, pshm, nn);
    vzShmDt(pshm);
    return nn;
  }
  return -1;
}

int VShm::SetData(const void *pdata, int ndata) {
  void *pshm = vzShmAt();
  if (pshm) {
    int nn = shm_size_ > ndata ? ndata : shm_size_;
    memcpy(pshm, pdata, nn);
    vzShmDt(pshm);
    return nn;
  }
  return -1;
}

//////////////////////////////////////////////////////////////////////////
CVzLogSock::CVzLogSock() : s_(INVALID_SOCKET)
  , max_nlog(A_LOG_SIZE) {
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(DEF_SERVER_PORT);
  addr_.sin_addr.s_addr = inet_addr(DEF_SERVER_HOST);
}

CVzLogSock::~CVzLogSock() {
  if (s_ != INVALID_SOCKET) {
#ifdef WIN32
    closesocket(s_);
#else
    ::close(s_);
#endif
    s_ = INVALID_SOCKET;
  }
}

int CVzLogSock::InitSocket() {
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
  fcntl(s, F_SETFL, mode | O_NONBLOCK);
#endif
  s_ = s;
  return 0;
}

void CVzLogSock::SetRemoteAddr(const char* ip, unsigned short port) {
  addr_.sin_family = AF_INET;
  addr_.sin_port = htons(port);
  addr_.sin_addr.s_addr = inet_addr(ip);
}

int CVzLogSock::SetRemoteAddr(struct sockaddr_in* addr) {
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

int CVzLogSock::Write(struct sockaddr_in* addr, const char* msg, unsigned int size) {
  SetRemoteAddr(addr);

  if (addr_.sin_port == 0 || addr_.sin_addr.s_addr == 0) {
    return -1;
  }
  int ns = ::sendto(s_, msg, size, 0,
                    (sockaddr*)&addr_, sizeof(addr_));
  return ns;
}

//////////////////////////////////////////////////////////////////////////
bool _TAG_WATCHDOG::Init(const char *name, const char* desc,
                         unsigned int ms_timeout) {
  if (name == NULL ||
      desc == NULL ||
      ms_timeout > MAX_WATCHDOG_TIMEOUT) {
    return false;
  }

  mark           = DEF_TAG_MARK;
  timeout        = ms_timeout;
  join_time      = GetSysSec();
  last_heartbeat = GetSysSec();
  strncpy(app_name, name, LEN_APP_NAME);
  strncpy(descrebe, desc, LEN_DESCREBE);
  return true;
}

bool _TAG_WATCHDOG::UpdateTimeout(unsigned int ms_timeout) {
  timeout = ms_timeout;
  return true;
}

bool _TAG_WATCHDOG::Heartbeat() {
  last_heartbeat = GetSysSec();
  return true;
}

bool _TAG_WATCHDOG::isSame(const char *name, const char* desc) {
  if ((mark == DEF_TAG_MARK) &&
      (0 == strncmp(descrebe, desc, LEN_DESCREBE)) &&  // 描述符
      (0 == strncmp(app_name, name, LEN_APP_NAME))) {  // 进程名
    return true;
  }
  return false;
}

bool _TAG_WATCHDOG::isEmpty() {
  if ((mark != DEF_TAG_MARK) && app_name[0] == '\0') {
    return true;
  }
  return false;
}

bool _TAG_WATCHDOG::isTimeout(unsigned int cur_time) {
  if (isEmpty()) {
    return false;
  }

  if ((cur_time - last_heartbeat) > timeout) {
    return true;
  }
  return false;
}

}  // namespace vzlogging
