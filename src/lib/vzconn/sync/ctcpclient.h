/************************************************************************/
/* Author      : Sober.Peng 17-06-20
/* Description :
/************************************************************************/
#ifndef LIBVZCONN_CTCPCLIENT_H_
#define LIBVZCONN_CTCPCLIENT_H_

#include "basictypes.h"

#include "vzconn/async/clibevent.h"

#include "vzconn/base/vsocket.h"
#include "vzconn/base/csocketbuffer.h"

namespace vzconn {

class CTcpClient : public VSocket {
 protected:
  CTcpClient(const EVT_LOOP *p_loop, CClientInterface *cli_hdl);

 public:
  static CTcpClient* Create(const EVT_LOOP   *p_loop,
                            CClientInterface *cli_hdl);
  virtual ~CTcpClient();

 public:
  // 设置已打开的SCOKET
  virtual bool  Open(SOCKET s, bool b_block=false);
  void          ResetEvent();

 public:
  // 链接到服务端;无需调用Open
  virtual bool  Connect(const CInetAddr *p_remote_addr,
                        bool             b_block,
                        bool             b_reuse,
                        uint32           n_timeout=5000);

 public:
  /************************************************************************/
  /* Description : 发送一包数据;缓存到发送cache中
  /* Parameters  : p_data[IN] 数据(body区)
                   n_data[IN] 数据长度
                   e_flag[IN] VZ为包头的flag[uint16]
  /* Return      : >0 缓存数据长度,<=0 发送失败
  /************************************************************************/
  virtual int32 AsyncWrite(const void  *p_data,
                           uint32       n_data,
                           uint16       e_flag);

  /************************************************************************/
  /* Description : 发送一包数据;缓存到发送cache中
  /* Parameters  : iov[IN]    数据(body区)
                   n_iov[IN]  iov个数
                   e_flag[IN] VZ为包头的flag[uint16]
  /* Return      : >0 缓存数据长度,<=0 发送失败
  /************************************************************************/
  virtual int32 AsyncWrite(struct iovec iov[],
                           uint32       n_iov,
                           uint16       e_flag);

  // 是否发生完成
  bool          isSendAllBuffer();
  // 是否接收到一包
  bool          isRecvOnePacket();

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

 protected:
  const EVT_LOOP   *p_evt_loop_;    // 随进程退出而销毁,不必关心生命周期

 protected:
  EVT_IO            c_evt_recv_;    // 接收事件
  CSockRecvData     c_recv_data_;   // 接收buffer
  uint32            b_send_all_buffer_;

  EVT_IO            c_evt_send_;    // 发送事件
  CSockSendData     c_send_data_;   // 发送buffer
  uint32            b_recv_one_packet_;
};

}  // namespace vzconn

#endif  // LIBVZCONN_CTCPCLIENT_H_
