#include "base/vtypes.h"
#include "base/stdafx.h"

#include "network/vevent.h"
#include "network/cmulticast.h"

#if 1
int main(int argc, char* argv[]) {
  app_init();
  InitLogging(argc, argv);

  int32_t n_ret = 0;
  EVT_LOOP c_evt_loop;
  n_ret = c_evt_loop.Start();
  if (n_ret != 0) {
    LOG(L_ERROR) << "start failed " << n_ret;
    return -1;
  }

  CMultiCast cmcast;

  CInetAddr c_addr("0.0.0.0", 12345);
  cmcast.Open(&c_addr, &c_evt_loop);

  struct ip_mreq  st_mreq_;
  st_mreq_.imr_interface.s_addr = htonl(INADDR_ANY);
  st_mreq_.imr_multiaddr.s_addr = inet_addr(DETECT_DEVICE_ADDR);
  if (cmcast.SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, &st_mreq_, sizeof(st_mreq_)) < 0) {
    perror("setsockopt");
    return -1;
  }

  cmcast.Send("hello worlds.\n", strlen("hello worlds.\n"), c_addr);
  while (1) {
    //cmcast.Send("hello worlds.\n", strlen("hello worlds.\n"), c_addr);
    c_evt_loop.Runing();
    _sleep(1000);
  }
  return 0;
}
#else

#include <stdio.h>  
#include <winsock2.h>  
#include <ws2tcpip.h>  
#pragma comment(lib, "ws2_32.lib")  

int main()
{
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);

  SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);

  int n = 0; // 注意， 这个n值很重要， 下面我会讲到  
  setsockopt(sock, IPPROTO_IP, IP_MULTICAST_TTL, (char*)&n, sizeof(n));

  sockaddr_in addr;
  addr.sin_addr.S_un.S_addr = inet_addr("234.2.2.2");
  addr.sin_family = AF_INET;
  addr.sin_port = htons(8888);

  static int i = 0;
  while (1)
  {
    char buf[100] = { 0 };
    sprintf(buf, "blablablabla:%d", i++);
    sendto(sock, buf, strlen(buf) + 1, 0, (sockaddr*)&addr, sizeof(sockaddr));
    Sleep(500);
  }

  closesocket(sock);
  WSACleanup();

  return 0;
}
#endif
