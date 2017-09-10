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
#include <process.h>
#else  // WIN32
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>
#endif  // WIN32

bool k_log_print = false;             // 输出使能
char k_app_name[LEN_APP_NAME] = {0};  // 进程名
vzlogging::CVzLogShm    k_shm_arg;    // 共享内存
//////////////////////////////////////////////////////////////////////////
#ifdef WIN32
#include <process.h>
typedef DWORD               TlsKey;
#define TLS_NULL            0
#else
#include <pthread.h>
typedef pthread_key_t       TlsKey;
#define TLS_NULL            (pthread_key_t)-1
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
#include <windows.h>
#define HDL_NULL  NULL
HANDLE vzShmOpen(const char* name, int size) {
  HANDLE shm_id = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, (LPCSTR)name);
  if (shm_id == NULL) {
    shm_id = ::CreateFileMapping(INVALID_HANDLE_VALUE,
                                 NULL,
                                 PAGE_READWRITE,
                                 0,
                                 size,
                                 (LPCSTR)name);
  }
  if (shm_id == NULL) {
    printf("OpenFileMapping failed.\n");
    return NULL;
  }

  printf("shm_id = 0x%x\n", shm_id);
  return shm_id;
}

void *vzShmAt(HANDLE shm_id) {
  void *p_ptr = ::MapViewOfFile(shm_id, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  if (p_ptr == NULL) {
    printf("MapViewOfFile failed.\n");
    return NULL;
  }
  return p_ptr;
}

void vzShmDt(void *p_ptr) {
  if (p_ptr) {
    if (FALSE == ::UnmapViewOfFile(p_ptr)) {
      printf("UnmapViewOfFile failed.\n");
    }
    p_ptr = NULL;
  }
}
#else
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

typedef int       HANDLE;
#define HDL_NULL  -1

static int create_file(const char *filename) {
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

HANDLE vzShmOpen(const char* name, int size) {
  create_file(name);
  key_t k = ftok(name, 1);
  if (k < 0) {
    return -2;
  }

  HANDLE shm_id = shmget(k, size, IPC_CREAT | 0660);
  if (shm_id < 0) {
    shm_id = shmget(k, 0, 0);
  }
  if (shm_id < 0) {
    return -1;
  }

  printf("shm_id = %d, %d\n", shm_id, k);
  return shm_id;
}

void *vzShmAt(HANDLE shm_id) {
  void *p_ptr = shmat(shm_id, 0, 0);
  if (p_ptr == NULL) {
    printf("shmat failed.\n");
    return NULL;
  }
  return p_ptr;
}

void vzShmDt(void *p_ptr) {
  if (p_ptr) {
    shmdt(p_ptr);
  }
}
#endif

/**共享内存-参数*********************************************************/
CVzLogShm::CVzLogShm() {
  vshm_  = HDL_NULL;
  valid_ = false;
}

CVzLogShm::~CVzLogShm() {
  Close();
}

bool CVzLogShm::Open() {
#ifdef _WIN32
  const char *shm_file = "./log_shm";
#else
  const char *shm_file = "/dev/shm/log_shm";
#endif
  vshm_ = vzShmOpen(shm_file, sizeof(TAG_SHM_ARG));
  return (vshm_ == HDL_NULL);
}

void CVzLogShm::Close() {
}

int CVzLogShm::Share(const void* data, unsigned int size) {
  void *pshm = vzShmAt(vshm_);
  if (pshm) {
    memcpy(pshm, data, size);
    return 0;
  }
  return -1;
}

unsigned int CVzLogShm::GetLevel() const {
  TAG_SHM_ARG* pArg = (TAG_SHM_ARG*)vzShmAt(vshm_);
  if (pArg) {
    return pArg->snd_level;
  }
  return 3;
}

struct sockaddr_in* CVzLogShm::GetSockAddr() const {
  TAG_SHM_ARG* pArg = (TAG_SHM_ARG*)vzShmAt(vshm_);
  if (pArg) {
    return &pArg->sock_addr;
  }
  return NULL;
}

int CVzLogShm::GetShmArg(TAG_SHM_ARG *arg) {
  void *pshm = vzShmAt(vshm_);
  if (pshm) {
    memcpy(arg, pshm, sizeof(TAG_SHM_ARG));
    return 0;
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

}  // namespace vzlogging
