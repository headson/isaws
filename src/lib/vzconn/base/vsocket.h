/************************************************************************/
/* Author      : SoberPeng 2017-06-13
/* Description : IPV4�����ַ,SOCKET��װ
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
/* Description : �ص���������ͷ��
/* Parameters  : p_data[IN] ����
                 n_data[IN] ���ݳ���
                 n_offset[OUT] ����ͷ��ƫ��
/* Return      : 0=δ�ҵ���ͷ,>0һ��������(head+body),<0(������)δ�ҵ���ͷ
/************************************************************************/
typedef int32 (*NETHEAD_PARSE_CALLBACK)(const void *p_data,
                                        uint32      n_data,
                                        uint32     *n_offset);
/************************************************************************/
/* Description : �ص�����ͷ�����
/* Parameters  : p_data[OUT] ����
                 n_data[IN]  ��ʹ�����ݳ���
                 n_body[IN]  ��������С
                 p_param[IN] WritePacketʱ�Ĳ���
/* Return      : >0��ͷռ�����ݳ���
/************************************************************************/
typedef int32 (*NETHEAD_PACKET_CALLBACK)(void       *p_data, 
                                         uint32      n_data, 
                                         uint32      n_body,
                                         const void *p_param);

class VSocket;
/* TCP���������¼��ӿ� */
class CTcpServerInterface {
public:
  virtual int32 HandleNewConnection(void *p_srv, VSocket *new_sock) = 0;
  virtual void  HandleClose(void *p_srv) = 0;
};

/* �ͻ��������¼��ӿ� */
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

  //��������
  int32         SetOption(int level, int option, void *optval, int optlen) const;
  int32         GetOption(int level, int option, void *optval, int *optlen) const;

  /*****************************************************************************
  * Author        : Sober.Peng 28:12:2016
  * Description   : �������ݽ���
  * Param         : pData[OUT] �������ݣ�nData[IN] �����С
  * Return        : >0 ���ݳ��ȣ�0 û�յ����ݣ�-1 ����
  ******************************************************************************/
  virtual int32 Recv(void *pData, uint32 nData);
  virtual int32 Recv(void *pData, uint32 nData, CInetAddr& cRemoteAddr);

  /*****************************************************************************
  * Author        : Sober.Peng 28:12:2016
  * Description   : ���緢�ͽ���
  * Param         : pData[IN] �������ݣ�nData[IN] �����С
  * Return        : >0 ���ݳ��ȣ�0 û�������ݣ�-1 ����
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
  NETHEAD_PARSE_CALLBACK  net_head_parse_;    // �ص���������ͷ��
  NETHEAD_PACKET_CALLBACK net_head_packet_;   // �ص�����ͷ�����
};

#endif  // VSOCKET_H
