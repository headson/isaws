/************************************************************************
*Author      : Sober.Peng 17-08-22
*Description :
************************************************************************/
#include "cvzlogmanage.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

namespace vzlogging {

CVzLogManage::CVzLogManage() {
  k_en_stdout = 0;
  k_en_watchdog = 1;

  memset(&k_shm_mod, 0, sizeof(k_shm_mod));
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

  res = k_shm_arg.Open();
  if (res != 0) {
    VZ_ERROR("share memory open failed.\n");
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
    VZ_ERROR("Open monitor module config file failed.%d.\n", res);
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
  k_srv_sock.Loop(ms);
}

void CVzLogManage::SetLogAddrAndLevel(const char *lhost, unsigned short port, unsigned int level) {
  k_shm_mod.snd_level = level;

  k_shm_mod.sock_addr.sin_family = AF_INET;
  k_shm_mod.sock_addr.sin_port = htons(port);
  k_shm_mod.sock_addr.sin_addr.s_addr = inet_addr(lhost);

  ShareBuffer();
}

void CVzLogManage::SetEnableStdout() {
  k_en_stdout = 1;
}

void CVzLogManage::SetDisableWatchdog() {
  k_en_watchdog = 0;
}

void CVzLogManage::ShareBuffer() {
  k_shm_arg.Share(&k_shm_mod, sizeof(k_shm_mod));
  k_shm_arg.Share(&k_shm_mod, sizeof(k_shm_mod));
  k_shm_arg.Share(&k_shm_mod, sizeof(k_shm_mod));
}

int CVzLogManage::InitLogRecord(const char* path, const char* log_fname, const char* wdg_fname) {
  int ret = -1;

  // 普通日志
  ret = k_log_file.Open(path, log_fname, DEF_LOG_FILE_SIZE);
  if (ret < 0) {
    VZ_ERROR("log record open failed.%d.\n", ret);
    return ret;
  }
  k_log_file.WriteSome("program log");

  // 看门狗日志
  ret = k_wdg_file.Open(path, wdg_fname, DEF_WDG_FILE_SIZE);
  if (ret < 0) {
    VZ_ERROR("wdg record open failed.%d.\n", ret);
    return ret;
  }
  k_wdg_file.WriteSome("watchdog start");

  return 0;
}

int CVzLogManage::InitMonitorModule(const char* s_path) {
  FILE* file = fopen(s_path, "rt");
  if (file) {
    int  n_line = 0;
    char s_line[128 + 1];
    do {
      memset(s_line, 0, 128);
      fgets(s_line, 128, file);
      if ((n_line = strlen(s_line)) <= 0) {
        break;
      }
      if (s_line[0] == '#') {
        continue;
      }

      for (int i = 0; i < DEF_PER_DEV_PROCESS_MAX; i++) {
        if (k_shm_mod.mod_state[i].mark == 0
            && k_shm_mod.mod_state[i].app_name[0] == '\0') {
          char s_app_name[32 + 1] = { 0 };
          char s_descrebe[8 + 1] = { 0 };
          int  n_timeout = 0;

          const char *p_app = ::strchr(s_line, ';');
          if (p_app == NULL) {
            break;
          }
          const char *p_desc = ::strchr(p_app + 1, ';');
          if (p_desc == NULL) {
            break;
          }
          strncpy(s_app_name, s_line,
                  (((p_app - s_line) > DEF_PROCESS_NAME_MAX) ?
                   DEF_PROCESS_NAME_MAX : (p_app - s_line)));
          strncpy(s_descrebe, p_app + 1,
                  (((p_desc - p_app - 1) > DEF_USER_DESCREBE_MAX) ?
                   DEF_USER_DESCREBE_MAX : (p_desc - p_app - 1)));
          n_timeout = atoi(p_desc + 1);
          if (s_app_name[0] == '\0' ||
              s_descrebe[0] == '\0' ||
              n_timeout < 5 || n_timeout > 180) {
            VZ_ERROR("get a error config %s.\n", s_line);
            continue;
          }

          // 去重复
          bool b_find = false;
          for (int j = 0; j < DEF_PER_DEV_PROCESS_MAX; j++) {
            if (k_shm_mod.mod_state[j].mark == DEF_TAG_MARK &&
                !strncmp(k_shm_mod.mod_state[j].descrebe, s_descrebe, DEF_USER_DESCREBE_MAX)       // 描述符
                && !strncmp(k_shm_mod.mod_state[j].app_name, s_app_name, DEF_PROCESS_NAME_MAX)) {  // 进程名
              b_find = true;
              break;
            }
          }

          // 加入新模块
          if (b_find == false) {
            k_shm_mod.mod_state[i].mark = DEF_TAG_MARK;
            k_shm_mod.mod_state[i].timeout = n_timeout;
            k_shm_mod.mod_state[i].last_heartbeat = get_sys_sec();
            strncpy(k_shm_mod.mod_state[i].app_name, s_app_name, DEF_PROCESS_NAME_MAX);
            strncpy(k_shm_mod.mod_state[i].descrebe, s_descrebe, DEF_USER_DESCREBE_MAX);
            VZ_ERROR("add module %s-%s-%d.\n",
                     k_shm_mod.mod_state[i].app_name,
                     k_shm_mod.mod_state[i].descrebe,
                     k_shm_mod.mod_state[i].timeout);
          }
          break;
        }
      }
    } while (!feof(file) && n_line > 0);

    fclose(file);

    ShareBuffer();
    return 0;
  }
  return -1;
}

int CVzLogManage::InitSrvSocket(const char *ip, unsigned short port,
                                const char *s_snd_addr) {
  k_srv_sock.SetCallback(callback_receive, this,
                         callback_timeout, this);

  int ret = -1;
  // 打开网络监听
  ret = k_srv_sock.OpenListenAddr(ip, port);
  if (ret < 0) {
    VZ_ERROR("open udp listen %d failed. %d.\n", port, ret);
    return ret;
  }

  ret = k_srv_sock.OpenTransAddr(s_snd_addr);
  if (ret < 0) {
    VZ_ERROR("open udp trans %s failed. %d.\n", s_snd_addr, ret);
  }
  return 0;
}

static int k_total_log = 0;
int CVzLogManage::OnReceive(SOCKET sock, const sockaddr_in *raddr,
                            const char *smsg, unsigned int nmsg) {
  if (k_en_stdout) {
    if (nmsg < DEF_LOG_MAX_SIZE) {
      ((char*)smsg)[nmsg] = '\0';
    }
    // printf(s_msg);
    vzlogging::VzDumpLogging(smsg, (int)nmsg);
    fflush(stdout);
  }

  if (smsg[0] != L_C_HEARTBEAT) {
    // 普通日志
    k_log_file.Write(smsg, nmsg);
  } else {
    // 心跳,发送给看门狗处理
    WatchdogProcess(smsg, nmsg);
  }

  // 透传接收到的日志信息
  k_srv_sock.TransMsgToServer(smsg, nmsg);

  k_total_log += nmsg;
  return nmsg;
}

int CVzLogManage::OnTimeout() {
  unsigned int n_now = get_sys_sec();
  static unsigned int n_last = n_now;
  if ((n_now - n_last) > 1) {  // 1S检查一次
    // 进程超时检查
    for (int i = 0; i < DEF_PER_DEV_PROCESS_MAX; i++) {
      if (k_shm_mod.mod_state[i].mark == DEF_TAG_MARK
          && k_shm_mod.mod_state[i].app_name[0] != '\0') {
        unsigned long n_diff_time =
          n_now - k_shm_mod.mod_state[i].last_heartbeat;
        if (n_diff_time > k_shm_mod.mod_state[i].timeout) {
          //if (k_en_watchdog) {
          OnModuleLostHeartbeat(n_now);
          //}

          memset(&k_shm_mod.mod_state[i], 0, sizeof(k_shm_mod.mod_state[i]));
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

int CVzLogManage::WatchdogProcess(const char* smsg, unsigned int nmsg) {
  const char* p_usr_msg = strrchr(smsg, ']');
  if (p_usr_msg == NULL) {
    VZ_PRINT("can't find ']' from heartbeat message.\n");
    return -1;
  }

  // 格式化获取字符串数据
  unsigned int n_timeout = 0;
  char s_app_name[DEF_PROCESS_NAME_MAX] = { 0 };
  char s_descrebe[DEF_USER_DESCREBE_MAX] = { 0 };
  sscanf(p_usr_msg + 2, "%s %d %s",
         s_app_name, &n_timeout, s_descrebe);

  int n_empty = -1;  // 未使用进程信息空间

  // 更新进程心跳时间
  for (int i = 0; i < DEF_PER_DEV_PROCESS_MAX; i++) {
    if (k_shm_mod.mod_state[i].mark == DEF_TAG_MARK &&
        !strncmp(k_shm_mod.mod_state[i].descrebe, s_descrebe, DEF_USER_DESCREBE_MAX)       // 描述符
        && !strncmp(k_shm_mod.mod_state[i].app_name, s_app_name, DEF_PROCESS_NAME_MAX)) {  // 进程名
      if (5 <= n_timeout && n_timeout < 180) {
        k_shm_mod.mod_state[i].timeout = n_timeout;
      }
      unsigned int nlast = k_shm_mod.mod_state[i].last_heartbeat;
      k_shm_mod.mod_state[i].last_heartbeat = get_sys_sec();

      if (0 == nlast) {
        ShareBuffer();
      }
      return 0;
    }

    if (n_empty == -1 &&
        k_shm_mod.mod_state[i].mark == 0) {
      n_empty = i;
    }
  }

  //
  if (n_empty == -1) {
    VZ_ERROR("not empty space to save heartbeat infomation.\n");
    return -2;
  }

  // 新进程
  k_shm_mod.mod_state[n_empty].mark = DEF_TAG_MARK;
  strncpy(k_shm_mod.mod_state[n_empty].app_name, s_app_name, DEF_PROCESS_NAME_MAX);
  strncpy(k_shm_mod.mod_state[n_empty].descrebe, s_descrebe, DEF_USER_DESCREBE_MAX);
  k_shm_mod.mod_state[n_empty].timeout = n_timeout;
  k_shm_mod.mod_state[n_empty].last_heartbeat = get_sys_sec();
  VZ_PRINT("%s %s register.\n", s_app_name, s_descrebe);
  ShareBuffer();
  return 0;
}

int CVzLogManage::OnModuleLostHeartbeat(time_t n_now) {
  // 排序
  vzlogging::TAG_MODULE_STATE cmodule;
  for (int i = 0; i < DEF_PER_DEV_PROCESS_MAX; i++) {
    for (int j = i + 1; j < DEF_PER_DEV_PROCESS_MAX - 1; j++) {
      if (strncmp(k_shm_mod.mod_state[i].app_name,
                  k_shm_mod.mod_state[j].app_name, DEF_PROCESS_NAME_MAX) < 0) {
        memcpy(&cmodule, &k_shm_mod.mod_state[i], sizeof(cmodule));
        if (k_shm_mod.mod_state[i].mark == DEF_TAG_MARK) {
          memcpy(&k_shm_mod.mod_state[i], &k_shm_mod.mod_state[j], sizeof(cmodule));
        }
        if (cmodule.mark == DEF_TAG_MARK) {
          memcpy(&k_shm_mod.mod_state[j], &cmodule, sizeof(cmodule));
        }
      }
    }
  }

  // 格式化输出
  int  nlog = 0, n1log = 0;
  char slog[DEF_LOG_MAX_SIZE] = { 0 };
  n1log = nlog = snprintf(slog, DEF_LOG_MAX_SIZE, "watchdog danager ");
  for (int j = 0; j < DEF_PER_DEV_PROCESS_MAX; j++) {
    if (k_shm_mod.mod_state[j].mark == DEF_TAG_MARK
        && k_shm_mod.mod_state[j].app_name[0] != '\0') {
      char *p_fmt = (char*)"--%s-%s[%d]";
      if (n1log == nlog) {  // 只有开始数据,第一包不包含"--"
        p_fmt = (char*)"%s-%s[%d]";
      }
      nlog += snprintf(slog + nlog, DEF_LOG_MAX_SIZE - nlog,
                       p_fmt,
                       k_shm_mod.mod_state[j].app_name,
                       k_shm_mod.mod_state[j].descrebe,
                       (n_now - k_shm_mod.mod_state[j].last_heartbeat));
    }
  }
  if (nlog < DEF_LOG_MAX_SIZE) {
    slog[nlog++] = '\n';
  }
  VZ_ERROR("%s", slog);
  k_wdg_file.Write(slog, nlog);
  k_wdg_file.WriteSome("watchdog stop");

  // 使能看门狗
  if (k_en_watchdog) {
    // 日志文件转存
    k_log_file.OnModuleLostHeartbeat(slog, nlog);
    // 重启设备
    system("reboot");
  }
  return 0;
}


}  // namespace vzlogging
