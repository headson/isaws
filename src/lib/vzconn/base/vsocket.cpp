/************************************************************************/
/* Author      : SoberPeng 2017-06-14
/* Description :
/************************************************************************/
#include "vzconn/base/vsocket.h"

#include <string.h>

#include "stdafx.h"
#include "vzconn/base/connhead.h"
#include "vzconn/base/byteorder.h"

namespace vzconn {

CInetAddr::CInetAddr() {
  memset(&c_sock_addr_, 0, sizeof(c_sock_addr_));
  c_sock_addr_.sin_family = AF_INET;
  c_sock_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
  c_sock_addr_.sin_port = htons(0);
}

CInetAddr::CInetAddr(const sockaddr_in& addr) {
  memcpy(&c_sock_addr_, &addr, sizeof(struct sockaddr_in));
}

CInetAddr::CInetAddr(uint32 ip, uint16 port) {
  memset(&c_sock_addr_, 0, sizeof(struct sockaddr_in));

  SetIP(ip);
  SetPort(port);

  c_sock_addr_.sin_family = AF_INET;
}

CInetAddr::CInetAddr(const CInetAddr &addr) {
  c_sock_addr_ = addr.c_sock_addr_;
}

CInetAddr::CInetAddr(const char* hostname, uint16 port) {
  memset(&c_sock_addr_, 0, sizeof(struct sockaddr_in));

  SetIP(hostname);
  SetPort(port);

  c_sock_addr_.sin_family = AF_INET;
}

CInetAddr::~CInetAddr() {
}

bool CInetAddr::IsIPV4(const char *s) {
  bool rv = true;
  int32 tmp1, tmp2, tmp3, tmp4, i;

  while(true) {
    i = sscanf(s, "%d.%d.%d.%d", &tmp1, &tmp2, &tmp3, &tmp4);

    if(i != 4) {
      rv = false;
      break;
    }

    if((tmp1 > 255) || (tmp2 > 255) || (tmp3 > 255) || (tmp4 > 255)) {
      rv = false;
      break;
    }

    const char *pChar;
    for(pChar = s; *pChar != 0; pChar++) {
      if((*pChar != '.') && ((*pChar < '0') || (*pChar > '9'))) {
        rv = false;
        break;
      }
    }
    break;
  }

  return rv;
}

bool CInetAddr::IsNull() const {
  return (ntohs(c_sock_addr_.sin_port) == 0);
}

CInetAddr& CInetAddr::operator = (const CInetAddr& addr) {
  this->c_sock_addr_ = addr.c_sock_addr_;
  return *this;
}

CInetAddr& CInetAddr::operator =(const sockaddr_in &addr) {
  this->c_sock_addr_ = addr;
  return *this;
}

CInetAddr& CInetAddr::operator =(const std::string &addr_str) {
  std::string::size_type pos = addr_str.find(':');
  if(std::string::npos == pos) {
    return *this;
  }

  SetPort(static_cast<uint16>(strtoul(addr_str.substr(pos + 1).c_str(), NULL, 10)));
  SetIP(addr_str.substr(0, pos).c_str());

  return *this;
}

void CInetAddr::SetIP(uint32 ip) {
  c_sock_addr_.sin_addr.s_addr = htonl(ip);
}

void CInetAddr::SetIP(const char* hostname) {
  c_sock_addr_.sin_addr.s_addr = inet_addr(hostname);
  if (c_sock_addr_.sin_addr.s_addr == (uint32)-1) { //地址为0.0.0.0，无效地址
    if (hostent * pHost = gethostbyname(hostname)) {
      c_sock_addr_.sin_addr.s_addr = (*reinterpret_cast<uint32 *>(pHost->h_addr_list[0]));
    }
  }
}

void CInetAddr::SetPort(uint16 port) {
  c_sock_addr_.sin_port = htons(port);
}

uint32 CInetAddr::GetIP() const {
  return ntohl(c_sock_addr_.sin_addr.s_addr);
}

uint16 CInetAddr::GetPort() const {
  return ntohs(c_sock_addr_.sin_port);
}

const sockaddr_in& CInetAddr::IPAddr() const {
  return c_sock_addr_;
}

sockaddr_in* CInetAddr::GetAddr() {
  return &c_sock_addr_;
}

const sockaddr_in* CInetAddr::GetAddr() const {
  return &c_sock_addr_;
}

const std::string CInetAddr::ToString() const {
  char ip_str[32] = {0};
#ifdef WIN32
  sprintf(ip_str, "%d.%d.%d.%d", c_sock_addr_.sin_addr.S_un.S_un_b.s_b1,
          c_sock_addr_.sin_addr.S_un.S_un_b.s_b2,
          c_sock_addr_.sin_addr.S_un.S_un_b.s_b3,
          c_sock_addr_.sin_addr.S_un.S_un_b.s_b4);
#else
  ::inet_ntop(c_sock_addr_.sin_family, &(c_sock_addr_.sin_addr), ip_str, sizeof(ip_str));
#endif

  sprintf(ip_str, "%s:%d", ip_str, GetPort());
  return ip_str;
}

const std::string CInetAddr::IP2String() const {
  char ip_str[32] = {0};

#ifdef WIN32
  sprintf(ip_str, "%d.%d.%d.%d", c_sock_addr_.sin_addr.S_un.S_un_b.s_b1,
          c_sock_addr_.sin_addr.S_un.S_un_b.s_b2,
          c_sock_addr_.sin_addr.S_un.S_un_b.s_b3,
          c_sock_addr_.sin_addr.S_un.S_un_b.s_b4);
#else
  ::inet_ntop(c_sock_addr_.sin_family, &(c_sock_addr_.sin_addr), ip_str, sizeof(ip_str));
#endif

  return ip_str;
}

int32 CInetAddr::ToIpcAddr(char *p_addr, uint32 n_addr) const {
  memset(p_addr, 0, n_addr);
  int32 n = snprintf(p_addr, n_addr-1,
                     "/tmp/_%d.sock", GetPort());

  return n;
}

bool CInetAddr::operator ==(const CInetAddr& addr) const {
  return (ntohl(c_sock_addr_.sin_addr.s_addr) == ntohl(addr.c_sock_addr_.sin_addr.s_addr)
          && ntohs(c_sock_addr_.sin_port) == ntohs(addr.c_sock_addr_.sin_port));
}

bool CInetAddr::operator !=(const CInetAddr &addr) const {
  return !((*this) == addr);
}

bool CInetAddr::operator <(const CInetAddr &addr) const {
  if(ntohl(c_sock_addr_.sin_addr.s_addr) < ntohl(addr.c_sock_addr_.sin_addr.s_addr))
    return true;
  else if(ntohl(c_sock_addr_.sin_addr.s_addr) > ntohl(addr.c_sock_addr_.sin_addr.s_addr))
    return false;
  else {
    return (ntohs(c_sock_addr_.sin_port) < ntohs(addr.c_sock_addr_.sin_port) ? true : false);
  }
}

bool CInetAddr::operator >(const CInetAddr& addr) const {
  if(ntohl(c_sock_addr_.sin_addr.s_addr) > ntohl(addr.c_sock_addr_.sin_addr.s_addr))
    return true;
  else if(ntohl(c_sock_addr_.sin_addr.s_addr) < ntohl(addr.c_sock_addr_.sin_addr.s_addr))
    return false;
  else {
    return (ntohs(c_sock_addr_.sin_port) > ntohs(addr.c_sock_addr_.sin_port) ? true : false);
  }
}

//////////////////////////////////////////////////////////////////////////
VSocket::VSocket(CClientInterface *cli_hdl)
  : handler_(INVALID_SOCKET)
  , rw_flag_(0)
  , cli_hdl_ptr_(cli_hdl) {
#ifndef WIN32
  rw_flag_ = MSG_NOSIGNAL | MSG_DONTWAIT;
#endif
}

VSocket::~VSocket() {
  Close();
}

SOCKET VSocket::GetSocket() const {
  return handler_;
}

void VSocket::SetSocket(SOCKET handle) {
  handler_ = handle;
}

void VSocket::Close() {
  if(handler_ != INVALID_SOCKET) {
    closesocket(handler_);
    handler_ = INVALID_SOCKET;
  }
}

bool VSocket::IsOpen() const {
  return (handler_ != INVALID_SOCKET);
}

int32 VSocket::SetOption(int level, int option, void *optval, int optlen) const {
  return setsockopt(GetSocket(), level, option, (char *)optval, optlen);
}

int32 VSocket::GetOption(int level, int option, void *optval, int *optlen) const {
#ifdef WIN32
  return getsockopt(GetSocket(), level, option, (char *)optval, optlen);
#else
  return getsockopt(GetSocket(), level, option, (char *)optval, (socklen_t *)optlen);
#endif
}

/*****************************************************************************
* Author        : Sober.Peng 28:12:2016
* Description   : 网络数据接收
* Param         : pData[OUT] 接收数据，nData[IN] 缓存大小
* Return        : >0 数据长度，0 没收到数据，-1 断网
******************************************************************************/
int32 VSocket::Recv(void *pData, uint32 nData) {
  int32 nRet = -1;
  if(!IsOpen()) {
    return nRet;
  }

  nRet = ::recv(GetSocket(), (char *)pData, nData, rw_flag_);
  if (nRet > 0) {
    return nRet;
  }

  if (nRet < 0 && error_no() == XEAGAIN) {
    return 0;
  }

  return -1;
}

/*****************************************************************************
* Author        : Sober.Peng 28:12:2016
* Description   : 网络数据接收
* Param         : pData[OUT] 接收数据，nData[IN] 缓存大小
* Return        : >0 数据长度，0 没收到数据，-1 断网
******************************************************************************/
int32 VSocket::Recv(void* pData, uint32 nData, CInetAddr& cRemoteAddr) {
  int32 nRet = -1;
  if(!IsOpen()) {
    return nRet;
  }

#ifdef WIN32
  int32 from_size = sizeof(struct sockaddr_in);
#else
  uint32 from_size = sizeof(struct sockaddr_in);
#endif

  nRet = ::recvfrom(GetSocket(), (char *)pData, nData,
                    rw_flag_, (struct sockaddr *)cRemoteAddr.GetAddr(), &from_size);
  if (nRet > 0) {
    return nRet;
  }

  if (nRet < 0 && error_no() == XEAGAIN) {
    return 0;
  }

  return -1;
}

/*****************************************************************************
* Author        : Sober.Peng 28:12:2016
* Description   : 网络发送接收
* Param         : pData[IN] 发送数据，nData[IN] 缓存大小
* Return        : >0 数据长度，0 没发送数据，-1 断网
******************************************************************************/
int32 VSocket::Send(const void *buf, uint32 buf_size) {
  int32 nRet = -1;
  if(!IsOpen()) {
    return nRet;
  }

  nRet = ::send(GetSocket(), (const char *)buf, buf_size, rw_flag_);
  if (nRet > 0) {
    return nRet;
  }

  if (nRet < 0 && error_no() == XEAGAIN) {
    return 0;
  }
  return nRet;
}

/*****************************************************************************
* Author        : Sober.Peng 28:12:2016
* Description   : 网络发送接收
* Param         : pData[IN] 发送数据，nData[IN] 缓存大小
* Return        : >0 数据长度，0 没发送数据，-1 断网
******************************************************************************/
int32 VSocket::Send(const void* buf, uint32 buf_size, const CInetAddr& remote_addr) {
  int32 nRet = -1;
  if(!IsOpen()) {
    return nRet;
  }

  nRet = ::sendto(GetSocket(), (const char *)buf, buf_size,
                  rw_flag_, (struct sockaddr *)remote_addr.GetAddr(), sizeof(sockaddr_in));
  if (nRet > 0) {
    return nRet;
  }

  if (nRet < 0 && error_no() == XEAGAIN) {
    return 0;
  }

  return nRet;
}

//////////////////////////////////////////////////////////////////////////
int32 CClientInterface::NetHeadSize() {
  return sizeof(NetHead);
}

int32 CClientInterface::NetHeadParse(const void *p_data,
                                     uint32      n_data) {
  if (n_data < NetHeadSize()) {
    return 0;
  }

  int32 n_len = 0;
  if ((((uint32)p_data) % sizeof(uint32)) == 0) {
    // 对齐解析
    NetHead* p_head = (NetHead*)p_data;
    if (p_head->mark[0] == NET_MARK_0
        && p_head->mark[1] == NET_MARK_1) {
      n_len = NetHeadSize();
      n_len += (ORDER_NETWORK == VZ_ORDER_BYTE) ?
               NetworkToHost32(p_head->data_size) : p_head->data_size;
    } else {
      return -1;
    }
  } else {
    // 未对齐解析
    NetHead c_head;
    memcpy(&c_head, p_data, NetHeadSize());
    if (c_head.mark[0] == NET_MARK_0
        && c_head.mark[1] == NET_MARK_1) {
      n_len = NetHeadSize();
      n_len += (ORDER_NETWORK == VZ_ORDER_BYTE) ?
               NetworkToHost32(c_head.data_size) : c_head.data_size;
    } else {
      return -1;
    }
  }
  return n_len;
}

int32 CClientInterface::NetHeadPacket(void  *p_data,
                                      uint32 n_data,
                                      uint32 n_body,
                                      uint16 n_flag) {
  if (p_data == NULL || n_data < NetHeadSize()) {
    return -1;
  }
  NetHead c_head;
  c_head.mark[0] = NET_MARK_0;
  c_head.mark[1] = NET_MARK_1;
  c_head.type_flag = (ORDER_NETWORK == VZ_ORDER_BYTE) ?
                     HostToNetwork16(n_flag) : n_flag;
  c_head.data_size = (ORDER_NETWORK == VZ_ORDER_BYTE) ?
                     HostToNetwork32(n_body) : n_body;
  memcpy(p_data, &c_head, NetHeadSize());

  return NetHeadSize();
}

}  // namespace vzconn
