/************************************************************************/
/* Author      : SoberPeng 2017-06-14
/* Description :
/************************************************************************/
#include "vsocket.h"
#include "base/stdafx.h"

CInetAddr::CInetAddr() {
  memset(&c_addr_, 0, sizeof(struct sockaddr_in));
  c_addr_.sin_family = AF_INET;
  c_addr_.sin_addr.s_addr = htonl(INADDR_ANY);
  c_addr_.sin_port = htons(0);
}

CInetAddr::CInetAddr(const sockaddr_in& addr) {
  memcpy(&c_addr_, &addr, sizeof(struct sockaddr_in));
}

CInetAddr::CInetAddr(uint32_t ip, uint16_t port) {
  memset(&c_addr_, 0, sizeof(struct sockaddr_in));

  SetIP(ip);
  SetPort(port);

  c_addr_.sin_family = AF_INET;
}

CInetAddr::CInetAddr(const CInetAddr &addr) {
  c_addr_ = addr.c_addr_;
}

CInetAddr::CInetAddr(const std::string& hostname, uint16_t port) {
  memset(&c_addr_, 0, sizeof(struct sockaddr_in));

  SetIP(hostname);
  SetPort(port);

  c_addr_.sin_family = AF_INET;
}

CInetAddr::~CInetAddr() {
}

bool CInetAddr::IsIPV4(const char *s) {
  bool rv = true;
  int32_t tmp1, tmp2, tmp3, tmp4, i;

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
  return (ntohs(c_addr_.sin_port) == 0);
}

CInetAddr& CInetAddr::operator = (const CInetAddr& addr) {
  this->c_addr_ = addr.c_addr_;
  return *this;
}

CInetAddr& CInetAddr::operator =(const sockaddr_in &addr) {
  this->c_addr_ = addr;
  return *this;
}

CInetAddr& CInetAddr::operator =(const std::string &addr_str) {
  std::string::size_type pos = addr_str.find(':');
  if(std::string::npos == pos) {
    return *this;
  }

  SetPort(static_cast<uint16_t>(strtoul(addr_str.substr(pos + 1).c_str(), NULL, 10)));
  SetIP(addr_str.substr(0, pos));

  return *this;
}

void CInetAddr::SetIP(uint32_t ip) {
  c_addr_.sin_addr.s_addr = htonl(ip);
}

void CInetAddr::SetIP(const std::string &hostname) {
  c_addr_.sin_addr.s_addr = inet_addr(hostname.c_str());
  if (c_addr_.sin_addr.s_addr == (uint32_t)-1) { //地址为0.0.0.0，无效地址
    if (hostent * pHost = gethostbyname(hostname.c_str())) {
      c_addr_.sin_addr.s_addr = (*reinterpret_cast<uint32_t *>(pHost->h_addr_list[0]));
    }
  }
}

void CInetAddr::SetPort(uint16_t port) {
  c_addr_.sin_port = htons(port);
}

uint32_t CInetAddr::GetIP() const {
  return ntohl(c_addr_.sin_addr.s_addr);
}

uint16_t CInetAddr::GetPort() const {
  return ntohs(c_addr_.sin_port);
}

const sockaddr_in& CInetAddr::IPAddr() const {
  return c_addr_;
}

sockaddr_in* CInetAddr::GetAddr() {
  return &c_addr_;
}

const sockaddr_in* CInetAddr::GetAddr() const {
  return &c_addr_;
}

std::string CInetAddr::ToString() const {
  char ip_str[32] = {0};
#ifdef WIN32
  sprintf(ip_str, "%d.%d.%d.%d", c_addr_.sin_addr.S_un.S_un_b.s_b1,
          c_addr_.sin_addr.S_un.S_un_b.s_b2,
          c_addr_.sin_addr.S_un.S_un_b.s_b3,
          c_addr_.sin_addr.S_un.S_un_b.s_b4);
#else
  ::inet_ntop(c_addr_.sin_family, &(c_addr_.sin_addr), ip_str, sizeof(ip_str));
#endif

  sprintf(ip_str, "%s:%d", ip_str, GetPort());
  return ip_str;
}

