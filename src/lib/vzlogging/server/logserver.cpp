/************************************************************************/
/* Copyright@ 2008 vzenith.com
/* All rights reserved.
/* ----------------------------------------------------------------------
/* Author      : Sober.Peng
/* Date        : 19:5:2017
/* Description : 日志\看门狗服务器入口
/************************************************************************/
#include <time.h>   // 超时
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/vzlogdef.h"
#include "base/vzshmarg.h"
#include "server/cvzlogserver.h"
#include "server/cvzlogdatapool.h"

static vzlogging::CVzShmArg    k_shm_arg;           // 共享参数
static vzlogging::CVzLogRecord k_log_rec;           // 普通日志
static vzlogging::CVzLogRecord k_wdg_rec;           // 看门狗日志

static vzlogging::CVzSockDgram k_srv_sock;          // 网络服务

static bool                    k_en_stdout = false;  // 使能输出

static struct {
  char          app_name[32];                       //
  unsigned int  pid;                                // 进程ID
  unsigned int  ppid;                               // 心跳线程ID
  unsigned int  no_heart_times;                     // 无心跳次数
  time_t        last_heartbeat;                     // 最好一次心跳时间
  // 注册时把传过来的字符串存储,当进程死亡时打印到文件中,共查错用
  char          msg[80];
} k_pro_heart[DEF_PER_DEV_PROCESS_MAX];             // 进程心跳信息

/***static function******************************************************/
// 共享参数
int InitShareParam(const char*    ip,
                   unsigned short port,
                   unsigned int   snd_level);

// 初始化日志记录
int InitLogRecord(const char* path,
                  const char* log_fname,
                  const char* wdg_fname);

// 初始化网络服务
int InitSrvSocket(const char* ip,
                  unsigned short port,
                  const char* s_snd_addr);

// 超时回调
int callback_timeout();
// 接收回调
int callback_receive(SOCKET             sock,
                     const sockaddr_in* addr,
                     const char*        msg,
                     unsigned int       size);

// 看门狗处理
int WatchdogProcess(const char* msg, unsigned int size);

/************************************************************************/
int main(int argc, char* argv[]) {
#ifdef WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
  srand((unsigned int)time(NULL));
#else
  srand((unsigned int)time(NULL));
  srandom((unsigned int)time(NULL));
#endif
  vzlogging::CVzLogDataPool::InitLogMemPool();
  memset(k_pro_heart, 0, sizeof(k_pro_heart));

  int ret = 0;
  // 传参到客户端,日志上传等级,上传端口号
  unsigned int  n_snd_level     = 3;  
  unsigned int  n_recv_port     = DEF_SERVER_PORT;
  // 转发服务器地址;格式[IP:PORT]
  char          s_snd_addr[32]  = {0};

  const int nLoop  = 10;
  const int nCount = 10000;
  for (int j = 0; j < nCount; ++j) {
    typedef vzlogging::CVzLogDataPool* LPTest;
    LPTest arData[nCount];
    for (int i = 0; i < nLoop; ++i) {
      arData[i] = new vzlogging::CVzLogDataPool();
    }

    for (int i = 0; i < nLoop; ++i) {
      delete arData[i];
    }
  }

  // 传参数
  for (int i = 0; i < argc; i++) {
    if (strlen(argv[i]) >= 2) {
      VZ_PRINT("%s\n", argv[i]);
      switch (argv[i][1]) {
      case 'H':               // 帮助
        printf("applet <-H> <-V[123]> -<P[port]> <-D> <-S[ip:port]>\n");
        break;

      // 打印输出使能
      case 'D':
      case 'd':
        k_en_stdout = true;
        break;

      // 配置透传服务器地址
      case 'S':
        sscanf(argv[i], "-S%s", s_snd_addr);
        break;
      case 's':
        sscanf(argv[i], "-s%s", s_snd_addr);
        break;

      // 配置接收端口
      case 'P':
        sscanf(argv[i], "-P%d", &n_recv_port);
        break;
      case 'p':             //
        sscanf(argv[i], "-p%d", &n_recv_port);
        break;

      // 传输级别
      case 'V':
        scanf(argv[i], "-V%d", &n_snd_level);
        break;
      case 'v':
        sscanf(argv[i], "-v%d", &n_snd_level);
        break;
      }
    }
  }

  // 共享参数
  k_shm_arg.Open();
  ret = InitShareParam(DEF_SERVER_HOST, n_recv_port, n_snd_level);

  // 打开日志存储
  ret = InitLogRecord(DEF_RECORD_PATH,
                      DEF_LOG_REC_FILE,
                      DEF_WDG_REC_FILE);
  if (ret < 0) {
    return ret;
  }
  k_wdg_rec.Print("stdout level %d.\n", n_snd_level);

  // 打开网络监听
  ret = InitSrvSocket("0.0.0.0", n_recv_port, s_snd_addr);
  if (ret < 0) {
    return ret;
  }

  while (true) {
    k_srv_sock.Loop();  // 等待网络接收
  }
  
#ifdef WIN32
  WSACleanup();
#endif
  vzlogging::CVzLogDataPool::ExitLogMemPool();
  return 0;
}

