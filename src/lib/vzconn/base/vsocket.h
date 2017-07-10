/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : IPV4网络地址,SOCKET封装
************************************************************************/
#ifndef LIBVZCONN_VSOCKET_H_
#define LIBVZCONN_VSOCKET_H_

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#include <io.h>

struct iovec {
  void  *iov_base;    /* Starting address */
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
#endif

#include <string>

#include "vzbase/base/basictypes.h"

namespace vzconn {

class CInetAddr {
 public:
  CInetAddr();
  CInetAddr(const CInetAddr& addr);
  CInetAddr(const struct sockaddr_in& addr);
  CInetAddr(unsigned int ip, unsigned short port);
  CInetAddr(const char *hostname, unsigned short port);

  virtual ~CInetAddr();

  static bool IsIPV4(const char *s);

  bool                IsNull() const;
  CInetAddr&          operator=(const CInetAddr& addr);
  CInetAddr&          operator=(const struct sockaddr_in& addr);
  CInetAddr&          operator=(const std::string& addr_str); // ip:port

  void                SetIP(unsigned int ip);
  void                SetIP(const char* hostname);
  void                SetPort(unsigned short port);

  unsigned int              GetIP() const;
  unsigned short            GetPort() const;
  const struct sockaddr_in& IPAddr() const;
  struct sockaddr_in*       GetAddr();
  const struct sockaddr_in* GetAddr() const;
  const std::string         ToString() const;
  const std::string         IP2String() const;

  int32 ToIpcAddr(char *p_addr, unsigned int n_addr) const;

  bool                operator==(const CInetAddr& addr) const;
  bool                operator!=(const CInetAddr& addr) const;
  bool                operator>(const CInetAddr& addr) const;
  bool                operator<(const CInetAddr& addr) const;

 private:
  struct sockaddr_in         c_sock_addr_;
};

class VSocket;
/* TCP服务网络事件接口 */
class CTcpServerInterface {
 public:
  virtual bool HandleNewConnection(VSocket *p_srv, VSocket *new_sock) = 0;
  virtual void HandleServerClose(VSocket *p_srv) = 0;
};

/* 客户端网络事件接口 */
class CClientInterface {
 public:
  /***********************************************************************
  *Description : 网络头部长度
  *Parameters  :
  *Return      :
  ***********************************************************************/
  virtual unsigned int NetHeadSize();

  /***********************************************************************
  *Description : 回调解析网络头部
  *Parameters  : p_data[IN] 数据
  *              n_data[IN] 数据长度
  *Return      : 0=未找到包头,>0一整包长度(head+body),<0(脏数据)未找到包头
  ***********************************************************************/
  virtual int32 NetHeadParse(const char     *p_data,
                             unsigned int    n_data,
                             unsigned short *n_flag);
  /***********************************************************************
  *Description : 回调网络头部打包
  *Parameters  : p_data[OUT] 数据
  *              n_data[IN]  可使用数据长度
  *              n_body[IN]  数据区大小
  *              n_flag[IN]  VZ包头的flag
  *Return      : >0包头占用数据长度
  ***********************************************************************/
  virtual int32 NetHeadPacket(char          *p_data,
                              unsigned int   n_data,
                              unsigned int   n_body,
                              unsigned short n_flag);
 public:
  virtual int32 HandleRecvPacket(VSocket       *p_cli,
                                 const char    *p_data,
                                 unsigned int   n_data,
                                 unsigned short n_flag) = 0;
  virtual int32 HandleSendPacket(VSocket *p_cli) = 0;
  virtual void  HandleClose(VSocket *p_cli) = 0;
};

class VSocket {
 public:
  VSocket(CClientInterface *cli_hdl);
  virtual ~VSocket();

  SOCKET          GetSocket() const;
  virtual void    SetSocket(SOCKET handle);

  virtual void    Close();

  bool            isOpen() const;
  bool            isClose();

  //属性设置
  int32 SetOption(int level, int option, void *optval, int optlen) const;
  int32 GetOption(int level, int option, void *optval, int *optlen) const;

  /************************************************************************
  *Description : 发送一包数据;缓存到发送cache中
  *Parameters  : p_data[IN] 数据(body区)
  *              n_data[IN] 数据长度
  *              e_flag[IN] VZ为包头的flag[unsigned short]
  *Return      : >0 缓存数据长度,<=0 发送失败
  ************************************************************************/
  virtual int32 AsyncWrite(const void       *p_data,
                           unsigned int      n_data,
                           unsigned short    e_flag);

  /************************************************************************
  *Description : 发送一包数据;缓存到发送cache中
  *Parameters  : iov[IN]    数据(body区)
  *              n_iov[IN]  iov个数
  *              e_flag[IN] VZ为包头的flag[unsigned short]
  *Return      : >0 缓存数据长度,<=0 发送失败
  ***********************************************************************/
  virtual int32 AsyncWrite(struct iovec     a_iov[],
                           unsigned int     n_iov,
                           unsigned short   e_flag);

  /***************************************************************************
  *Author        : Sober.Peng 28:12:2016
  *Description   : 网络数据接收
  *Param         : pData[OUT] 接收数据，nData[IN] 缓存大小
  *Return        : >0 数据长度，0 没收到数据，-1 断网
  ***************************************************************************/
  virtual int32 Recv(void *pData, unsigned int nData);
  virtual int32 Recv(void *pData, unsigned int nData, CInetAddr& cRemoteAddr);

  /***************************************************************************
  *Author        : Sober.Peng 28:12:2016
  *Description   : 网络发送接收
  *Param         : pData[IN] 发送数据，nData[IN] 缓存大小
  *Return        : >0 数据长度，0 没发送数据，-1 断网
  ****************************************************************************/
  virtual int32 Send(const void* pData, unsigned int nData);
  virtual int32 Send(const void*      pData,
                     unsigned int           nData,
                     const CInetAddr  &cRemoteAddr);

  friend class CSockRecvData;
  friend class CSockSendData;

 protected:
  SOCKET              handler_;
  int32               rw_flag_;

 protected:
  CClientInterface    *cli_hdl_ptr_;  // 客户端事件处理
};

}  // namespace vzconn
#endif  // LIBVZCONN_VSOCKET_H_
