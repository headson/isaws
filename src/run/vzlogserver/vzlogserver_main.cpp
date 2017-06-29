/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : ��־\���Ź����������
************************************************************************/
#include <time.h>   // ��ʱ
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vzlogging/server/vzflags.h"

#include "vzlogging/base/vzlogdef.h"
#include "vzlogging/base/vzshmarg.h"
#include "vzlogging/logging/vzlogging.h"
#include "vzlogging/server/cvzlogserver.h"

static vzlogging::CVzShmArg       k_shm_arg;      // �������
static vzlogging::CVzLoggingFile  k_log_file;     // ��ͨ��־
static vzlogging::CVzWatchdogFile k_wdg_file;     // ���Ź���־
static vzlogging::CVzSockDgram    k_srv_sock;     // �������
static bool                       k_en_stdout = false;    // ʹ�����
static bool                       k_en_watchdog = true;   // ���Ź�ʹ��

/***����������Ϣ*********************************************************/
typedef struct _TAG_WATCHDOG {
  unsigned int  n_mark;
  char          s_app_name[DEF_PROCESS_NAME_MAX];     // ������
  char          s_descrebe[DEF_USER_DESCREBE_MAX+4];  // �û�����
  unsigned int  n_timeout;                            // ��ʱ
  time_t        last_heartbeat;                       // ���һ������ʱ��
} TAG_HEARTBEAT;
static TAG_HEARTBEAT k_pro_heart[DEF_PER_DEV_PROCESS_MAX];  // ����������Ϣ

/***static function******************************************************/
/*�������*/
int InitShareParam(const char*    s_ip,
                   unsigned short n_port,
                   unsigned int   n_snd_level);

/*��ʼ����־��¼*/
int InitLogRecord(const char* s_path,
                  const char* s_log_fname,
                  const char* s_wdg_fname);

/*��ʼ�����Ź����ģ��*/
int InitMonitorModule(const char* s_path);

/*��ʼ���������*/
int InitSrvSocket(const char*     s_ip,
                  unsigned short  n_port,
                  const char*     s_snd_addr);

/*��ʱ�ص�*/
int callback_timeout();

/*ι���ص�*/
int callback_feeddog();

/*���ջص�*/
int callback_receive(SOCKET             sock,
                     const sockaddr_in* addr,
                     const char*        msg,
                     unsigned int       size);

/*���Ź�����*/
int WatchdogProcess(const char* msg, unsigned int size);

/*��ģ�鶪ʧ����ʱ����*/
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
  system("killall watchdog");

  VZ_ERROR("applet compile time: %s %s\n", __TIME__, __DATE__);
  memset(k_pro_heart, 0, sizeof(k_pro_heart));

  int ret = 0;
  // ���ε��ͻ���,��־�ϴ��ȼ�,�ϴ��˿ں�
  unsigned int  n_snd_level = 3;
  unsigned int  n_recv_port = DEF_SERVER_PORT;
  // ת����������ַ;��ʽ[IP:PORT]
  char          s_snd_addr[32] = { 0 };
  char          s_log_path[64] = { 0 };
  strncpy(s_log_path, DEF_RECORD_PATH, 64);  // Ĭ��·��

  int opt = 0;
  while ((opt = getopt(argc, argv, "v:V:p:P:s:S:dDhHfF")) != -1) {
    switch (opt) {
    case 'v':   // ���������������
    case 'V':
      n_snd_level = atoi(optarg);
      break;

    case 'f':   // ���Ź��ر�
    case 'F':
      k_en_watchdog = false;
      break;

    case 'p':   // �������紫��˿�
    case 'P':
      n_recv_port = atoi(optarg);
      break;

    case 's':   // ��־�洢·��
    case 'S': {
      memset(s_log_path, 0, 64);
      strncpy(s_log_path, optarg, 64);
    }
    break;

    case 'd':   // ���ñ��ش�ӡ
    case 'D':
      k_en_stdout = true;
      break;

    case 'h':   // ����
    case 'H':
      PrintUsage();
      break;
    }
  }
  if (k_en_watchdog == false) {
    VZ_ERROR("close watchdog.\n");
  }

  // �������
  k_shm_arg.Open();
  ret = InitShareParam(DEF_SERVER_HOST, n_recv_port, n_snd_level);

  // ����־�洢
  ret = InitLogRecord(s_log_path,
                      DEF_LOG_REC_FILE,
                      DEF_WDG_REC_FILE);
  if (ret < 0) {
    VZ_ERROR("Open record failed. path %s, log file %s, watchdog file %s.\n",
             s_log_path, DEF_LOG_REC_FILE, DEF_WDG_REC_FILE);
    return ret;
  }
  VZ_PRINT("send log level %d.\n", n_snd_level);

  /*��ʼ�����Ź����ģ��*/
  ret = InitMonitorModule(DEF_WDG_MODULE_FILE);
  if (ret < 0) {
    VZ_ERROR("Open monitor module config file failed.%d.\n", ret);
    //return ret;
  }

  // ���������
  ret = InitSrvSocket("0.0.0.0", n_recv_port, s_snd_addr);
  if (ret < 0) {
    return ret;
  }

  while (true) {
    k_srv_sock.Loop();  // �ȴ��������
  }

