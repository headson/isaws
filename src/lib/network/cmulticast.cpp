#include "cmulticast.h"
#include "base/stdafx.h"

CMultiCast::CMultiCast()
  : CSocketInterface() {
}

CMultiCast::~CMultiCast() {
  /*退出多播组*/
  SetOption(IPPROTO_IP, IP_DROP_MEMBERSHIP, &st_mreq_, sizeof(st_mreq_));

  Close();
}

int32_t CMultiCast::Open(const CInetAddr *p_mcast_addr, const EVT_LOOP* p_evt_loop) {
  if (!p_evt_loop || !p_evt_loop) {
    LOG(L_ERROR)<<"param error.";
    return -1;
  }

  handler_ = socket(AF_INET, SOCK_DGRAM, 0);
  if(INVALID_SOCKET == handler_) {
    LOG(L_ERROR)<<"socket failed.";
    return -1;
  }

#ifdef WIN32 //解决WINSOCK2 的UDP端口ICMP的问题
#if defined(SIO_UDP_CONNRESET)
  int32_t byte_retruned = 0;
  bool new_be = false;
  int32_t status = WSAIoctl(handler_, SIO_UDP_CONNRESET,
                            &new_be, sizeof(new_be), NULL, 0, (LPDWORD)&byte_retruned, NULL, NULL);
#endif
#endif

  //设置异步模式
  set_socket_nonblocking(handler_);

  //设置端口复用
  int32_t val = 1;
  SetOption(SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int32_t));

  int32_t ret = 0;
  ret = ::bind(handler_,
               (struct sockaddr *)p_mcast_addr->GetAddr(),
               sizeof(sockaddr_in));
  if (ret == -1) {
    perror("bind failed.\n");
    LOG(L_ERROR) << "bind failed." << error_no();
    return ret;
  }

  //int n = 0; // 注意， 这个n值很重要， 下面我会讲到  
  //SetOption(IPPROTO_IP, IP_MULTICAST_TTL, (char*)&n, sizeof(n));

  /*加入多播组*/
  //st_mreq_.imr_multiaddr.s_addr = inet_addr(p_mcast_addr->IP2String().c_str());
  //st_mreq_.imr_interface.s_addr = htonl(INADDR_ANY);
  //if (SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, &st_mreq_, sizeof(st_mreq_)) < 0) {
  //  perror("setsockopt");
  //  return -1;
  //}

  ret = InitEvent(p_evt_loop);
  if (ret != 0) {
    LOG(L_ERROR)<<"init event failed.";
  }

  c_mcast_addr_ = *p_mcast_addr;
  return ret;
}

//int32_t CMultiCast::Recv(void* pData, uint32_t nData) {
//  CInetAddr c_remote_addr;
//  return VSocket::Recv(pData, nData, c_remote_addr);
//}
//
//int32_t CMultiCast::Send(const void* pData, uint32_t nData) {
//  return VSocket::Send(pData, nData, c_mcast_addr_);
//}

int32_t CMultiCast::OnRecv(int32_t n_evt, const void* p_usr_arg) {
  char s_data[1024] = {0};
  int32_t n_data = Recv(s_data, 1024);
  printf("%s[%d].\n", s_data, n_data);
  return n_data;
}

int32_t CMultiCast::OnSend(int32_t n_evt, const void* p_usr_arg) {
  return 0;
}

