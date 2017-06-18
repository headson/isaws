/************************************************************************/
/* Author      : SoberPeng 2017-06-14
/* Description :
/************************************************************************/
#include "vzconn/base/vsocket.h"

#include <string.h>

#include "stdafx.h"

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
  SetIP(addr_str.substr(0, pos));

  return *this;
}

void CInetAddr::SetIP(uint32 ip) {
  c_sock_addr_.sin_addr.s_addr = htonl(ip);
}

void CInetAddr::SetIP(const std::string &hostname) {
  c_sock_addr_.sin_addr.s_addr = inet_addr(hostname.c_str());
  if (c_sock_addr_.sin_addr.s_addr == (uint32)-1) { //��ַΪ0.0.0.0����Ч��ַ
    if (hostent * pHost = gethostbyname(hostname.c_str())) {
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

std::string CInetAddr::ToString() const {
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

std::string CInetAddr::IP2String() const {
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
VSocket::VSocket()
  : handler_(INVALID_SOCKET)
  , rw_flag_(0)
  , net_head_parse_(NULL)
  , net_head_packet_(NULL) {
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
* Description   : �������ݽ���
* Param         : pData[OUT] �������ݣ�nData[IN] �����С
* Return        : >0 ���ݳ��ȣ�0 û�յ����ݣ�-1 ����
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
* Description   : �������ݽ���
* Param         : pData[OUT] �������ݣ�nData[IN] �����С
* Return        : >0 ���ݳ��ȣ�0 û�յ����ݣ�-1 ����
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
* Description   : ���緢�ͽ���
* Param         : pData[IN] �������ݣ�nData[IN] �����С
* Return        : >0 ���ݳ��ȣ�0 û�������ݣ�-1 ����
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

  return -1;
}

/*****************************************************************************
* Author        : Sober.Peng 28:12:2016
* Description   : ���緢�ͽ���
* Param         : pData[IN] �������ݣ�nData[IN] �����С
* Return        : >0 ���ݳ��ȣ�0 û�������ݣ�-1 ����
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

  return -1;
}

NETHEAD_PARSE_CALLBACK VSocket::GetNetHeadParseCallback() {
  return net_head_parse_;
}

void VSocket::SetNetHeadParseCallback(NETHEAD_PARSE_CALLBACK net_head_perse) {
  net_head_parse_ = net_head_perse;
}

NETHEAD_PACKET_CALLBACK VSocket::GetNetHeadPacketCallback() {
  return net_head_packet_;
}

void VSocket::SetNetHeadPacketCallback(NETHEAD_PACKET_CALLBACK net_head_packet) {
  net_head_packet_ = net_head_packet;
}
