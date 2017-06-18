/************************************************************************/
/* Author      : Sober.Peng 17-06-15
/* Description : 由TCP Server创建的对象用户层不能直接调用delete干掉,调用CloseSocket关闭连接
/************************************************************************/
#ifndef LIBVZCONN_CEVTTCPCLIENT_H_
#define LIBVZCONN_CEVTTCPCLIENT_H_

#include "clibevent.h"
#include "basictypes.h"

#include "vzconn/base/vsocket.h"
#include "vzconn/base/cblockbuffer.h"

class CEvtTcpServer;
class CEvtTcpClient : public VSocket {
 protected:
  CEvtTcpClient(const EVT_LOOP *p_loop, CClientInterface *cli_hdl);
  virtual ~CEvtTcpClient();

 public:
  static CEvtTcpClient* Create(const EVT_LOOP   *p_loop,
                               CClientInterface *cli_hdl);

  // 主动断开链接
  virtual void Remove();

 public:
  // 设置已打开的SCOKET
  virtual int32 Open(SOCKET s, bool b_block=false);

 public:
  // 链接到服务端;无需调用Open
  virtual int32 Connect(const CInetAddr *p_remote_addr,
                        bool             b_block,
                        bool             b_reuse,
                        uint32           n_timeout=5000);

 public:
  /************************************************************************/
  /* Description : 发送一包数据
  /* Parameters  : p_data[IN] 数据(body区)
                   n_data[IN] 数据长度
                   p_param[IN] 参数.(VZ为包头的flag[uint16])
  /* Return      :
  /************************************************************************/
  virtual int32 SendPacket(const void  *p_data,
                           uint32       n_data,
                           const void  *p_param);

 protected:
  friend class CEvtTcpServer;

 protected:
  static int32  EvtRecv(SOCKET        fd,
                        short         events,
                        const void   *p_usr_arg);
  virtual int32 OnRecv();

  static int32  EvtSend(SOCKET        fd,
                        short         events,
                        const void   *p_usr_arg);
  virtual int32 OnSend();

  // 异步链接服务器,暂时不成功,待研究
  static int32  EvtConnect(SOCKET      fd,
                           short       events,
                           const void *p_usr_arg);
  virtual int32 OnConnect(SOCKET fd);

 protected:
  const EVT_LOOP   *p_evt_loop_;    // 随进程退出而销毁,不必关心生命周期

 protected:
  EVT_IO            c_evt_recv_;    // 接收事件
  CBlockBuffer      c_recv_data_;   // 接收buffer

  EVT_IO            c_evt_send_;    // 发送事件
  CBlockBuffer      c_send_data_;   // 发送buffer

 protected:
  CClientInterface  *cli_handle_ptr_;  // 客户端事件处理
};

#endif  // LIBVZCONN_CEVTTCPCLIENT_H_