/***static function******************************************************/
// 初始化共享内存
int InitShareParam(const char*    ip,
                   unsigned short port,
                   unsigned int   snd_level) {
  vzlogging::TAG_SHM_ARG arg;

  arg.snd_level                 = snd_level;
  arg.sock_addr.sin_family      = AF_INET;
  arg.sock_addr.sin_port        = htons(port);
  arg.sock_addr.sin_addr.s_addr = inet_addr(ip);

  k_shm_arg.Share(&arg, sizeof(arg));
  k_shm_arg.Share(&arg, sizeof(arg));
  return k_shm_arg.Share(&arg, sizeof(arg));
}

// 初始化日志记录
int InitLogRecord(const char* path,
                  const char* log_fname,
                  const char* wdg_fname) {
  int ret = -1;

  // 普通日志
  ret = k_log_rec.Open(path, log_fname, DEF_LOG_FILE_SIZE);
  if (ret < 0) {
    VZ_PRINT("log record open failed.%d.\n", ret);
    return ret;
  }

  // 看门狗日志
  ret = k_wdg_rec.Open(path, wdg_fname, DEF_WDG_FILE_SIZE);
  if (ret < 0) {
    VZ_PRINT("wdg record open failed.%d.\n", ret);
    return ret;
  }

  return 0;
}

// 初始化网络服务
int InitSrvSocket(const char* ip, unsigned short port,
                  const char* s_snd_addr) {
  k_srv_sock.SetCallback(callback_receive,
                         callback_timeout);

  int ret = -1;
  // 打开网络监听
  ret = k_srv_sock.OpenListenAddr(ip, port);
  if (ret < 0) {
    VZ_PRINT("open udp listen %d failed. %d.\n", port, ret);
    return ret;
  }

  ret = k_srv_sock.OpenTransAddr(s_snd_addr);
  if (ret < 0) {
    VZ_PRINT("open udp trans %s failed. %d.\n", s_snd_addr, ret);
  }
  return 0;
}

// 接收回调
int callback_receive(SOCKET             sock,
                     const sockaddr_in* sa_remote,
                     const char*        s_msg,
                     unsigned int       n_msg) {
  if (k_en_stdout) {
    if (n_msg < DEF_LOG_MAX_SIZE) {
      ((char*)s_msg)[n_msg] = '\0';
    }
    printf(s_msg);
  }

  if (s_msg[0] != 'H') {
    // 普通日志
    k_log_rec.Write(s_msg, n_msg);
  } else {
    // 心跳,发送给看门狗处理
    WatchdogProcess(s_msg, n_msg);
  }

  // 透传接收到的日志信息
  k_srv_sock.TransMsgToServer(s_msg, n_msg);
  return n_msg;
}

// 超时回调
int callback_timeout() {
  time_t n_now = time(NULL);
  static time_t n_last = n_now;
  if ((n_now - n_last) > 10) {
    // 进程超时检查
    for (int i = 0; i < DEF_PER_DEV_PROCESS_MAX; i++) {
      if (k_pro_heart[i].app_name[0] != '\0') {
        if ((n_now - k_pro_heart[i].last_heartbeat) > 45) {
          if ((k_pro_heart[i].no_heart_times++) >= 2) {
            k_wdg_rec.Print("%s %d %d no heartbeat, %s.\n",
                            k_pro_heart[i].app_name,
                            k_pro_heart[i].pid,
                            k_pro_heart[i].ppid,
                            k_pro_heart[i].msg);
            // 重启进程
            // 重启设备
          }
          k_pro_heart[i].last_heartbeat = n_now;
        }
      }
    }

    n_last = n_now;
  }

  k_log_rec.Sync();
  k_wdg_rec.Sync();
  return 0;
}

// 看门狗处理
int WatchdogProcess(const char* s_msg, unsigned int n_msg) {
  unsigned int pid, ppid;
  char s_app_name[32] = {0};
  sscanf(s_msg, "H %04d %04d %s ", &pid, &ppid, s_app_name);

  // 更新进程心跳时间
  int n_empty = 0;  // 未使用进程信息空间
  for (int i = 0; i < DEF_PER_DEV_PROCESS_MAX; i++) {
    if (k_pro_heart[i].ppid == ppid &&                      // 线程&&进程名
        !strncmp(k_pro_heart[i].app_name, s_app_name, 32)) {
      k_pro_heart[i].no_heart_times = 0;
      k_pro_heart[i].last_heartbeat = time(NULL);
      return 0;
    }

    if (k_pro_heart[i].app_name[0] == '\0') {
      n_empty = i;
    }
  }

  // 新进程
  strncpy(k_pro_heart[n_empty].app_name, s_app_name, 32);
  k_pro_heart[n_empty].pid  = pid;
  k_pro_heart[n_empty].ppid = ppid;
  k_pro_heart[n_empty].no_heart_times = 0;
  k_pro_heart[n_empty].last_heartbeat = time(NULL);
  char* token = strrchr((char*)s_msg, ']');
  if (token) {
    memcpy(k_pro_heart[n_empty].msg, token + 2, 80);
  }
  k_wdg_rec.Print("%s %d %d register.\n", s_app_name, pid, ppid);
  return 0;
}

