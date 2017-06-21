/************************************************************************/
/* Author      : Sober.Peng 17-06-21
/* Description :
/************************************************************************/
#ifndef LIBDISPATCH_CDPCLIENT_H_
#define LIBDISPATCH_CDPCLIENT_H_

#include "basictypes.h"
#include "dpclient_c.h"
#include "vzconn/async/cevttcpclient.h"

class CClientProcess : public vzconn::CClientInterface {
 public:
  virtual int32 HandleRecvPacket(vzconn::VSocket  *p_cli,
                                 const uint8            *p_data,
                                 uint32                  n_data,
                                 uint16                  n_flag);
  virtual int32 HandleSendPacket(vzconn::VSocket *p_cli) {
    return 0;
  }
  virtual void  HandleClose(vzconn::VSocket *p_cli);
};

class CTcpClient : public vzconn::CEvtTcpClient {
 protected:
  CTcpClient();
  virtual void  Remove() { }

 public:
  static CTcpClient* Create();
  virtual ~CTcpClient();

 public:
  // �����Ѵ򿪵�SCOKET
  virtual bool  Open(SOCKET s, bool b_block = false);

  /************************************************************************/
  /* Description : ������Ϣ
  /* Parameters  : b_wait_recv[IN] true �ȴ��뷢�Ͷ�Ӧ�Ļ�ִ����;���յ�idһ�µ��˳�,����ʱ�˳�loop
                                   false ���յ�һ�������˳�loop
  /* Return      :
  /************************************************************************/
  void          Reset(bool b_wait_recv=false);

  int32         RunLoop(uint32 n_timeout);
  int32         PollRunLoop(uint32 n_timeout);

  void          SetCallback(DpClient_MessageCallback callback,
                            void *p_usr_arg);

  /************************************************************************/
  /* Description : ����method;add\remove
  /************************************************************************/
  virtual int32 ListenMessage(DpMessage   *dP_msg,
                              const char  *method_set[],
                              unsigned int set_size,
                              uint16       n_flag);

 protected:
  virtual int32 OnRecv();
  virtual int32 OnSend();

 public:
  friend class CClientProcess;

 public:
  uint32 get_chn_id() {
    return n_chn_id_;
  }
  uint32 new_msg_seq() {
    if (n_msg_seq_ >= MAX_MESSAGE_ID) {
      n_msg_seq_ = 1;
    }

    n_msg_seq_++;
    return n_chn_id_ + n_msg_seq_;
  }
  uint32 get_msg_req() {
    if (n_msg_seq_ >= MAX_MESSAGE_ID) {
      n_msg_seq_ = 1;
    }
    return n_chn_id_ + n_msg_seq_;
  }
  uint32 get_recv_one_packet() {
    return n_recv_one_packet_;
  }
  bool   get_send_all_buffer() {
    return (1 == b_send_all_buffer_) ? true : false;
  }

  uint32 get_resp_type() {
    return n_resp_type_;
  }

 protected:
  vzconn::EVT_LOOP          evt_loop_;    //
  uint32                    n_recv_one_packet_;   // �յ�N������
  uint32                    b_send_all_buffer_;   // ������buffer

 protected:
  DpClient_MessageCallback  callback_;    // �ص�
  void                     *p_usr_arg_;   // �ص��û�����
  CClientProcess            c_cli_proc_;  // ��Ϣ����

 protected:
  uint32                    n_chn_id_;    // ͨ�����
  uint32                    n_msg_seq_;   // �����

 protected:
  // 1 �ȴ��뷢�Ͷ�Ӧ�Ļ�ִ����;���յ�idһ�µ��˳�,����ʱ�˳�loop
  // 0 ���յ�һ�������˳�loop
  uint32                    b_wait_recv_;
  // b_wait_recv_=1ʱ,���ػ�ִ����type
  uint32                    n_resp_type_;

  static const int          MAX_MESSAGE_ID = 0X00FFFFFF;

 public:
  static int EncDpMsg(DpMessage      *p_msg,
                      unsigned char   n_type,
                      unsigned char   n_chn_id,
                      const char     *method,
                      unsigned int    n_msg_id,
                      int             data_size);

  static int DecDpMsg(DpMessage     *p_msg,
                      const void    *p_data,
                      uint32         n_data);
};

#endif  // LIBDISPATCH_CDPCLIENT_H_
