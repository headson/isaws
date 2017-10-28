/************************************************************************
*Author      : Sober.Peng 17-08-22
*Description :
************************************************************************/
#include "cvzlogmanage.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#ifndef _WIN32
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include "vzlogging/base/vzbases.h"

namespace vzlogging {

CVzLogManage::CVzLogManage() {
  is_stdout_ = 0;
  is_reboot_ = 1;

  shm_mod_   = NULL;

  stride_watch_time_ = 0;
}

CVzLogManage::~CVzLogManage() {
}

CVzLogManage *CVzLogManage::Instance() {
  static CVzLogManage mgr;

  return &mgr;
}

int CVzLogManage::Start(const char* log_path,
                        unsigned short log_port,
                        const char* tran_addr) {
  int res = 0;
#ifndef _WIN32
  mkdir("/dev/shm", 0777);
#endif
  if (!k_shm_arg.Open(DEF_WDG_SHM_ARG, sizeof(TAG_SHM_ARG))) {
    VZ_ERROR("share memory open failed.\n");
    return -1;
  }
  shm_mod_ = (TAG_SHM_ARG*)k_shm_arg.GetData();
  if (shm_mod_ == NULL) {
    VZ_ERROR("get share arg failed.");
    return -1;
  }

  /* 打开日志记录 */
  res = InitLogRecord(log_path, DEF_LOG_REC_FILE, DEF_WDG_REC_FILE);
  if (res < 0) {
    VZ_ERROR("Open record failed. path %s, log file %s, watchdog file %s.\n",
             log_path, DEF_LOG_REC_FILE, DEF_WDG_REC_FILE);
  }

  /* 初始化看门狗监控模块 */
  res = InitMonitorModule(DEF_WDG_MODULE_FILE);
  if (res < 0) {
    VZ_ERROR("Open monitor %s failed.%d.\n", DEF_WDG_MODULE_FILE);
  }

  /* 打开网络监听 */
  res = InitSrvSocket("0.0.0.0", log_port, tran_addr);
  if (res < 0) {
    VZ_ERROR("Open log port %d failed.%d.\n", log_port, res);
    return res;
  }
  return 0;
}

void CVzLogManage::RunLoop(int ms) {
  log_srv_.Loop(ms);
}

void CVzLogManage::SetLogAddrAndLevel(const char *lhost, unsigned short port,
                                      unsigned int level) {
  if (shm_mod_ == NULL) {
    return;
  }

  shm_mod_->snd_level = level;

  shm_mod_->sock_addr.sin_family      = AF_INET;
  shm_mod_->sock_addr.sin_port        = htons(port);
  shm_mod_->sock_addr.sin_addr.s_addr = inet_addr(lhost);
}

void CVzLogManage::SetEnableStdout() {
  is_stdout_ = 1;
}

void CVzLogManage::SetDisableWatchdog() {
  is_reboot_ = 0;
}

int CVzLogManage::InitLogRecord(const char* path,
                                const char* log_fname, const char* wdg_fname) {
  int ret = -1;

  // 普通日志
  ret = log_file_.Open(path, log_fname, MAX_LOG_FILE_SIZE);
  if (ret < 0) {
    VZ_ERROR("log record open failed.%d.\n", ret);
    return ret;
  }
  log_file_.WriteSome("program log");

  // 看门狗日志
  ret = wdg_file_.Open(path, wdg_fname, MAX_WDG_FILE_SIZE);
  if (ret < 0) {
    VZ_ERROR("wdg record open failed.%d.\n", ret);
    return ret;
  }
  wdg_file_.WriteSome("watchdog start");

  return 0;
}

int CVzLogManage::InitMonitorModule(const char* s_path) {
  if (shm_mod_ == NULL) {
    return -1;
  }
  memset(shm_mod_->mod_state, 0, sizeof(shm_mod_->mod_state));

  FILE* file = fopen(s_path, "rt");
  if (file) {
    int  n_line = 0;
    char s_line[128 + 1];
    do {
      memset(s_line, 0, 128);
      fgets(s_line, 127, file);
      if ((n_line = strlen(s_line)) <= 0) {
        break;
      }
      if (s_line[0] == '#') {
        continue;
      }

      for (int i = 0; i < MAX_WATCHDOG_A_DEVICE; i++) {
        if (shm_mod_->mod_state[i].isEmpty()) {
          int  n_timeout = 0;
          char s_app_name[LEN_APP_NAME + 1] = { 0 };
          char s_descrebe[LEN_DESCREBE + 1] = { 0 };

          const char *p_app = ::strchr(s_line, ';');
          if (p_app == NULL) {
            break;
          }
          const char *p_desc = ::strchr(p_app + 1, ';');
          if (p_desc == NULL) {
            break;
          }
          strncpy(s_app_name, s_line,
                  (((p_app - s_line) > LEN_APP_NAME) ?
                   LEN_APP_NAME : (p_app - s_line)));
          strncpy(s_descrebe, p_app + 1,
                  (((p_desc - p_app - 1) > LEN_DESCREBE) ?
                   LEN_DESCREBE : (p_desc - p_app - 1)));
          n_timeout = atoi(p_desc + 1);
          if (s_app_name[0] == '\0' ||
              s_descrebe[0] == '\0' ||
              n_timeout < 5 || n_timeout > MAX_WATCHDOG_TIMEOUT) {
            VZ_ERROR("get a error config %s.\n", s_line);
            continue;
          }

          // 去重复
          bool b_find = false;
          for (int j = 0; j < MAX_WATCHDOG_A_DEVICE; j++) {
            if (shm_mod_->mod_state[j].isSame(s_app_name, s_descrebe)) {  // 进程名
              b_find = true;
              break;
            }
          }

          // 加入新模块
          if (b_find == false) {
            shm_mod_->mod_state[i].Init(s_app_name, s_descrebe, n_timeout);
            VZ_ERROR("add module %s-%s-%d.\n",
                     shm_mod_->mod_state[i].app_name,
                     shm_mod_->mod_state[i].descrebe,
                     shm_mod_->mod_state[i].timeout);
          }
          break;
        }
      }
    } while (!feof(file) && n_line > 0);

    fclose(file);

    stride_watch_time_ = GetSysSec() + 1;   // 1S, 跳过检查心跳
    return 0;
  }
  return -1;
}

int CVzLogManage::InitSrvSocket(const char *ip, unsigned short port,
                                const char *trans_addr) {
  log_srv_.SetCallback(callback_receive, this,
                       callback_timeout, this);

  int ret = -1;
  // 打开网络监听
  ret = log_srv_.OpenListenAddr(ip, port);
  if (ret < 0) {
    VZ_ERROR("open udp listen %d failed. %d.\n", port, ret);
    return ret;
  }

  ret = log_srv_.OpenTransAddr(trans_addr);
  if (ret < 0) {
    VZ_ERROR("open udp trans %s failed. %d.\n", trans_addr, ret);
  }
  return 0;
}

static int k_total_log = 0;
int CVzLogManage::OnReceive(SOCKET sock, const sockaddr_in *raddr,
                            const char *smsg, unsigned int nmsg) {
  if (is_stdout_) {
    vzlogging::VzLogPrint(smsg, (int)nmsg);
    fflush(stdout);
  }
  k_total_log += nmsg;

  // 比GetSysSec大,跳过记录日志和心跳检查;
  if (stride_watch_time_ > 0) {
    if (stride_watch_time_ <= GetSysSec()) {
      stride_watch_time_ = 0;
    }
    return nmsg;
  }

  if (smsg[0] == L_C_HEARTBEAT) {
    WatchdogProcess(smsg, nmsg);            // 心跳,发送给看门狗处理
  } else if (smsg[0] == L_C_RESET) {
    InitMonitorModule(DEF_WDG_MODULE_FILE); // 重新初始化
  } else {
    log_file_.Write(smsg, nmsg);           // 普通日志
  }

  // 透传接收到的日志信息
  log_srv_.TransMsgToServer(smsg, nmsg);
  return nmsg;
}

int CVzLogManage::OnTimeout() {
  if (shm_mod_ == NULL) {
    return -1;
  }

  unsigned int n_now = GetSysSec();
  static unsigned int n_last = n_now;
  if ((n_now - n_last) > 1) {  // 1S检查一次
    // 进程超时检查
    for (int i = 0; i < MAX_WATCHDOG_A_DEVICE; i++) {
      if (!shm_mod_->mod_state[i].isEmpty() &&
          shm_mod_->mod_state[i].isTimeout(n_now)) {
        OnModuleLostHeartbeat(n_now);
        memset(&shm_mod_->mod_state[i], 0, sizeof(TAG_MODULE_STATE));
        break;
      }
    }

    n_last = n_now;
  }

  log_file_.Sync();
  wdg_file_.Sync();
  return 0;
}

int CVzLogManage::WatchdogProcess(const char* smsg, unsigned int nmsg) {
  if (shm_mod_ == NULL) {
    return -1;
  }

  const char* p_usr_msg = strrchr(smsg, ']');
  if (p_usr_msg == NULL) {
    VZ_PRINT("can't find ']' from heartbeat message.\n");
    return -1;
  }

  // 格式化获取字符串数据
  unsigned int n_timeout = 0;
  char s_app_name[LEN_APP_NAME] = { 0 };
  char s_descrebe[LEN_DESCREBE] = { 0 };
  sscanf(p_usr_msg + 2, "%s %d %s",
         s_app_name, &n_timeout, s_descrebe);

  int n_empty = -1;  // 未使用进程信息空间

  // 更新进程心跳时间
  for (int i = 0; i < MAX_WATCHDOG_A_DEVICE; i++) {
    if (shm_mod_->mod_state[i].isSame(s_app_name, s_descrebe)) {
      shm_mod_->mod_state[i].UpdateTimeout(n_timeout);
      shm_mod_->mod_state[i].Heartbeat();
      return 0;
    }

    if (n_empty == -1 && shm_mod_->mod_state[i].isEmpty()) {
      n_empty = i;
    }
  }

  //
  if (n_empty == -1) {
    VZ_ERROR("not empty space to save heartbeat infomation.\n");
    return -2;
  }

  // 新进程
  shm_mod_->mod_state[n_empty].Init(s_app_name, s_descrebe, n_timeout);
  VZ_ERROR("register module %s-%s-%d.\n",
           shm_mod_->mod_state[n_empty].app_name,
           shm_mod_->mod_state[n_empty].descrebe,
           shm_mod_->mod_state[n_empty].timeout);
  return 0;
}

int CVzLogManage::OnModuleLostHeartbeat(time_t n_now) {
  if (shm_mod_ == NULL) {
    return -1;
  }

  // 排序
  vzlogging::TAG_MODULE_STATE cmodule;
  for (int i = 0; i < MAX_WATCHDOG_A_DEVICE; i++) {
    for (int j = i + 1; j < MAX_WATCHDOG_A_DEVICE - 1; j++) {
      if (strncmp(shm_mod_->mod_state[i].app_name,
                  shm_mod_->mod_state[j].app_name, LEN_APP_NAME) < 0) {
        memcpy(&cmodule, &shm_mod_->mod_state[i], sizeof(cmodule));
        if (shm_mod_->mod_state[i].mark == DEF_TAG_MARK) {
          memcpy(&shm_mod_->mod_state[i], &shm_mod_->mod_state[j], sizeof(cmodule));
        }
        if (cmodule.mark == DEF_TAG_MARK) {
          memcpy(&shm_mod_->mod_state[j], &cmodule, sizeof(cmodule));
        }
      }
    }
  }

  // 格式化输出
  int  nlog = 0, n1log = 0;
  char slog[A_LOG_SIZE+1] = { 0 };
  n1log = nlog = snprintf(slog, A_LOG_SIZE, "watchdog danager ");
  for (int j = 0; j < MAX_WATCHDOG_A_DEVICE; j++) {
    if (false == shm_mod_->mod_state[j].isEmpty()) {
      char *p_fmt = (char*)"--%s-%s[%d]";
      if (n1log == nlog) {  // 只有开始数据,第一包不包含"--"
        p_fmt = (char*)"%s-%s[%d]";
      }
      nlog += snprintf(slog + nlog, A_LOG_SIZE - nlog,
                       p_fmt,
                       shm_mod_->mod_state[j].app_name,
                       shm_mod_->mod_state[j].descrebe,
                       (n_now - shm_mod_->mod_state[j].last_heartbeat));
    }
  }
  slog[nlog++] = '\n';
  VZ_ERROR("%s", slog);
  // wdg_file_.WriteSome("watchdog stop");

  // 使能看门狗
  printf("%s[%d] %d\n", __FUNCTION__, __LINE__, is_reboot_);
  if (is_reboot_) {
#ifndef _WIN32
    if (fork() == 0) {
      printf("son process reboot.\n");
      system("sleep 10; reboot; sleep 1; reboot");
    }
#endif
    try {
      wdg_file_.Write(slog, nlog);
      wdg_file_.Sync();

      log_file_.OnModuleLostHeartbeat(slog, nlog);    // 日志文件转存
    } catch (...) {
    }

    // 重启设备
    system("reboot;sleep 1;reboot");
    exit(127);
  }
  return 0;
}
}  // namespace vzlogging

