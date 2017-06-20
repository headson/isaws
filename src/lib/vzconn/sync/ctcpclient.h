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
  // �����Ѵ򿪵�SCOKET
  virtual bool  Open(SOCKET s, bool b_block=false);
  void          ResetEvent();

 public:
  // ���ӵ������;�������Open
  virtual bool  Connect(const CInetAddr *p_remote_addr,
                        bool             b_block,
                        bool             b_reuse,
                        uint32           n_timeout=5000);

 public:
  /************************************************************************/
  /* Description : ����һ������;���浽����cache��
  /* Parameters  : p_data[IN] ����(body��)
                   n_data[IN] ���ݳ���
                   e_flag[IN] VZΪ��ͷ��flag[uint16]
  /* Return      : >0 �������ݳ���,<=0 ����ʧ��
  /************************************************************************/
  virtual int32 AsyncWrite(const void  *p_data,
                           uint32       n_data,
                           uint16       e_flag);

  /************************************************************************/
  /* Description : ����һ������;���浽����cache��
  /* Parameters  : iov[IN]    ����(body��)
                   n_iov[IN]  iov����
                   e_flag[IN] VZΪ��ͷ��flag[uint16]
  /* Return      : >0 �������ݳ���,<=0 ����ʧ��
  /************************************************************************/
  virtual int32 AsyncWrite(struct iovec iov[],
                           uint32       n_iov,
                           uint16       e_flag);

  // �Ƿ������
  bool          isSendAllBuffer();
  // �Ƿ���յ�һ��
  bool          isRecvOnePacket();

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

 protected:
  const EVT_LOOP   *p_evt_loop_;    // ������˳�������,���ع�����������

 protected:
  EVT_IO            c_evt_recv_;    // �����¼�
  CSockRecvData     c_recv_data_;   // ����buffer
  uint32            b_send_all_buffer_;

  EVT_IO            c_evt_send_;    // �����¼�
  CSockSendData     c_send_data_;   // ����buffer
  uint32            b_recv_one_packet_;
};

}  // namespace vzconn

#endif  // LIBVZCONN_CTCPCLIENT_H_
