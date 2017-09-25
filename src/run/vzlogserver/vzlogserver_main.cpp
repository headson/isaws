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

static void PrintUsage();
static void HardwareFeeddog();

#ifdef _WIN32

inline void usleep(unsigned int us) {
  if (us < 1000) {
    ::Sleep(1);
  } else {
    ::Sleep(us / 1000);
  }
}
#endif

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

  unsigned int  is_print = 0;              // Ĭ�Ϲر���־����̨��ӡ
  unsigned int  is_reboot = 1;

  unsigned int  log_level = 3;                // ��־�ϴ��ȼ�,
  unsigned int  log_sport = DEF_SERVER_PORT;   // �ϴ��˿ں�

  char          tran_addr[32] = { 0 };  // ת����������ַ;��ʽ[IP:PORT]
  char          log_path[256] = DEF_RECORD_PATH;
  
  int opt = 0;
  while ((opt = getopt(argc, argv, "v:V:p:P:s:S:dDhHfFwW")) != -1) {
    switch (opt) {
    case 'v':   // ���������������
    case 'V':
      log_level = atoi(optarg);
      break;

    case 'f':   // ���Ź��ر�
    case 'F':
      is_reboot = 0;
      break;

    case 'p':   // �������紫��˿�
    case 'P':
      log_sport = atoi(optarg);
      break;

    case 's':   // ��־�洢·��
    case 'S': {
      memset(log_path, 0, 256);
      strncpy(log_path, optarg, 255);
    }
    break;

    case 'd':   // ���ñ��ش�ӡ
    case 'D':
      is_print = 1;
      break;

    case 'h':   // ����
    case 'H':
      PrintUsage();
      break;
    }
  }
  
  VZ_ERROR("applet compile time: %s %s\n", __TIME__, __DATE__);

  vzlogging::CVzLogManage *pmgr =
    vzlogging::CVzLogManage::Instance();
  if (pmgr == NULL) {
    VZ_ERROR("get instance failed.\n");
    return -1;
  }

  pmgr->Start(log_path, log_sport, tran_addr);
  pmgr->SetLogAddrAndLevel(DEF_SERVER_HOST,
                           log_sport, log_level);
  if (1 == is_print) {
    pmgr->SetEnableStdout();
  }

  if (0 == is_reboot) {
    pmgr->SetDisableWatchdog();
  }

  unsigned int old_sec = vzlogging::GetSysSec();
  unsigned int now_sec = vzlogging::GetSysSec();
  while (true) {
    pmgr->RunLoop(1*1000);  // �ȴ��������

    now_sec = vzlogging::GetSysSec();
    if ((now_sec - old_sec) >= 1) {
      old_sec = now_sec;
      HardwareFeeddog();
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
void HardwareFeeddog() {
}

static void PrintUsage() {
  printf("\n");
  printf("\n");
  printf("  usage:\n");
  printf("\n");
  printf("      -v  <trans level 123> Defualt value is 3 L_ERROR send to server\n");
  printf("      -p  <trans port> Defualt value is 5760\n");
  printf("      -s  <PATH> change log save path\n");
  printf("      -d  print all log in console\n");
  printf("      -h  print usage\n");
  printf("\n");
  exit(1);
}


