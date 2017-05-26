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

#include "base/flags/yhflags.h"

#include "vzlogging/base/vzlogdef.h"
#include "vzlogging/base/vzshmarg.h"
#include "vzlogging/include/vzlogging.h"
#include "vzlogging/server/cvzlogserver.h"

static vzlogging::CVzShmArg       k_shm_arg;      // 共享参数
static vzlogging::CVzLoggingFile  k_log_file;     // 普通日志
static vzlogging::CVzWatchdogFile k_wdg_file;     // 看门狗日志
static vzlogging::CVzSockDgram    k_srv_sock;     // 网络服务
static bool                       k_en_stdout = false;   // 使能输出

/***进程心跳信息*********************************************************/
typedef struct _TAG_WATCHDOG {
  unsigned int  n_mark;
  char          s_app_name[DEF_PROCESS_NAME_MAX];     // 进程名
  char          s_descrebe[DEF_USER_DESCREBE_MAX+4];  // 用户描述
  unsigned int  n_timeout;                            // 超时
  time_t        last_heartbeat;                       // 最好一次心跳时间
} TAG_HEARTBEAT;
static TAG_HEARTBEAT k_pro_heart[DEF_PER_DEV_PROCESS_MAX];  // 进程心跳信息

/***static function******************************************************/
/*共享参数*/
int InitShareParam(const char*    ip,
                   unsigned short port,
                   unsigned int   snd_level);

/*初始化日志记录*/
int InitLogRecord(const char* path,
                  const char* log_fname,
                  const char* wdg_fname);

/*初始化网络服务*/
int InitSrvSocket(const char* ip,
                  unsigned short port,
                  const char* s_snd_addr);

/*超时回调*/
int callback_timeout();
/*接收回调*/
int callback_receive(SOCKET             sock,
                     const sockaddr_in* addr,
                     const char*        msg,
                     unsigned int       size);

/*看门狗处理*/
int WatchdogProcess(const char* msg, unsigned int size);

/*当模块丢失心跳时调用*/
int OnModuleLostHeartbeat(time_t n_now);

static void PrintUsage() {
  printf("\n");
  printf("\n");
  printf("  usage:\n");
  printf("\n");
  printf(
    "      -v  <trans level 123> Defualt value is 3 L_ERROR send to server\n");
  printf(
    "      -p  <trans port> Defualt value is 5760\n");
  printf(
    "      -s  <IP:PORT> remote server address\n");
  printf(
    "      -d  print all log in console\n");
  printf(
    "      -h  print usage\n");
  printf("\n");
  exit(1);
}

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
  memset(k_pro_heart, 0, sizeof(k_pro_heart));

  int ret = 0;
  // 传参到客户端,日志上传等级,上传端口号
  unsigned int  n_snd_level = 3;
  unsigned int  n_recv_port = DEF_SERVER_PORT;
  // 转发服务器地址;格式[IP:PORT]
  char          s_snd_addr[32] = { 0 };
  char          s_log_path[64] = { 0 };
  strncpy(s_log_path, DEF_RECORD_PATH, 64);  // 默认路径

  int opt = 0;
  while ((opt = getopt(argc, argv, "v:V:p:P:s:S:dDhH")) != -1) {
    switch (opt) {
    case 'v':   // 配置网络输出级别
    case 'V':
      n_snd_level = atoi(optarg);
      break;

    case 'p':   // 配置网络传输端口
    case 'P':
      n_recv_port = atoi(optarg);
      break;

    case 's':   // 配置转发服务器地址
    case 'S': {
      memset(s_log_path, 0, 64);
      strncpy(s_log_path, optarg, 64);
    }
    break;

    case 'd':   // 配置本地打印
    case 'D':
      k_en_stdout = true;
      break;

    case 'h':   // 帮助
    case 'H':
      PrintUsage();
      break;
    }
  }

  // 共享参数
  k_shm_arg.Open();
  ret = InitShareParam(DEF_SERVER_HOST, n_recv_port, n_snd_level);

  // 打开日志存储
  ret = InitLogRecord(s_log_path,
                      DEF_LOG_REC_FILE,
                      DEF_WDG_REC_FILE);
  if (ret < 0) {
    VZ_PRINT("Open record failed. path %s, log file %s, watchdog file %s\n",
             s_log_path, DEF_LOG_REC_FILE, DEF_WDG_REC_FILE);
    return ret;
  }
  VZ_PRINT("send log level %d.\n", n_snd_level);

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
  return 0;
}

