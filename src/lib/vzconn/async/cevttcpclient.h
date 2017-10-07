/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : TCP�ͻ���
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

  // �����Ͽ�����
  virtual void  Remove();

 public:
  // �����Ѵ򿪵�SCOKET
  virtual bool  Open(SOCKET s, bool is_block=false);

 public:
  /***********************************************************************
  *Description : ����һ������;���浽����cache��
  *Parameters  : pdata[IN] ����(body��)
  *              ndata[IN] ���ݳ���
  *              eflag[IN] VZΪ��ͷ��flag[uint16]
  *Return      : >0 �������ݳ���,<=0 ����ʧ��
  ***********************************************************************/
  virtual int32 AsyncWrite(const void *pdata, uint32 ndata, uint16 eflag);

  /***********************************************************************
  *Description : ����һ������;���浽����cache��
  *Parameters  : aiov[IN]  ����(body��)
  *              niov[IN]  iov����
  *              eflag[IN] VZΪ��ͷ��flag[uint16]
  *Return      : >0 �������ݳ���,<=0 ����ʧ��
  ***********************************************************************/
  virtual int32 AsyncWrite(struct iovec aiov[], uint32 niov, uint16 eflag);

  virtual int32 AsyncWrite(ByteBuffer::Ptr buffer, uint16 eflag);

 protected:
  friend class CEvtTcpServer;

 protected:
  // �����¼�
  static int  EvtRecv(SOCKET fd, short events, const void *usr_arg);
  virtual int OnRecv();

  // �����¼�
  static int  EvtSend(SOCKET fd, short events, const void *usr_arg);
  virtual int OnSend();

 protected:
  const EVT_LOOP   *evt_loop_;    // ������˳�������,���ع�����������

 protected:
  EVT_IO            evt_recv_;    // �����¼�
  ByteBuffer::Ptr   recv_data_;

  char             *head_data_;     // ͷ
  unsigned int      head_size_;     // ͷ����
  uint32            wait_recv_size_;   // ���������ݴ�С

 protected:
  EVT_IO            evt_send_;    // �����¼�

  typedef std::list<ByteBuffer::Ptr> SEND_LIST;
  SEND_LIST         send_list_;
  ByteBuffer::Ptr   send_data_;
};

typedef CEvtTcpClient TcpConnect;

}  // namespace vzconn
#endif  // LIBVZCONN_CEVTTCPCLIENT_H_
