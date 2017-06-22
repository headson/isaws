#include "basictypes.h"

#include "stdafx.h"

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
    LOG(L_ERROR)<<"start failed "<<n_ret;
    return -1;
  }

  CMultiCast cmcast;

  CInetAddr c_addr("0.0.0.0", 12345);
  cmcast.Open(&c_addr, &c_evt_loop);

  int n = 0; // 注意， 这个n值很重要， 下面我会讲到  
  cmcast.SetOption(IPPROTO_IP, IP_MULTICAST_TTL, (char*)&n, sizeof(n));

  CInetAddr c_mcast(DETECT_DEVICE_ADDR, 12345);
  while (1) {
    cmcast.Send("hello worlds.\n", strlen("hello worlds.\n"), c_addr);
    //c_evt_loop.Runing();
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

  sockaddr_in addr;
  addr.sin_family = AF_INET;
  addr.sin_addr.S_un.S_addr = INADDR_ANY;
  addr.sin_port = htons(8888);

  bind(sock, (sockaddr*)&addr, sizeof(addr));

  // 加入组播啦  
  ip_mreq multiCast;
  multiCast.imr_interface.S_un.S_addr = INADDR_ANY;
  multiCast.imr_multiaddr.S_un.S_addr = inet_addr("234.2.2.2");
  setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&multiCast, sizeof(multiCast));

  int len = sizeof(sockaddr);
  while (true)
  {
    char buf[100] = { 0 };
    recvfrom(sock, buf, sizeof(buf)-1, 0, (sockaddr*)&addr, &len);
    printf("%s\n", buf);
  }

  closesocket(sock);
  WSACleanup();

  return 0;
}
#endif