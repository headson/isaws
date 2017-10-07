/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : TCP客户端
************************************************************************/
#ifndef LIBVZCONN_CEVTTCPCLIENT_H_
#define LIBVZCONN_CEVTTCPCLIENT_H_

#include "vzbase/base/basictypes.h"
#include "vzbase/base/boost_settings.hpp"

#include "vzconn/base/vsocket.h"
#include "vzconn/base/clibevent.h"
#include "vzconn/buffer/cblockbuffer.h"
#include "vzconn/buffer/perfectbufferpool.h"

namespace vzconn {

class CEvtTcpClient : public VSocket,
  public boost::noncopyable,
  public boost::enable_shared_from_this<CEvtTcpClient> {

 public:
  typedef boost::shared_ptr<CEvtTcpClient> Ptr;

 protected:
  CEvtTcpClient(const EVT_LOOP *evt_loop, CClientInterface *cli_hdl);
  virtual ~CEvtTcpClient();

 public:
  static CEvtTcpClient* Create(const EVT_LOOP *evt_loop,
                               CClientInterface *cli_hdl);

  // 主动断开链接
  virtual void  Remove();

 public:
  // 设置已打开的SCOKET
  virtual bool  Open(SOCKET s, bool is_block=false);

 public:
  /***********************************************************************
  *Description : 发送一包数据;缓存到发送cache中
  *Parameters  : pdata[IN] 数据(body区)
  *              ndata[IN] 数据长度
  *              eflag[IN] VZ为包头的flag[uint16]
  *Return      : >0 缓存数据长度,<=0 发送失败
  ***********************************************************************/
  virtual int32 AsyncWrite(const void *pdata, uint32 ndata, uint16 eflag);

  /***********************************************************************
  *Description : 发送一包数据;缓存到发送cache中
  *Parameters  : aiov[IN]  数据(body区)
  *              niov[IN]  iov个数
  *              eflag[IN] VZ为包头的flag[uint16]
  *Return      : >0 缓存数据长度,<=0 发送失败
  ***********************************************************************/
  virtual int32 AsyncWrite(struct iovec aiov[], uint32 niov, uint16 eflag);

  virtual int32 AsyncWrite(ByteBuffer::Ptr buffer, uint16 eflag);

 protected:
  friend class CEvtTcpServer;

 protected:
  // 接收事件
  static int  EvtRecv(SOCKET fd, short events, const void *usr_arg);
  virtual int OnRecv();

  // 发送事件
  static int  EvtSend(SOCKET fd, short events, const void *usr_arg);
  virtual int OnSend();

 protected:
  const EVT_LOOP   *evt_loop_;    // 随进程退出而销毁,不必关心生命周期

 protected:
  EVT_IO            evt_recv_;    // 接收事件
  ByteBuffer::Ptr   recv_data_;

  char             *head_data_;     // 头
  unsigned int      head_size_;     // 头长度
  uint32            wait_recv_size_;   // 待接收数据大小

 protected:
  EVT_IO            evt_send_;    // 发送事件

  typedef std::list<ByteBuffer::Ptr> SEND_LIST;
  SEND_LIST         send_list_;
  ByteBuffer::Ptr   send_data_;
};

typedef CEvtTcpClient TcpConnect;

}  // namespace vzconn
#endif  // LIBVZCONN_CEVTTCPCLIENT_H_
