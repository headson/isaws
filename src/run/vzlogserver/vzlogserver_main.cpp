/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : ��־\���Ź����������
************************************************************************/
#include <time.h>   // ��ʱ
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "vzlogging/base/vzlogdef.h"
#include "vzlogging/server/cvzlogmanage.h"

#ifdef _WIN32
#include <windows.h>
#include <winsock2.h>
#include "vzlogging/server/getopt.h"
#else
#include "getopt.h"
#endif

#ifdef HISI_R
#include "vzlogging/dog/vzhardwaredog.h"
#include "vzlogging/rtc/vz_r_rtc.h"
#define MAX_HISIR_FEED_DOG_TIME 2500     // HISI R���Ĭ��Ӳ��ι��ʱ��Ϊ2.5s
static int hisir_feed_dog_times = 0;
#endif

/*ι���ص�*/
int sys_feeddog();

#ifdef HISI_R
/*����15sʱ��ι�� ����WATCHDOG ����ʱ��...*/
void FeedHardwareDog();
#endif  // HISI_R

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
#ifdef HISI_R
    //15sֻιӲ����
    FeedHardwareDog();
#else
    VZ_ERROR("callback_feeddod.\n");
    int i = 0;
    if (k_en_20s_wait == 1) {
      VZ_ERROR("wait some time.\n");
      for (i = 0; i < 18 * 1000; i += 5) { // 18s
        sys_feeddog();
        sleep(1);
      }
    }
#endif  // HISI_R
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
static int wdt_fd = 0;
static int feed_times = 0;
int sys_feeddog() {
#ifndef _WIN32
#ifdef HISI_R
  /* ����Ƿ����Ӳ��ι�� */
  hisir_feed_dog_times += 5;
  if (hisir_feed_dog_times >= MAX_HISIR_FEED_DOG_TIME) {
    hisir_feed_dog_times = 0;
    vzlog::PWMWatchDog::FeedPwmWatchDog();
  }
#else
  if (wdt_fd <= 0) {
    wdt_fd = open("/dev/watchdog", O_WRONLY);
    if (wdt_fd < 0) {
      perror("open device /dev/watchdog");
      exit(1);
    }

    ioctl(wdt_fd, WDIOC_SETOPTIONS, WDIOS_ENABLECARD);
    ioctl(wdt_fd, WDIOC_SETTIMEOUT, 6);
  }

  if (wdt_fd > 0) {
    feed_times += 5;  // 5ms
    if (feed_times >= 1000) {   // 1S
      feed_times = 0;
      ioctl(wdt_fd, WDIOC_KEEPALIVE, 1);
    }
  }
#endif  // HISI_R
#endif
  return 0;
}

#ifdef HISI_R
void FeedHardwareDog() {
  // start hisi_r hardware watch dog
  if (!vzlog::PWMWatchDog::StartPwmWatchDog()) {
    VZ_ERROR("start hardware watch dog failed.");
    return;
  }

  for (int i = 0; i < 15; i++) {
    vzlog::PWMWatchDog::FeedPwmWatchDog();
    sleep(1);
  }
}
#endif  // HISI_R

