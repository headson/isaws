/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : TCP�ͻ���
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

  // �����Ͽ�����
  virtual void  Remove();

 public:
  // �����Ѵ򿪵�SCOKET
  virtual bool  Open(SOCKET s, bool b_block=false);

 public:
  // ���ӵ������;�������Open
  virtual bool  Connect(const CInetAddr *p_remote_addr,
                        bool             b_block,
                        bool             b_reuse,
                        uint32           n_timeout=5000);

 public:
  /***********************************************************************
  *Description : ����һ������;���浽����cache��
  *Parameters  : p_data[IN] ����(body��)
  *              n_data[IN] ���ݳ���
  *              e_flag[IN] VZΪ��ͷ��flag[uint16]
  *Return      : >0 �������ݳ���,<=0 ����ʧ��
  ***********************************************************************/
  virtual int32 AsyncWrite(const void  *p_data,
                           uint32       n_data,
                           uint16       e_flag);

  /***********************************************************************
  *Description : ����һ������;���浽����cache��
  *Parameters  : iov[IN]    ����(body��)
  *              n_iov[IN]  iov����
  *              e_flag[IN] VZΪ��ͷ��flag[uint16]
  *Return      : >0 �������ݳ���,<=0 ����ʧ��
  ***********************************************************************/
  virtual int32 AsyncWrite(struct iovec iov[],
                           uint32       n_iov,
                           uint16       e_flag);

 protected:
  friend class CEvtTcpServer;

 protected:
  // �����¼�
  static int32  EvtRecv(SOCKET        fd,
                        short         events,
                        const void   *p_usr_arg);
  virtual int32 OnRecv();

  // �����¼�
  static int32  EvtSend(SOCKET        fd,
                        short         events,
                        const void   *p_usr_arg);
  virtual int32 OnSend();

  // �첽���ӷ�����,��ʱ���ɹ�,���о�
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
  const EVT_LOOP   *p_evt_loop_;    // ������˳�������,���ع�����������

 protected:
  EVT_IO            c_evt_recv_;    // �����¼�
  CSockRecvData     c_recv_data_;   // ����buffer

  EVT_IO            c_evt_send_;    // �����¼�
  CSockSendData     c_send_data_;   // ����buffer
};

}  // namespace vzconn
#endif  // LIBVZCONN_CEVTTCPCLIENT_H_
