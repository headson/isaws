/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : ��־\���Ź����������
************************************************************************/
#include <time.h>   // ��ʱ
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "vzlogging/base/vzbases.h"
#include "vzlogging/server/cvzlogmanage.h"

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include "vzlogging/server/getopt.h"
#else
#include "getopt.h"
#endif

/*ι���ص�*/
int sys_feeddog();

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
    "      -s  <PATH> change log save path\n");
  printf(
    "      -d  print all log in console\n");
  printf(
    "      -h  print usage\n");
  printf("\n");
  exit(1);
}

#ifdef _WIN32

inline void usleep(unsigned int us) {
  if (us < 1000) {
    ::Sleep(1);
  } else {
    ::Sleep(us / 1000);
  }
}
#endif

static unsigned int  k_en_stdout   = 0;              // Ĭ�Ϲر���־����̨��ӡ
static unsigned int  k_en_watchdog = 1;

static unsigned int  k_en_20s_wait = 0;

static unsigned int  k_log_level = 3;                // ��־�ϴ��ȼ�,
static unsigned int  k_log_port = DEF_SERVER_PORT;   // �ϴ��˿ں�

static char          k_tran_addr[32] = { 0 };  // ת����������ַ;��ʽ[IP:PORT]
static char          k_log_path[64] = DEF_RECORD_PATH;
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
  // system("killall watchdog");
  system("killall feeddog");
  usleep(1000);

  int cmd = 0;
  if (argc >= 2) {
    sscanf(argv[1], "%d", &cmd);
    if (cmd == 1) {
      k_en_20s_wait = 1;
    }
  }

  int opt = 0;
  while ((opt = getopt(argc, argv, "v:V:p:P:s:S:dDhHfFwW")) != -1) {
    switch (opt) {
    case 'v':   // ���������������
    case 'V':
      k_log_level = atoi(optarg);
      break;

    case 'f':   // ���Ź��ر�
    case 'F':
      k_en_watchdog = 0;
      break;

    case 'p':   // �������紫��˿�
    case 'P':
      k_log_port = atoi(optarg);
      break;

    case 's':   // ��־�洢·��
    case 'S': {
      memset(k_log_path, 0, 64);
      strncpy(k_log_path, optarg, 64);
    }
    break;

    case 'd':   // ���ñ��ش�ӡ
    case 'D':
      k_en_stdout = 1;
      break;

    case 'h':   // ����
    case 'H':
      PrintUsage();
      break;
    }
  }

#ifdef HISI_R
  /*set system rtc time*/
  if (!vzlogging::HisiRtcContrl::SetRTCTimeToLocal()) {
    VZ_PRINT("Set rtc time to system error.");
  }
#endif  // HISI_R

  if (1 == k_en_watchdog) {
#ifndef _WIN32
#endif  // WIN32
  } else {
    VZ_ERROR("close watchdog.\n");
  }

  VZ_ERROR("applet compile time: %s %s\n", __TIME__, __DATE__);

  vzlogging::CVzLogManage *pmgr =
    vzlogging::CVzLogManage::Instance();
  if (pmgr == NULL) {
    VZ_ERROR("get instance failed.\n");
    return -1;
  }

  pmgr->Start(k_log_path, k_log_port, k_tran_addr);

  pmgr->SetLogAddrAndLevel(DEF_SERVER_HOST,
                           k_log_port, k_log_level);
  if (1 == k_en_stdout) {
    pmgr->SetEnableStdout();
  }

  if (0 == k_en_watchdog) {
    pmgr->SetDisableWatchdog();
  }

  while (true) {
    pmgr->RunLoop(5*1000);  // �ȴ��������

    if (1 == k_en_watchdog) {
      sys_feeddog();
    }
  }

#ifdef WIN32
  WSACleanup();
#endif
  return 0;
}

/*ι���ص�*/
#ifndef _WIN32
#include <errno.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <linux/watchdog.h>
#endif
int sys_feeddog() {
#ifndef _WIN32

#endif
  return 0;
}

