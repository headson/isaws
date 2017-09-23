/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#ifndef LIBDISPATCH_CDPPOLLCLIENT_H_
#define LIBDISPATCH_CDPPOLLCLIENT_H_

#include "vzbase/base/basictypes.h"

#include "dispatcher/sync/dpclient_c.h"

#include "vzconn/sync/ctcpclient.h"
#include "dispatcher/sync/cdpclient.h"

class CDpPollClient : public CDpClient {
 protected:
  CDpPollClient(const char *server, unsigned short port,
                DpClient_MessageCallback   p_msg_cb,
                void                      *p_msg_usr_arg,
                DpClient_PollStateCallback p_state_cb,
                void                      *p_state_usr_arg,
                vzconn::EVT_LOOP          *p_evt_loop);

 public:
  static CDpPollClient* Create(const char *server, unsigned short port,
                               DpClient_MessageCallback   p_msg_cb,
                               void                      *p_msg_usr_arg,
                               DpClient_PollStateCallback p_state_cb,
                               void                      *p_state_usr_arg,
                               vzconn::EVT_LOOP          *p_evt_loop = NULL);
  virtual ~CDpPollClient();

 public:
  vzconn::EVT_LOOP *GetEvtLoop() {
    return p_evt_loop_;
  }

  /* method;add\remove */
  int32 ListenMessage(uint8        e_type,
                      const char  *p_method[],    /* ���������32Byte���ȷ��� */
                      unsigned int n_method_no,   /* �������� */
                      uint16       n_flag);

  bool CheckAndConnected();


 protected:
  virtual int32 HandleRecvPacket(vzconn::VSocket  *p_cli,
                                 const uint8            *p_data,
                                 uint32                  n_data,
                                 uint16                  n_flag);
  virtual int32 HandleSendPacket(vzconn::VSocket *p_cli) {
    return 0;
  }
  virtual void  HandleClose(vzconn::VSocket *p_cli) {
  }

  static int32 evt_timer_cb(SOCKET          fd,
                            short           events,
                            const void      *p_usr_arg) {
    if (p_usr_arg) {
      ((CDpPollClient*)p_usr_arg)->OnEvtTimer();
    }
    return 0;
  }

  int32 OnEvtTimer();

 public:
  class CFlagLive {
   public:
    CFlagLive(uint32 &flag)
      : flag_(flag) {
      flag_ = 1;
    }
    ~CFlagLive() {
      flag_ = 0;
    }
   private:
    uint32 &flag_;
  };

 protected:
  vzconn::EVT_TIMER            c_evt_timer_;   // ������

  uint32                       is_add_msg_;    // 1=����ע����Ϣ;����ע��ʱ���յ���Ϣ,��������ѭ��
  uint32                       had_reg_msg_;   // 0=��Ҫע����Ϣ,1=��ע����Ϣ

 protected:
  DpClient_MessageCallback     p_poll_msg_cb_;        // ��Ϣ�ص�
  void                        *p_poll_msg_usr_arg_;   // �ص��û�����

  DpClient_PollStateCallback   p_state_cb_;           // ״̬�ص�
  void                        *p_state_usr_arg_;      // �ص��û�����
};

#endif  // LIBDISPATCH_CDPCLIENT_H_