#ifdef WIN32
  WSACleanup();
#endif
  return 0;
}

/***static function******************************************************/
unsigned int get_sys_sec() {
#ifdef WIN32
  return (unsigned int)(GetTickCount()/1000);
#else
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (unsigned int )ts.tv_sec;
#endif
  return 0;
}

/*��ʼ�������ڴ�*/
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

/*��ʼ����־��¼*/
int InitLogRecord(const char* path,
                  const char* log_fname,
                  const char* wdg_fname) {
  int ret = -1;

  // ��ͨ��־
  ret = k_log_file.Open(path, log_fname, DEF_LOG_FILE_SIZE);
  if (ret < 0) {
    VZ_ERROR("log record open failed.%d.\n", ret);
    return ret;
  }
  k_log_file.StartRecord("program log");

  // ���Ź���־
  ret = k_wdg_file.Open(path, wdg_fname, DEF_WDG_FILE_SIZE);
  if (ret < 0) {
    VZ_ERROR("wdg record open failed.%d.\n", ret);
    return ret;
  }
  k_wdg_file.StartRecord("watchdog");

  return 0;
}

/*��ʼ�����Ź����ģ��*/
int InitMonitorModule(const char* s_path) {
  FILE* file = fopen(s_path, "rt");
  if (file) {
    int  n_line = 0;
    char s_line[128+1];
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
        if (k_pro_heart[i].n_mark == 0
            && k_pro_heart[i].s_app_name[0] == '\0') {
          char s_app_name[32+1] = {0};
          char s_descrebe[8+1]  = {0};
          int  n_timeout        = 0;

          const char *p_app = ::strchr(s_line, ';');
          if (p_app == NULL) {
            break;
          }
          const char *p_desc = ::strchr(p_app+1, ';');
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
              s_descrebe[0] =='\0' ||
              n_timeout < 5 || n_timeout > 80) {
            VZ_ERROR("get a error config %s.\n", s_line);
            break;
          }

          // ȥ�ظ�
          bool b_find = false;
          for (int j = 0; j < DEF_PER_DEV_PROCESS_MAX; j++) {
            if (k_pro_heart[j].n_mark == DEF_TAG_MARK &&
                !strncmp(k_pro_heart[j].s_descrebe, s_descrebe, DEF_USER_DESCREBE_MAX)       // ������
                && !strncmp(k_pro_heart[j].s_app_name, s_app_name, DEF_PROCESS_NAME_MAX)) {  // ������
              b_find = true;
              break;
            }
          }

          // ������ģ��
          if (b_find == false) {
            k_pro_heart[i].n_mark = DEF_TAG_MARK;
            k_pro_heart[i].n_timeout = n_timeout;
            k_pro_heart[i].last_heartbeat = get_sys_sec();
            strncpy(k_pro_heart[i].s_app_name, s_app_name, DEF_PROCESS_NAME_MAX);
            strncpy(k_pro_heart[i].s_descrebe, s_descrebe, DEF_USER_DESCREBE_MAX);
            VZ_ERROR("add module %s-%s-%d.\n",
                     k_pro_heart[i].s_app_name,
                     k_pro_heart[i].s_descrebe,
                     k_pro_heart[i].n_timeout);
          }
          break;
        }
      }
    } while (!feof(file) && n_line > 0);

    fclose(file);
    return 0;
  }
  return -1;
}

/*��ʼ���������*/
int InitSrvSocket(const char* ip, unsigned short port,
                  const char* s_snd_addr) {
  k_srv_sock.SetCallback(callback_receive,
                         callback_timeout,
                         callback_feeddog);

  int ret = -1;
  // ���������
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
/*���ջص�*/
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
    // ��ͨ��־
    k_log_file.Write(s_msg, n_msg);
  } else {
    // ����,���͸����Ź�����
    WatchdogProcess(s_msg, n_msg);
  }

  // ͸�����յ�����־��Ϣ
  k_srv_sock.TransMsgToServer(s_msg, n_msg);

  k_total_log += n_msg;
  return n_msg;
}