/***static function******************************************************/
/*初始化共享内存*/
int InitShareParam(const char*    ip,
                   unsigned short port,
                   unsigned int   snd_level) {
  vzlogging::TAG_SHM_ARG arg;

  arg.snd_level = snd_level;
  arg.sock_addr.sin_family = AF_INET;
  arg.sock_addr.sin_port = htons(port);
  arg.sock_addr.sin_addr.s_addr = inet_addr(ip);

  k_shm_arg.Share(&arg, sizeof(arg));
  k_shm_arg.Share(&arg, sizeof(arg));
  return k_shm_arg.Share(&arg, sizeof(arg));
}

/*初始化日志记录*/
int InitLogRecord(const char* path,
                  const char* log_fname,
                  const char* wdg_fname) {
  int ret = -1;

  // 普通日志
  ret = k_log_file.Open(path, log_fname, DEF_LOG_FILE_SIZE);
  if (ret < 0) {
    VZ_PRINT("log record open failed.%d.\n", ret);
    return ret;
  }
  k_log_file.StartRecord("program log");

  // 看门狗日志
  ret = k_wdg_file.Open(path, wdg_fname, DEF_WDG_FILE_SIZE);
  if (ret < 0) {
    VZ_PRINT("wdg record open failed.%d.\n", ret);
    return ret;
  }
  k_wdg_file.StartRecord("watchdog");

  return 0;
}

/*初始化网络服务*/
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

static int k_total_log = 0;
/*接收回调*/
int callback_receive(SOCKET             sock,
                     const sockaddr_in* sa_remote,
                     const char*        s_msg,
                     unsigned int       n_msg) {
  if (k_en_stdout) {
    if (n_msg < DEF_LOG_MAX_SIZE) {
      ((char*)s_msg)[n_msg] = '\0';
    }
    // printf(s_msg);
    vzlogging::VzDumpLogging(s_msg, (int)n_msg);
    fflush(stdout);
  }

  if (s_msg[0] != L_C_HEARTBEAT) {
    // 普通日志
    k_log_file.Write(s_msg, n_msg);
  } else {
    // 心跳,发送给看门狗处理
    WatchdogProcess(s_msg, n_msg);
  }

  // 透传接收到的日志信息
  k_srv_sock.TransMsgToServer(s_msg, n_msg);

  k_total_log += n_msg;
  return n_msg;
}

/*超时回调*/
int callback_timeout() {
  time_t n_now = time(NULL);
  static time_t n_last = n_now;
  if ((n_now - n_last) > 1) {  // 1S检查一次
    // 进程超时检查
    for (int i = 0; i < DEF_PER_DEV_PROCESS_MAX; i++) {
      if (k_pro_heart[i].n_mark == DEF_TAG_MARK
          && k_pro_heart[i].s_app_name[0] != '\0') {
        unsigned int n_diff_time = abs(n_now - k_pro_heart[i].last_heartbeat);
        if (n_diff_time > k_pro_heart[i].n_timeout) {
          OnModuleLostHeartbeat(n_now);

          memset(&k_pro_heart[i], 0, sizeof(k_pro_heart[i]));
          break;
        }
      }
    }

    n_last = n_now;
  }

  k_log_file.Sync();
  k_wdg_file.Sync();
  return 0;
}

