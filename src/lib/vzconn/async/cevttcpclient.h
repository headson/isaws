/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : TCP�ͻ���
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

  // �����Ͽ�����
  virtual void  Remove();

 public:
  // �����Ѵ򿪵�SCOKET
  virtual bool  Open(SOCKET s, bool is_block=false);

 public:
  // ���ӵ������;�������Open
  virtual bool  Connect(const CInetAddr *remote_addr,
                        bool             is_block,
                        bool             is_reuse,
                        uint32           ms_timeout=5000);

 public:
  /***********************************************************************
  *Description : ����һ������;���浽����cache��
  *Parameters  : pdata[IN] ����(body��)
  *              ndata[IN] ���ݳ���
  *              eflag[IN] VZΪ��ͷ��flag[uint16]
  *Return      : >0 �������ݳ���,<=0 ����ʧ��
  ***********************************************************************/
  virtual int32 AsyncWrite(const void *pdata,
                           uint32      ndata,
                           uint16      eflag);

  /***********************************************************************
  *Description : ����һ������;���浽����cache��
  *Parameters  : aiov[IN]  ����(body��)
  *              niov[IN]  iov����
  *              eflag[IN] VZΪ��ͷ��flag[uint16]
  *Return      : >0 �������ݳ���,<=0 ����ʧ��
  ***********************************************************************/
  virtual int32 AsyncWrite(struct iovec aiov[],
                           uint32       niov,
                           uint16       eflag);

 protected:
  friend class CEvtTcpServer;

 protected:
  // �����¼�
  static int32  EvtRecv(SOCKET        fd,
                        short         events,
                        const void   *usr_arg);
  virtual int32 OnRecv();

  // �����¼�
  static int32  EvtSend(SOCKET        fd,
                        short         events,
                        const void   *usr_arg);
  virtual int32 OnSend();

  // �첽���ӷ�����,��ʱ���ɹ�,���о�
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
  const EVT_LOOP   *p_evt_loop_;    // ������˳�������,���ع�����������

 protected:
  EVT_IO            c_evt_recv_;    // �����¼�
  CBlockBuffer      c_recv_data_;   // ����buffer

  EVT_IO            c_evt_send_;    // �����¼�
  CBlockBuffer      c_send_data_;   // ����buffer
};

typedef CEvtTcpClient TcpConnect;

}  // namespace vzconn
#endif  // LIBVZCONN_CEVTTCPCLIENT_H_
