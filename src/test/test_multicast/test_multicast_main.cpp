#include "vzbase/base/basictypes.h"
#include "vzbase/helper/stdafx.h"

#include "vzconn/multicast/cmcastsocket.h"
using namespace vzconn;

#if 0

class CClientProcess : public CClientInterface {
 public:
  virtual int32 HandleRecvPacket(VSocket      *p_cli,
                                 const uint8  *p_data,
                                 uint32        n_data,
                                 uint16        n_flag) {
    ((char*)p_data)[n_data] = '\0';
    LOG(L_INFO) << "  "<<(char*)p_data;
    return 0;
  }
  virtual int32 HandleSendPacket(VSocket *p_cli) {
    return 0;
  }
  virtual void  HandleClose(VSocket *p_cli) {
  }
};

static const char DEF_CERTER_IP[] = "224.5.6.2";
static const int  DEF_CENTER_PORT = 20004;

int32 timer_cb(SOCKET          fd,
               short           events,
               const void      *p_usr_arg) {
  ((CMCastSocket*)p_usr_arg)->SendUdpData(DEF_CERTER_IP, DEF_CENTER_PORT,
                                          "hello wolrds\0", 13);
  return 0;
}

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);
#ifdef WIN32
  ShowVzLoggingAlways();
#endif

  int32_t n_ret = 0;
  EVT_LOOP c_evt_loop;
  n_ret = c_evt_loop.Start();
  if (n_ret != 0) {
    LOG(L_ERROR) << "start failed " << n_ret;
    return -1;
  }

  CClientProcess c_cli_proc;
  CMCastSocket *cmcast =
    CMCastSocket::Create(&c_evt_loop, &c_cli_proc);

  n_ret = cmcast->Open(DEF_CERTER_IP, DEF_CENTER_PORT);
  if (n_ret != 0) {
    LOG(L_ERROR) << "open failed.";
    return -1;
  }

  EVT_TIMER evt_timer;
  evt_timer.Init(&c_evt_loop, timer_cb, cmcast);
  evt_timer.Start(1000, 1000);

  while (true) {
    c_evt_loop.RunLoop();
  }
  return 0;
}
#else

#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[])
{
  int sockfd;
  struct ifconf ifconf;
  struct ifreq *ifreq;
  char buf[512];//缓冲区
  //初始化ifconf
  ifconf.ifc_len = 512;
  ifconf.ifc_buf = buf;
  if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
  {
    perror("socket");
    exit(1);
  }
  ioctl(sockfd, SIOCGIFCONF, &ifconf); //获取所有接口信息

  //接下来一个一个的获取IP地址
  ifreq = (struct ifreq*)ifconf.ifc_buf;
  printf("ifconf.ifc_len:%d\n", ifconf.ifc_len);
  printf("sizeof (struct ifreq):%d\n", sizeof (struct ifreq));

  for (int i = (ifconf.ifc_len / sizeof (struct ifreq)); i > 0; i--)
  {
    if (ifreq->ifr_flags == AF_INET){ //for ipv4
      printf("name =[%s]\n", ifreq->ifr_name);
      printf("local addr = [%s]\n", inet_ntoa(((struct sockaddr_in*)&(ifreq->ifr_addr))->sin_addr));
      ifreq++;
    }
  }

  getchar();//system("pause");//not used in linux 
  return 0;
}

#endif