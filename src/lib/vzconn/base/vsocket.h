/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : IPV4�����ַ,SOCKET��װ
************************************************************************/
#ifndef LIBVZCONN_VSOCKET_H_
#define LIBVZCONN_VSOCKET_H_

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#include <io.h>

struct iovec {
  const void  *iov_base;    /* Starting address */
  size_t iov_len;     /* Number of bytes to transfer */
};
#else  // _LINUX
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#endif

#include <string>

#include "vzbase/base/basictypes.h"
#include "vzbase/base/boost_settings.hpp"

namespace vzconn {

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
  CInetAddr&          operator=(const std::string& addr_str); // ip:port

  void                SetIP(uint32 ip);
  void                SetIP(const char* hostname);
  void                SetPort(uint16 port);

  uint32              GetIP() const;
  uint16              GetPort() const;
  const struct sockaddr_in&  IPAddr() const;
  struct sockaddr_in*        GetAddr();
  const struct sockaddr_in*  GetAddr() const;
  const std::string         ToString() const;
  const std::string         IP2String() const;

  int32 ToIpcAddr(char *p_addr, uint32 n_addr) const;

  bool                operator==(const CInetAddr& addr) const;
  bool                operator!=(const CInetAddr& addr) const;
  bool                operator>(const CInetAddr& addr) const;
  bool                operator<(const CInetAddr& addr) const;

 private:
  struct sockaddr_in         c_sock_addr_;
};

/* TCP���������¼��ӿ� */
class CClientInterface;
class CTcpServerInterface;

class VSocket : public vzbase::noncopyable,
  public boost::enable_shared_from_this<VSocket> {

 public:
   boost::shared_ptr<VSocket> Ptr;

 public:
  static bool IsSocketClosed(SOCKET s);

 public:
  VSocket(CClientInterface *cli_hdl);
  virtual ~VSocket();

  void SetSocketObserver(CClientInterface *cli_hdl) {
    cli_hdl_ptr_ = cli_hdl;
  }

  SOCKET          GetSocket() const;
  virtual void    SetSocket(SOCKET handle);

  virtual void    Close();

  const std::string ip_addr();
  const CInetAddr &remote_addr();
  void            SetRemoteAddr(const CInetAddr &addr);

  bool            isOpen() const;
  bool            isClose();

  //��������
  int32 SetOption(int level, int option, void *optval, int optlen) const;
  int32 GetOption(int level, int option, void *optval, int *optlen) const;

  /************************************************************************
  *Description : ����һ������;���浽����cache��
  *Parameters  : p_data[IN] ����(body��)
  *              n_data[IN] ���ݳ���
  *              e_flag[IN] VZΪ��ͷ��flag[uint16]
  *Return      : >0 �������ݳ���,<=0 ����ʧ��
  ************************************************************************/
  virtual int32 AsyncWrite(const void  *p_data,
                           uint32       n_data,
                           uint16       e_flag);

  /************************************************************************
  *Description : ����һ������;���浽����cache��
  *Parameters  : iov[IN]    ����(body��)
  *              n_iov[IN]  iov����
  *              e_flag[IN] VZΪ��ͷ��flag[uint16]
  *Return      : >0 �������ݳ���,<=0 ����ʧ��
  ***********************************************************************/
  virtual int32 AsyncWrite(struct iovec iov[],
                           uint32       n_iov,
                           uint16       e_flag);

  /***************************************************************************
  *Author        : Sober.Peng 28:12:2016
  *Description   : �������ݽ���
  *Param         : pData[OUT] �������ݣ�nData[IN] �����С
  *Return        : >0 ���ݳ��ȣ�0 û�յ����ݣ�-1 ����
  ***************************************************************************/
  virtual int32 Recv(void *pData, uint32 nData);
  virtual int32 Recv(void *pData, uint32 nData, CInetAddr& cRemoteAddr);

  /***************************************************************************
  *Author        : Sober.Peng 28:12:2016
  *Description   : ���緢�ͽ���
  *Param         : pData[IN] �������ݣ�nData[IN] �����С
  *Return        : >0 ���ݳ��ȣ�0 û�������ݣ�-1 ����
  ****************************************************************************/
  virtual int32 Send(const void* pData, uint32 nData);
  virtual int32 Send(const void*      pData,
                     uint32           nData,
                     const CInetAddr  &cRemoteAddr);

  friend class CSockRecvData;
  friend class CSockSendData;

 protected:
  SOCKET              handler_;
  int32               rw_flag_;

 protected:
  CClientInterface    *cli_hdl_ptr_;  // �ͻ����¼�����
  CInetAddr           remote_addr_;
  std::string         remote_ip_addr_;
};

/* TCP���������¼��ӿ� */
class CTcpServerInterface {
 public:
  virtual bool HandleNewConnection(VSocket *srv, VSocket *new_cli) = 0;
  virtual void HandleServerClose(VSocket *srv) = 0;
};

/* �ͻ��������¼��ӿ� */
class CClientInterface {
 public:
  /***********************************************************************
  *Description : ����ͷ������
  *Parameters  :
  *Return      :
  ***********************************************************************/
  virtual uint32 NetHeadSize();

  /***********************************************************************
  *Description : �ص���������ͷ��
  *Parameters  : p_data[IN] ���ݽ�����ַ;[�˴��ڴ�4�ֽڶ���]
  *              n_data[IN] ���ݳ���
  *              n_flag[OUT] ͷ��FLAG
  *Return      : 0=δ�ҵ���ͷ,>0һ��������(head+body),<0(������)δ�ҵ���ͷ
  ***********************************************************************/
  virtual int32 NetHeadParse(const uint8 *p_data,
                             uint32       n_data,
                             uint16      *n_flag);

  /***********************************************************************
  *Description : �ص�����ͷ�����
  *Parameters  : p_data[OUT] ���ݴ洢��ַ;[�˴������ڴ治����,��ʹ��memcpy��������ͷ]
  *              n_data[IN]  ��ʹ�����ݳ���
  *              n_body[IN]  ��������С
  *              n_flag[IN]  VZ��ͷ��flag
  *Return      : >0��ͷռ�����ݳ���
  ***********************************************************************/
  virtual int32 NetHeadPacket(uint8  *p_data,
                              uint32  n_data,
                              uint32  n_body,
                              uint16  n_flag);
 public:
  virtual int32 HandleRecvPacket(VSocket       *p_cli,
                                 const uint8   *p_data,
                                 uint32         n_data,
                                 uint16         n_flag) = 0;
  virtual int32 HandleSendPacket(VSocket *p_cli) = 0;
  virtual void  HandleClose(VSocket *p_cli) = 0;
};

typedef VSocket VzConnect;

}  // namespace vzconn
#endif  // LIBVZCONN_VSOCKET_H_
