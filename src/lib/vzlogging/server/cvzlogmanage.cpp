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

  /* ����־��¼ */
  res = InitLogRecord(log_path, DEF_LOG_REC_FILE, DEF_WDG_REC_FILE);
  if (res < 0) {
    VZ_ERROR("Open record failed. path %s, log file %s, watchdog file %s.\n",
             log_path, DEF_LOG_REC_FILE, DEF_WDG_REC_FILE);
  }

  /* ��ʼ�����Ź����ģ�� */
  res = InitMonitorModule(DEF_WDG_MODULE_FILE);
  if (res < 0) {
    VZ_ERROR("Open monitor %s failed.%d.\n", DEF_WDG_MODULE_FILE);
  }

  /* ��������� */
  res = InitSrvSocket("0.0.0.0", log_port, tran_addr);
  if (res < 0) {
    VZ_ERROR("Open log port %d failed.%d.\n", log_port, res);
    return res;
  }
  return 0;
}

void CVzLogManage::RunLoop(int ms) {
  k_log_srv.Loop(ms);
}

void CVzLogManage::SetLogAddrAndLevel(const char *lhost, unsigned short port,
                                      unsigned int level) {
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
  k_shm_arg.SetShmArg(&k_shm_mod);
}

int CVzLogManage::InitLogRecord(const char* path,
                                const char* log_fname, const char* wdg_fname) {
  int ret = -1;

  // ��ͨ��־
  ret = k_log_file.Open(path, log_fname, MAX_LOG_FILE_SIZE);
  if (ret < 0) {
    VZ_ERROR("log record open failed.%d.\n", ret);
    return ret;
  }
  k_log_file.WriteSome("program log");

  // ���Ź���־
  ret = k_wdg_file.Open(path, wdg_fname, MAX_WDG_FILE_SIZE);
  if (ret < 0) {
    VZ_ERROR("wdg record open failed.%d.\n", ret);
    return ret;
  }
  k_wdg_file.WriteSome("watchdog start");

  return 0;
}

