/************************************************************************/
/* Author      : SoberPeng 2017-06-13
/* Description : IPV4网络地址,SOCKET封装
/************************************************************************/
#ifndef VSOCKET_H
#define VSOCKET_H

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#else  // _LINUX
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#endif

#include <string>

#include "basictypes.h"
#include "boost/shared_ptr.hpp"

class CInetAddr {
 public:
  CInetAddr();
  CInetAddr(const CInetAddr& addr);
  CInetAddr(const struct sockaddr_in& addr);
  CInetAddr(uint32 ip, uint16 port);
  CInetAddr(const char *hostname, uint16 port);

  virtual ~CInetAddr();

  static bool IsIPV4(const char *s);

  bool                IsNull() const;
  CInetAddr&          operator=(const CInetAddr& addr);
  CInetAddr&          operator=(const struct sockaddr_in& addr);
  CInetAddr&          operator=(const std::string& addr_str);

  void                SetIP(uint32 ip);
  void                SetIP(const std::string& hostname);
  void                SetPort(uint16 port);

  uint32              GetIP() const;
  uint16              GetPort() const;
  const struct sockaddr_in&  IPAddr() const;
  struct sockaddr_in*        GetAddr();
  const struct sockaddr_in*  GetAddr() const;
  std::string         ToString() const;
  std::string         IP2String() const;

  bool                operator==(const CInetAddr& addr) const;
  bool                operator!=(const CInetAddr& addr) const;
  bool                operator>(const CInetAddr& addr) const;
  bool                operator<(const CInetAddr& addr) const;

 private:
  struct sockaddr_in         c_sock_addr_;
};

/************************************************************************/
/* Description : 回调解析网络头部
/* Parameters  : p_data[IN] 数据
                 n_data[IN] 数据长度
                 n_offset[OUT] 数据头部偏移
/* Return      : 0=未找到包头,>0一整包长度(head+body),<0(脏数据)未找到包头
/************************************************************************/
typedef int32 (*NETHEAD_PARSE_CALLBACK)(const void *p_data,
                                        uint32      n_data,
                                        uint32     *n_offset);
/************************************************************************/
/* Description : 回调网络头部打包
/* Parameters  : p_data[OUT] 数据
                 n_data[IN]  可使用数据长度
                 n_body[IN]  数据区大小
                 p_param[IN] WritePacket时的参数
/* Return      : >0包头占用数据长度
/************************************************************************/
typedef int32 (*NETHEAD_PACKET_CALLBACK)(void       *p_data, 
                                         uint32      n_data, 
                                         uint32      n_body,
                                         const void *p_param);

class VSocket;
/* TCP服务网络事件接口 */
class CTcpServerInterface {
public:
  virtual int32 HandleNewConnection(void *p_srv, VSocket *new_sock) = 0;
  virtual void  HandleClose(void *p_srv) = 0;
};

/* 客户端网络事件接口 */
class CClientInterface {
 public:
  virtual int32 HandleRecvPacket(void       *p_cli, 
                                 const void *p_data, 
                                 uint32     n_data) = 0;
  virtual int32 HandleSendPacket(void *p_cli) = 0;
  virtual void  HandleClose(void *p_cli) = 0;
};

class VSocket {
 public:
  VSocket();
  virtual ~VSocket();

  SOCKET          GetSocket() const;
  virtual void    SetSocket(SOCKET handle);

  virtual void    Close();

  bool            IsOpen() const;

  //属性设置
  int32         SetOption(int level, int option, void *optval, int optlen) const;
  int32         GetOption(int level, int option, void *optval, int *optlen) const;

  /*****************************************************************************
  * Author        : Sober.Peng 28:12:2016
  * Description   : 网络数据接收
  * Param         : pData[OUT] 接收数据，nData[IN] 缓存大小
  * Return        : >0 数据长度，0 没收到数据，-1 断网
  ******************************************************************************/
  virtual int32 Recv(void *pData, uint32 nData);
  virtual int32 Recv(void *pData, uint32 nData, CInetAddr& cRemoteAddr);

  /*****************************************************************************
  * Author        : Sober.Peng 28:12:2016
  * Description   : 网络发送接收
  * Param         : pData[IN] 发送数据，nData[IN] 缓存大小
  * Return        : >0 数据长度，0 没发送数据，-1 断网
  ******************************************************************************/
  virtual int32 Send(const void* pData, uint32 nData);
  virtual int32 Send(const void*      pData,
                       uint32           nData,
                       const CInetAddr  &cRemoteAddr);

 protected:
  SOCKET              handler_;
  int32             rw_flag_;

 public:
  NETHEAD_PARSE_CALLBACK GetNetHeadParseCallback();
  void SetNetHeadParseCallback(NETHEAD_PARSE_CALLBACK net_head_perse);

  NETHEAD_PACKET_CALLBACK GetNetHeadPacketCallback();
  void SetNetHeadPacketCallback(NETHEAD_PACKET_CALLBACK net_head_packet);

 protected:
  NETHEAD_PARSE_CALLBACK  net_head_parse_;    // 回调解析网络头部
  NETHEAD_PACKET_CALLBACK net_head_packet_;   // 回调网络头部打包
};

#endif  // VSOCKET_H