/*看门狗处理*/
int WatchdogProcess(const char* s_msg, unsigned int n_msg) {
  const char* p_usr_msg = strrchr(s_msg, ']');
  if (p_usr_msg == NULL) {
    VZ_PRINT("can't find ']' from heartbeat message.\n");
    return -1;
  }

  // 格式化获取字符串数据
  unsigned int n_timeout = 0;
  char s_app_name[DEF_PROCESS_NAME_MAX]  = {0};
  char s_descrebe[DEF_USER_DESCREBE_MAX] = {0};
  sscanf(p_usr_msg+2, "%s %d %s",
         s_app_name, &n_timeout, s_descrebe);

  int n_empty = -1;  // 未使用进程信息空间

  // 更新进程心跳时间
  for (int i = 0; i < DEF_PER_DEV_PROCESS_MAX; i++) {
    if (k_pro_heart[i].n_mark == DEF_TAG_MARK &&
        !strncmp(k_pro_heart[i].s_descrebe,
                 s_descrebe, DEF_USER_DESCREBE_MAX)       // 描述符
        && !strncmp(k_pro_heart[i].s_app_name,
                    s_app_name, DEF_PROCESS_NAME_MAX)) {  // 进程名
      k_pro_heart[i].last_heartbeat = time(NULL);
      return 0;
    }

    if (n_empty == -1 &&
        k_pro_heart[i].n_mark == 0) {
      n_empty = i;
    }
  }

  //
  if (n_empty == -1) {
    VZ_PRINT("not empty space to save heartbeat infomation.\n");
    return -2;
  }

  // 新进程
  k_pro_heart[n_empty].n_mark         = DEF_TAG_MARK;
  strncpy(k_pro_heart[n_empty].s_app_name, s_app_name, DEF_PROCESS_NAME_MAX);
  strncpy(k_pro_heart[n_empty].s_descrebe, s_descrebe, DEF_USER_DESCREBE_MAX);
  k_pro_heart[n_empty].n_timeout      = n_timeout;
  k_pro_heart[n_empty].last_heartbeat = time(NULL);
  VZ_PRINT("%s %s register.\n", s_app_name, s_descrebe);
  return 0;
}

int OnModuleLostHeartbeat(time_t n_now) {
  // 排序
  TAG_HEARTBEAT c_heart;
  for (int i = 0; i < DEF_PER_DEV_PROCESS_MAX; i++) {
    for (int j = i + 1; j < DEF_PER_DEV_PROCESS_MAX - 1; j++) {
      if (strncmp(k_pro_heart[i].s_app_name,
                  k_pro_heart[j].s_app_name, DEF_PROCESS_NAME_MAX) < 0) {
        memcpy(&c_heart, &k_pro_heart[i], sizeof(TAG_HEARTBEAT));
        if (k_pro_heart[i].n_mark == DEF_TAG_MARK) {
          memcpy(&k_pro_heart[i], &k_pro_heart[j], sizeof(TAG_HEARTBEAT));
        }
        if (c_heart.n_mark == DEF_TAG_MARK) {
          memcpy(&k_pro_heart[j], &c_heart, sizeof(TAG_HEARTBEAT));
        }
      }
    }
  }

  // 格式化输出
  int  n_log = 0, n_1_log = 0;
  char s_log[DEF_LOG_MAX_SIZE] = { 0 };
  n_1_log = n_log = snprintf(s_log,
                             DEF_LOG_MAX_SIZE,
                             "watchdog danager ");
  for (int j = 0; j < DEF_PER_DEV_PROCESS_MAX; j++) {
    if (k_pro_heart[j].n_mark == DEF_TAG_MARK
        && k_pro_heart[j].s_app_name[0] != '\0') {
      char *p_fmt = "--%s-%s[%d]";
      if (n_1_log == n_log) {  // 只有开始数据,第一包不包含"--"
        p_fmt = "%s-%s[%d]";
      }
      n_log += snprintf(s_log + n_log, DEF_LOG_MAX_SIZE - n_log,
                        p_fmt,
                        k_pro_heart[j].s_app_name,
                        k_pro_heart[j].s_descrebe,
                        (n_now - k_pro_heart[j].last_heartbeat));
    }
  }
  if (n_log < DEF_LOG_MAX_SIZE) {
    s_log[n_log++] = '\n';
  }
  VZ_PRINT("%s", s_log);
  k_wdg_file.Write(s_log, n_log);
  k_wdg_file.StartRecord("watchdog");

  // 日志文件转存
  k_log_file.OnModuleLostHeartbeat(s_log, n_log);

  // 重启设备
  return 0;
}

