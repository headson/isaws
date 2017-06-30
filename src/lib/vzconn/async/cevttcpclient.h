/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : TCP客户端
************************************************************************/
#ifndef LIBVZCONN_CEVTTCPCLIENT_H_
#define LIBVZCONN_CEVTTCPCLIENT_H_

#include "vzbase/base/basictypes.h"

#include "vzconn/base/vsocket.h"
#include "vzconn/base/clibevent.h"
#include "vzconn/buffer/csocketbuffer.h"

namespace vzconn {

class CEvtTcpClient : public VSocket {
 protected:
  CEvtTcpClient(const EVT_LOOP *p_loop, CClientInterface *cli_hdl);
  virtual ~CEvtTcpClient();

 public:
  static CEvtTcpClient* Create(const EVT_LOOP   *p_loop,
                               CClientInterface *cli_hdl);

  // 主动断开链接
  virtual void  Remove();

 public:
  // 设置已打开的SCOKET
  virtual bool  Open(SOCKET s, bool b_block=false);

 public:
  // 链接到服务端;无需调用Open
  virtual bool  Connect(const CInetAddr *p_remote_addr,
                        bool             b_block,
                        bool             b_reuse,
                        uint32           n_timeout=5000);

 public:
  /***********************************************************************
  *Description : 发送一包数据;缓存到发送cache中
  *Parameters  : p_data[IN] 数据(body区)
  *              n_data[IN] 数据长度
  *              e_flag[IN] VZ为包头的flag[uint16]
  *Return      : >0 缓存数据长度,<=0 发送失败
  ***********************************************************************/
  virtual int32 AsyncWrite(const void  *p_data,
                           uint32       n_data,
                           uint16       e_flag);

  /***********************************************************************
  *Description : 发送一包数据;缓存到发送cache中
  *Parameters  : iov[IN]    数据(body区)
  *              n_iov[IN]  iov个数
  *              e_flag[IN] VZ为包头的flag[uint16]
  *Return      : >0 缓存数据长度,<=0 发送失败
  ***********************************************************************/
  virtual int32 AsyncWrite(struct iovec iov[],
                           uint32       n_iov,
                           uint16       e_flag);

 protected:
  friend class CEvtTcpServer;

 protected:
  // 接收事件
  static int32  EvtRecv(SOCKET        fd,
                        short         events,
                        const void   *p_usr_arg);
  virtual int32 OnRecv();

  // 发送事件
  static int32  EvtSend(SOCKET        fd,
                        short         events,
                        const void   *p_usr_arg);
  virtual int32 OnSend();

  // 异步链接服务器,暂时不成功,待研究
  static int32  EvtConnect(SOCKET      fd,
                           short       events,
                           const void *p_usr_arg);
  virtual int32 OnConnect(SOCKET fd);

 public:
  EVT_IO& GetEvtRecv() {
    return c_evt_recv_;
  }
  EVT_IO& GetEvtSend() {
    return c_evt_send_;
  }
  CSockRecvData& GetRecvData() {
    return c_recv_data_;
  }
  CSockSendData& GetSendData() {
    return c_send_data_;
  }

 protected:
  const EVT_LOOP   *p_evt_loop_;    // 随进程退出而销毁,不必关心生命周期

 protected:
  EVT_IO            c_evt_recv_;    // 接收事件
  CSockRecvData     c_recv_data_;   // 接收buffer

  EVT_IO            c_evt_send_;    // 发送事件
  CSockSendData     c_send_data_;   // 发送buffer
};

}  // namespace vzconn
#endif  // LIBVZCONN_CEVTTCPCLIENT_H_
