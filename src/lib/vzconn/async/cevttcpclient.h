/************************************************************************/
/* Author      : Sober.Peng 17-06-15
/* Description : ��TCP Server�����Ķ����û��㲻��ֱ�ӵ���delete�ɵ�,����CloseSocket�ر�����
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

  // �����Ͽ�����
  virtual void Remove();

 public:
  // �����Ѵ򿪵�SCOKET
  virtual int32 Open(SOCKET s, bool b_block=false);

 public:
  // ���ӵ������;�������Open
  virtual int32 Connect(const CInetAddr *p_remote_addr,
                        bool             b_block,
                        bool             b_reuse,
                        uint32           n_timeout=5000);

 public:
  /************************************************************************/
  /* Description : ����һ������
  /* Parameters  : p_data[IN] ����(body��)
                   n_data[IN] ���ݳ���
                   p_param[IN] ����.(VZΪ��ͷ��flag[uint16])
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

  // �첽���ӷ�����,��ʱ���ɹ�,���о�
  static int32  EvtConnect(SOCKET      fd,
                           short       events,
                           const void *p_usr_arg);
  virtual int32 OnConnect(SOCKET fd);

 protected:
  const EVT_LOOP   *p_evt_loop_;    // ������˳�������,���ع�����������

 protected:
  EVT_IO            c_evt_recv_;    // �����¼�
  CBlockBuffer      c_recv_data_;   // ����buffer

  EVT_IO            c_evt_send_;    // �����¼�
  CBlockBuffer      c_send_data_;   // ����buffer

 protected:
  CClientInterface  *cli_handle_ptr_;  // �ͻ����¼�����
};

#endif  // LIBVZCONN_CEVTTCPCLIENT_H_
