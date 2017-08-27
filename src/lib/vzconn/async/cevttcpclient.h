/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : TCP客户端
************************************************************************/
#ifndef LIBVZCONN_CEVTTCPCLIENT_H_
#define LIBVZCONN_CEVTTCPCLIENT_H_

#include "vzbase/base/basictypes.h"

#include "vzconn/base/vsocket.h"
#include "vzconn/base/clibevent.h"
#include "vzconn/buffer/cblockbuffer.h"
#include "vzbase/base/boost_settings.hpp"

namespace vzconn {

class CEvtTcpClient : public VSocket,
  public vzbase::noncopyable,
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
  // 链接到服务端;无需调用Open
  virtual bool  Connect(const CInetAddr *remote_addr,
                        bool             is_block,
                        bool             is_reuse,
                        uint32           ms_timeout=5000);

 public:
  /***********************************************************************
  *Description : 发送一包数据;缓存到发送cache中
  *Parameters  : pdata[IN] 数据(body区)
  *              ndata[IN] 数据长度
  *              eflag[IN] VZ为包头的flag[uint16]
  *Return      : >0 缓存数据长度,<=0 发送失败
  ***********************************************************************/
  virtual int32 AsyncWrite(const void *pdata,
                           uint32      ndata,
                           uint16      eflag);

  /***********************************************************************
  *Description : 发送一包数据;缓存到发送cache中
  *Parameters  : aiov[IN]  数据(body区)
  *              niov[IN]  iov个数
  *              eflag[IN] VZ为包头的flag[uint16]
  *Return      : >0 缓存数据长度,<=0 发送失败
  ***********************************************************************/
  virtual int32 AsyncWrite(struct iovec aiov[],
                           uint32       niov,
                           uint16       eflag);

 protected:
  friend class CEvtTcpServer;

 protected:
  // 接收事件
  static int32  EvtRecv(SOCKET        fd,
                        short         events,
                        const void   *usr_arg);
  virtual int32 OnRecv();

  // 发送事件
  static int32  EvtSend(SOCKET        fd,
                        short         events,
                        const void   *usr_arg);
  virtual int32 OnSend();

  // 异步链接服务器,暂时不成功,待研究
  static int32  EvtConnect(SOCKET      fd,
                           short       events,
                           const void *usr_arg);
  virtual int32 OnConnect(SOCKET fd);

 public:
  EVT_IO& GetEvtRecv() {
    return c_evt_recv_;
  }
  EVT_IO& GetEvtSend() {
    return c_evt_send_;
  }
  CBlockBuffer& GetRecvData() {
    return c_recv_data_;
  }
  CBlockBuffer& GetSendData() {
    return c_send_data_;
  }

 protected:
  const EVT_LOOP   *p_evt_loop_;    // 随进程退出而销毁,不必关心生命周期

 protected:
  EVT_IO            c_evt_recv_;    // 接收事件
  CBlockBuffer      c_recv_data_;   // 接收buffer

  EVT_IO            c_evt_send_;    // 发送事件
  CBlockBuffer      c_send_data_;   // 发送buffer
};

typedef CEvtTcpClient TcpConnect;

}  // namespace vzconn
#endif  // LIBVZCONN_CEVTTCPCLIENT_H_
