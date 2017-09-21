/************************************************************************
* Author      : SoberPeng 2017-09-08
* Description : 日志基础库
************************************************************************/
#ifndef LIBVZLOGGING_VZCOMMON_H
#define LIBVZLOGGING_VZCOMMON_H

#include "vzbases.h"
#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#define HDL_NULL  NULL
#else   // LINUX
#include <fcntl.h>
#include <unistd.h>

#include <sys/time.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#ifndef INVALID_SOCKET
#define INVALID_SOCKET  -1
#endif  // INVALID_SOCKET

typedef int       HANDLE;
#define HDL_NULL  -1
#endif  // LINUX

namespace vzlogging {

/**共享内存-参数*********************************************************/
typedef struct _TAG_WATCHDOG {
  unsigned int  mark;

  char          app_name[LEN_APP_NAME];    // 进程名
  char          descrebe[LEN_DESCREBE];    // 用户描述
  unsigned int  join_time;                 // 加入组时间

  unsigned int  status;                    // 

  unsigned int  timeout;                   // 超时
  
  unsigned int  last_heartbeat;            // 最好一次心跳时间
} TAG_MODULE_STATE;

typedef struct {
  unsigned int        snd_level;    // 日志打印级别
  struct sockaddr_in  sock_addr;    //

  unsigned int        checksum;     // 校验;校验共享内存为被串改

  TAG_MODULE_STATE    mod_state[MAX_WATCHDOG_A_DEVICE];
} TAG_SHM_ARG;

class VShm;
class CVzLogShm {
 public:
  CVzLogShm();
  virtual ~CVzLogShm();

  // 打开
  bool  Open();

  int GetShmArg(TAG_SHM_ARG *arg);
  int SetShmArg(const TAG_SHM_ARG *arg);

  // 获取参数
  unsigned int        GetLevel() const;
  struct sockaddr_in* GetSockAddr() const;

 private:
  VShm*       vshm_;
  bool        valid_;   // 有意义的传参
};

/**日志线程私有数据*******************************************************/
class CVzLogSock {
public:
  CVzLogSock();
  ~CVzLogSock();

  int InitSocket();

  void SetRemoteAddr(const char* ip, unsigned short port);
  int SetRemoteAddr(struct sockaddr_in* addr);              // 地址不合法不发送日志

  int Write(struct sockaddr_in* addr, const char* msg, unsigned int size);

private:
  SOCKET              s_;
  struct sockaddr_in  addr_;

public:
  const int           max_nlog;
  int                 nlog;
  char                slog[A_LOG_SIZE + 4];  // 日志使用buffer
};

}  // namespace vzlogging

extern bool k_log_print;                        // 输出使能
extern char k_app_name[LEN_APP_NAME];           // 进程名
extern vzlogging::CVzLogShm   k_shm_arg;        // 共享内存
extern vzlogging::CVzLogSock *GetVzLogSock();   // 获取日志SOCKET句柄(tls)

#endif  // LIBVZLOGGING_VZCOMMON_H
