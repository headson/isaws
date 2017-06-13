#include "cmulticast.h"
#include "base/stdafx.h"

CMultiCast::CMultiCast()
  : VSocket() {
}

CMultiCast::~CMultiCast() {
  /*退出多播组*/
  SetOption(IPPROTO_IP, IP_DROP_MEMBERSHIP, &st_mreq_, sizeof(st_mreq_));

  Close();
}

int32_t CMultiCast::Open(const CInetAddr& c_mcast_addr, bool nonblocking, bool resue, bool client) {
  handler_ = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(INVALID_SOCKET == handler_) {
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
  if(nonblocking) {
    set_socket_nonblocking(handler_);
  }

  //设置端口复用
  if(resue) {
    int32_t val = 1;
    SetOption(SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int32_t));
  }

  int32_t ret = Bind(c_mcast_addr);
  if(ret == -1) {
    return ret;
  }

  /*加入多播组*/
  st_mreq_.imr_multiaddr.s_addr = inet_addr(c_mcast_addr.IP2String().c_str());
  st_mreq_.imr_interface.s_addr = htonl(INADDR_ANY);
  if (SetOption(IPPROTO_IP, IP_ADD_MEMBERSHIP, &st_mreq_, sizeof(st_mreq_)) < 0) {
    perror("setsockopt");
    return -1;
  }

  c_mcast_addr_ = c_mcast_addr;
  return ret;
}

int32_t CMultiCast::Recv(void* pData, uint32_t nData) {
  CInetAddr c_remote_addr;
  return VSocket::Recv(pData, nData, c_remote_addr);
}

int32_t CMultiCast::Send(const void* pData, uint32_t nData) {
  return VSocket::Send(pData, nData, c_mcast_addr_);
}