std::string CInetAddr::IP2String() const {
  char ip_str[32] = {0};

#ifdef WIN32
  sprintf(ip_str, "%d.%d.%d.%d", c_addr_.sin_addr.S_un.S_un_b.s_b1,
          c_addr_.sin_addr.S_un.S_un_b.s_b2,
          c_addr_.sin_addr.S_un.S_un_b.s_b3,
          c_addr_.sin_addr.S_un.S_un_b.s_b4);
#else
  ::inet_ntop(c_addr_.sin_family, &(c_addr_.sin_addr), ip_str, sizeof(ip_str));
#endif

  return ip_str;
}

bool CInetAddr::operator ==(const CInetAddr& addr) const {
  return (ntohl(c_addr_.sin_addr.s_addr) == ntohl(addr.c_addr_.sin_addr.s_addr)
          && ntohs(c_addr_.sin_port) == ntohs(addr.c_addr_.sin_port));
}

bool CInetAddr::operator !=(const CInetAddr &addr) const {
  return !((*this) == addr);
}

bool CInetAddr::operator <(const CInetAddr &addr) const {
  if(ntohl(c_addr_.sin_addr.s_addr) < ntohl(addr.c_addr_.sin_addr.s_addr))
    return true;
  else if(ntohl(c_addr_.sin_addr.s_addr) > ntohl(addr.c_addr_.sin_addr.s_addr))
    return false;
  else {
    return (ntohs(c_addr_.sin_port) < ntohs(addr.c_addr_.sin_port) ? true : false);
  }
}

bool CInetAddr::operator >(const CInetAddr& addr) const {
  if(ntohl(c_addr_.sin_addr.s_addr) > ntohl(addr.c_addr_.sin_addr.s_addr))
    return true;
  else if(ntohl(c_addr_.sin_addr.s_addr) < ntohl(addr.c_addr_.sin_addr.s_addr))
    return false;
  else {
    return (ntohs(c_addr_.sin_port) > ntohs(addr.c_addr_.sin_port) ? true : false);
  }
}

std::ostream& operator<<(std::ostream& os, const CInetAddr& addr) {
  os << addr.ToString();
  return os;
}

//////////////////////////////////////////////////////////////////////////
VSocket::VSocket()
  : handler_(INVALID_SOCKET)
  , rw_flag_(0) {
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

int32_t VSocket::SetOption(int level, int option, void *optval, int optlen) const {
  return setsockopt(GetSocket(), level, option, (char *)optval, optlen);
}

int32_t VSocket::GetOption(int level, int option, void *optval, int *optlen) const {
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
int32_t VSocket::Recv(void *pData, uint32_t nData) {
  int32_t nRet = -1;
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
int32_t VSocket::Recv(void* pData, uint32_t nData, CInetAddr& cRemoteAddr) {
  int32_t nRet = -1;
  if(!IsOpen()) {
    return nRet;
  }

#ifdef WIN32
  int32_t from_size = sizeof(struct sockaddr_in);
#else
  uint32_t from_size = sizeof(struct sockaddr_in);
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
int32_t VSocket::Send(const void *buf, uint32_t buf_size) {
  int32_t nRet = -1;
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

  return -1;
}

/*****************************************************************************
* Author        : Sober.Peng 28:12:2016
* Description   : 网络发送接收
* Param         : pData[IN] 发送数据，nData[IN] 缓存大小
* Return        : >0 数据长度，0 没发送数据，-1 断网
******************************************************************************/
int32_t VSocket::Send(const void* buf, uint32_t buf_size, const CInetAddr& remote_addr) {
  int32_t nRet = -1;
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

  return -1;
}

