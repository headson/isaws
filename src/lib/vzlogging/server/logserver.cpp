/************************************************************************/
/* Copyright@ 2008 vzenith.com
/* All rights reserved.
/* ----------------------------------------------------------------------
/* Author      : Sober.Peng
/* Date        : 19:5:2017
/* Description : ��־\���Ź����������
/************************************************************************/
#include <time.h>   // ��ʱ
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base/vzlogdef.h"
#include "base/vzshmarg.h"
#include "server/cvzlogserver.h"
#include "server/cvzlogdatapool.h"

static vzlogging::CVzShmArg    k_shm_arg;           // �������
static vzlogging::CVzLogRecord k_log_rec;           // ��ͨ��־
static vzlogging::CVzLogRecord k_wdg_rec;           // ���Ź���־

static vzlogging::CVzSockDgram k_srv_sock;          // �������

static bool                    k_en_stdout = false;  // ʹ�����

static struct {
  char          app_name[32];                       //
  unsigned int  pid;                                // ����ID
  unsigned int  ppid;                               // �����߳�ID
  unsigned int  no_heart_times;                     // ����������
  time_t        last_heartbeat;                     // ���һ������ʱ��
  // ע��ʱ�Ѵ��������ַ����洢,����������ʱ��ӡ���ļ���,�������
  char          msg[80];
} k_pro_heart[DEF_PER_DEV_PROCESS_MAX];             // ����������Ϣ

/***static function******************************************************/
// �������
int InitShareParam(const char*    ip,
                   unsigned short port,
                   unsigned int   snd_level);

// ��ʼ����־��¼
int InitLogRecord(const char* path,
                  const char* log_fname,
                  const char* wdg_fname);

// ��ʼ���������
int InitSrvSocket(const char* ip,
                  unsigned short port,
                  const char* s_snd_addr);

// ��ʱ�ص�
int callback_timeout();
// ���ջص�
int callback_receive(SOCKET             sock,
                     const sockaddr_in* addr,
                     const char*        msg,
                     unsigned int       size);

// ���Ź�����
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
  // ���ε��ͻ���,��־�ϴ��ȼ�,�ϴ��˿ں�
  unsigned int  n_snd_level     = 3;  
  unsigned int  n_recv_port     = DEF_SERVER_PORT;
  // ת����������ַ;��ʽ[IP:PORT]
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

  // ������
  for (int i = 0; i < argc; i++) {
    if (strlen(argv[i]) >= 2) {
      VZ_PRINT("%s\n", argv[i]);
      switch (argv[i][1]) {
      case 'H':               // ����
        printf("applet <-H> <-V[123]> -<P[port]> <-D> <-S[ip:port]>\n");
        break;

      // ��ӡ���ʹ��
      case 'D':
      case 'd':
        k_en_stdout = true;
        break;

      // ����͸����������ַ
      case 'S':
        sscanf(argv[i], "-S%s", s_snd_addr);
        break;
      case 's':
        sscanf(argv[i], "-s%s", s_snd_addr);
        break;

      // ���ý��ն˿�
      case 'P':
        sscanf(argv[i], "-P%d", &n_recv_port);
        break;
      case 'p':             //
        sscanf(argv[i], "-p%d", &n_recv_port);
        break;

      // ���伶��
      case 'V':
        scanf(argv[i], "-V%d", &n_snd_level);
        break;
      case 'v':
        sscanf(argv[i], "-v%d", &n_snd_level);
        break;
      }
    }
  }

  // �������
  k_shm_arg.Open();
  ret = InitShareParam(DEF_SERVER_HOST, n_recv_port, n_snd_level);

  // ����־�洢
  ret = InitLogRecord(DEF_RECORD_PATH,
                      DEF_LOG_REC_FILE,
                      DEF_WDG_REC_FILE);
  if (ret < 0) {
    return ret;
  }
  k_wdg_rec.Print("stdout level %d.\n", n_snd_level);

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
  vzlogging::CVzLogDataPool::ExitLogMemPool();
  return 0;
}

/***static function******************************************************/
// ��ʼ�������ڴ�
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

// ��ʼ����־��¼
int InitLogRecord(const char* path,
                  const char* log_fname,
                  const char* wdg_fname) {
  int ret = -1;

  // ��ͨ��־
  ret = k_log_rec.Open(path, log_fname, DEF_LOG_FILE_SIZE);
  if (ret < 0) {
    VZ_PRINT("log record open failed.%d.\n", ret);
    return ret;
  }

  // ���Ź���־
  ret = k_wdg_rec.Open(path, wdg_fname, DEF_WDG_FILE_SIZE);
  if (ret < 0) {
    VZ_PRINT("wdg record open failed.%d.\n", ret);
    return ret;
  }

  return 0;
}

// ��ʼ���������
int InitSrvSocket(const char* ip, unsigned short port,
                  const char* s_snd_addr) {
  k_srv_sock.SetCallback(callback_receive,
                         callback_timeout);

  int ret = -1;
  // ���������
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

// ���ջص�
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
    // ��ͨ��־
    k_log_rec.Write(s_msg, n_msg);
  } else {
    // ����,���͸����Ź�����
    WatchdogProcess(s_msg, n_msg);
  }

  // ͸�����յ�����־��Ϣ
  k_srv_sock.TransMsgToServer(s_msg, n_msg);
  return n_msg;
}

// ��ʱ�ص�
int callback_timeout() {
  time_t n_now = time(NULL);
  static time_t n_last = n_now;
  if ((n_now - n_last) > 10) {
    // ���̳�ʱ���
    for (int i = 0; i < DEF_PER_DEV_PROCESS_MAX; i++) {
      if (k_pro_heart[i].app_name[0] != '\0') {
        if ((n_now - k_pro_heart[i].last_heartbeat) > 45) {
          if ((k_pro_heart[i].no_heart_times++) >= 2) {
            k_wdg_rec.Print("%s %d %d no heartbeat, %s.\n",
                            k_pro_heart[i].app_name,
                            k_pro_heart[i].pid,
                            k_pro_heart[i].ppid,
                            k_pro_heart[i].msg);
            // ��������
            // �����豸
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

// ���Ź�����
int WatchdogProcess(const char* s_msg, unsigned int n_msg) {
  unsigned int pid, ppid;
  char s_app_name[32] = {0};
  sscanf(s_msg, "H %04d %04d %s ", &pid, &ppid, s_app_name);

  // ���½�������ʱ��
  int n_empty = 0;  // δʹ�ý�����Ϣ�ռ�
  for (int i = 0; i < DEF_PER_DEV_PROCESS_MAX; i++) {
    if (k_pro_heart[i].ppid == ppid &&                      // �߳�&&������
        !strncmp(k_pro_heart[i].app_name, s_app_name, 32)) {
      k_pro_heart[i].no_heart_times = 0;
      k_pro_heart[i].last_heartbeat = time(NULL);
      return 0;
    }

    if (k_pro_heart[i].app_name[0] == '\0') {
      n_empty = i;
    }
  }

  // �½���
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