/*��ʱ�ص�*/
int callback_timeout() {
  time_t n_now = get_sys_sec();
  static time_t n_last = n_now;
  if ((n_now - n_last) > 1) {  // 1S���һ��
    // ���̳�ʱ���
    for (int i = 0; i < DEF_PER_DEV_PROCESS_MAX; i++) {
      if (k_pro_heart[i].n_mark == DEF_TAG_MARK
          && k_pro_heart[i].s_app_name[0] != '\0') {
        unsigned long long n_diff_time =
          abs(n_now - k_pro_heart[i].last_heartbeat);
        if (n_diff_time > k_pro_heart[i].n_timeout) {
          //if (k_en_watchdog) {
          OnModuleLostHeartbeat(n_now);
          //}

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

/*ι���ص�*/
int callback_feeddog() {
  return 0;
}

/*���Ź�����*/
int WatchdogProcess(const char* s_msg, unsigned int n_msg) {
  const char* p_usr_msg = strrchr(s_msg, ']');
  if (p_usr_msg == NULL) {
    VZ_PRINT("can't find ']' from heartbeat message.\n");
    return -1;
  }

  // ��ʽ����ȡ�ַ�������
  unsigned int n_timeout = 0;
  char s_app_name[DEF_PROCESS_NAME_MAX]  = {0};
  char s_descrebe[DEF_USER_DESCREBE_MAX] = {0};
  sscanf(p_usr_msg+2, "%s %d %s",
         s_app_name, &n_timeout, s_descrebe);

  int n_empty = -1;  // δʹ�ý�����Ϣ�ռ�

  // ���½�������ʱ��
  for (int i = 0; i < DEF_PER_DEV_PROCESS_MAX; i++) {
    if (k_pro_heart[i].n_mark == DEF_TAG_MARK &&
        !strncmp(k_pro_heart[i].s_descrebe,
                 s_descrebe, DEF_USER_DESCREBE_MAX)       // ������
        && !strncmp(k_pro_heart[i].s_app_name,
                    s_app_name, DEF_PROCESS_NAME_MAX)) {  // ������
      if (5 <= n_timeout && n_timeout <= 80) {
        k_pro_heart[i].n_timeout = n_timeout;
      }
      k_pro_heart[i].last_heartbeat = get_sys_sec();
      return 0;
    }

    if (n_empty == -1 &&
        k_pro_heart[i].n_mark == 0) {
      n_empty = i;
    }
  }

  //
  if (n_empty == -1) {
    VZ_ERROR("not empty space to save heartbeat infomation.\n");
    return -2;
  }

  // �½���
  k_pro_heart[n_empty].n_mark         = DEF_TAG_MARK;
  strncpy(k_pro_heart[n_empty].s_app_name, s_app_name, DEF_PROCESS_NAME_MAX);
  strncpy(k_pro_heart[n_empty].s_descrebe, s_descrebe, DEF_USER_DESCREBE_MAX);
  k_pro_heart[n_empty].n_timeout      = n_timeout;
  k_pro_heart[n_empty].last_heartbeat = get_sys_sec();
  VZ_PRINT("%s %s register.\n", s_app_name, s_descrebe);
  return 0;
}

int OnModuleLostHeartbeat(time_t n_now) {
  // ����
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

  // ��ʽ�����
  int  n_log = 0, n_1_log = 0;
  char s_log[DEF_LOG_MAX_SIZE] = { 0 };
  n_1_log = n_log = snprintf(s_log,
                             DEF_LOG_MAX_SIZE,
                             "watchdog danager ");
  for (int j = 0; j < DEF_PER_DEV_PROCESS_MAX; j++) {
    if (k_pro_heart[j].n_mark == DEF_TAG_MARK
        && k_pro_heart[j].s_app_name[0] != '\0') {
      char *p_fmt = (char*)"--%s-%s[%d]";
      if (n_1_log == n_log) {  // ֻ�п�ʼ����,��һ��������"--"
        p_fmt = (char*)"%s-%s[%d]";
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
  VZ_ERROR("%s", s_log);
  k_wdg_file.Write(s_log, n_log);
  k_wdg_file.StartRecord("watchdog");

  // ʹ�ܿ��Ź�
  if (k_en_watchdog) {
    // ��־�ļ�ת��
    k_log_file.OnModuleLostHeartbeat(s_log, n_log);
    // �����豸
    system("reboot");
  }
  return 0;
}