int CVzLogManage::InitMonitorModule(const char* s_path) {
  memset(k_shm_mod.mod_state, 0, sizeof(k_shm_mod.mod_state));

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

      for (int i = 0; i < MAX_WATCHDOG_A_DEVICE; i++) {
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

          // ȥ�ظ�
          bool b_find = false;
          for (int j = 0; j < MAX_WATCHDOG_A_DEVICE; j++) {
            if (k_shm_mod.mod_state[j].mark == DEF_TAG_MARK &&
                !strncmp(k_shm_mod.mod_state[j].descrebe, s_descrebe, LEN_DESCREBE)       // ������
                && !strncmp(k_shm_mod.mod_state[j].app_name, s_app_name, LEN_APP_NAME)) {  // ������
              b_find = true;
              break;
            }
          }

          // ������ģ��
          if (b_find == false) {
            k_shm_mod.mod_state[i].mark = DEF_TAG_MARK;
            k_shm_mod.mod_state[i].timeout = n_timeout;
            k_shm_mod.mod_state[i].join_time = GetSysSec();
            k_shm_mod.mod_state[i].last_heartbeat = GetSysSec();
            strncpy(k_shm_mod.mod_state[i].app_name, s_app_name, LEN_APP_NAME);
            strncpy(k_shm_mod.mod_state[i].descrebe, s_descrebe, LEN_DESCREBE);
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
#ifndef _WIN32
    sleep(1);
#endif
    return 0;
  }
  return -1;
}

int CVzLogManage::InitSrvSocket(const char *ip, unsigned short port,
                                const char *trans_addr) {
  k_log_srv.SetCallback(callback_receive, this,
                        callback_timeout, this);

  int ret = -1;
  // ���������
  ret = k_log_srv.OpenListenAddr(ip, port);
  if (ret < 0) {
    VZ_ERROR("open udp listen %d failed. %d.\n", port, ret);
    return ret;
  }

  ret = k_log_srv.OpenTransAddr(trans_addr);
  if (ret < 0) {
    VZ_ERROR("open udp trans %s failed. %d.\n", trans_addr, ret);
  }
  return 0;
}

static int k_total_log = 0;
int CVzLogManage::OnReceive(SOCKET sock, const sockaddr_in *raddr,
                            const char *smsg, unsigned int nmsg) {
  if (k_en_stdout) {
    vzlogging::VzLogPrint(smsg, (int)nmsg);
    fflush(stdout);
  }

  if (smsg[0] == L_C_HEARTBEAT) {    
    WatchdogProcess(smsg, nmsg);            // ����,���͸����Ź�����
  } else if (smsg[0] == L_C_RESET) {
    InitMonitorModule(DEF_WDG_MODULE_FILE); // ���³�ʼ��
  } else {
    k_log_file.Write(smsg, nmsg);           // ��ͨ��־
  }

  // ͸�����յ�����־��Ϣ
  k_log_srv.TransMsgToServer(smsg, nmsg);

  k_total_log += nmsg;
  return nmsg;
}

int CVzLogManage::OnTimeout() {
  unsigned int n_now = GetSysSec();
  static unsigned int n_last = n_now;
  if ((n_now - n_last) > 1) {  // 1S���һ��
    // ���̳�ʱ���
    for (int i = 0; i < MAX_WATCHDOG_A_DEVICE; i++) {
      if (k_shm_mod.mod_state[i].mark == DEF_TAG_MARK
          && k_shm_mod.mod_state[i].app_name[0] != '\0') {

        unsigned long n_diff_time =
          n_now - k_shm_mod.mod_state[i].last_heartbeat;
        if (n_diff_time > k_shm_mod.mod_state[i].timeout) {
          OnModuleLostHeartbeat(n_now);
          memset(&k_shm_mod.mod_state[i], 0, sizeof(TAG_MODULE_STATE));
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

  // ��ʽ����ȡ�ַ�������
  unsigned int n_timeout = 0;
  char s_app_name[LEN_APP_NAME] = { 0 };
  char s_descrebe[LEN_DESCREBE] = { 0 };
  sscanf(p_usr_msg + 2, "%s %d %s",
         s_app_name, &n_timeout, s_descrebe);

  int n_empty = -1;  // δʹ�ý�����Ϣ�ռ�

  // ���½�������ʱ��
  for (int i = 0; i < MAX_WATCHDOG_A_DEVICE; i++) {
    if (k_shm_mod.mod_state[i].mark == DEF_TAG_MARK &&
        !strncmp(k_shm_mod.mod_state[i].descrebe, s_descrebe, LEN_DESCREBE)       // ������
        && !strncmp(k_shm_mod.mod_state[i].app_name, s_app_name, LEN_APP_NAME)) {  // ������
      if (5 <= n_timeout && n_timeout < MAX_WATCHDOG_TIMEOUT) {
        k_shm_mod.mod_state[i].timeout = n_timeout;
      }
      k_shm_mod.mod_state[i].last_heartbeat = GetSysSec();

      ShareBuffer();
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

  // �½���
  k_shm_mod.mod_state[n_empty].mark = DEF_TAG_MARK;
  strncpy(k_shm_mod.mod_state[n_empty].app_name, s_app_name, LEN_APP_NAME);
  strncpy(k_shm_mod.mod_state[n_empty].descrebe, s_descrebe, LEN_DESCREBE);
  k_shm_mod.mod_state[n_empty].timeout = n_timeout;
  k_shm_mod.mod_state[n_empty].join_time = GetSysSec();
  k_shm_mod.mod_state[n_empty].last_heartbeat = GetSysSec();
  VZ_PRINT("%s %s register.\n", s_app_name, s_descrebe);
  ShareBuffer();
  return 0;
}

int CVzLogManage::OnModuleLostHeartbeat(time_t n_now) {
  // ����
  vzlogging::TAG_MODULE_STATE cmodule;
  for (int i = 0; i < MAX_WATCHDOG_A_DEVICE; i++) {
    for (int j = i + 1; j < MAX_WATCHDOG_A_DEVICE - 1; j++) {
      if (strncmp(k_shm_mod.mod_state[i].app_name,
                  k_shm_mod.mod_state[j].app_name, LEN_APP_NAME) < 0) {
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

  // ��ʽ�����
  int  nlog = 0, n1log = 0;
  char slog[A_LOG_SIZE] = { 0 };
  n1log = nlog = snprintf(slog, A_LOG_SIZE, "watchdog danager ");
  for (int j = 0; j < MAX_WATCHDOG_A_DEVICE; j++) {
    if (k_shm_mod.mod_state[j].mark == DEF_TAG_MARK
        && k_shm_mod.mod_state[j].app_name[0] != '\0') {
      char *p_fmt = (char*)"--%s-%s[%d]";
      if (n1log == nlog) {  // ֻ�п�ʼ����,��һ��������"--"
        p_fmt = (char*)"%s-%s[%d]";
      }
      nlog += snprintf(slog + nlog, A_LOG_SIZE - nlog,
                       p_fmt,
                       k_shm_mod.mod_state[j].app_name,
                       k_shm_mod.mod_state[j].descrebe,
                       (n_now - k_shm_mod.mod_state[j].last_heartbeat));
    }
  }
  if (nlog < A_LOG_SIZE) {
    slog[nlog++] = '\n';
  }
  VZ_ERROR("%s", slog);
  k_wdg_file.Write(slog, nlog);
  // k_wdg_file.WriteSome("watchdog stop");

  // ʹ�ܿ��Ź�
  if (k_en_watchdog) {
    // ��־�ļ�ת��
    k_log_file.OnModuleLostHeartbeat(slog, nlog);
    // �����豸
    system("reboot");
    exit(127);
  }
  return 0;
}
}  // namespace vzlogging

