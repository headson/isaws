/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description : dispatcher worker,������Ϣ
/************************************************************************/
#ifndef _CLISTENMESSAGE_H
#define _CLISTENMESSAGE_H

#include "vzbase/base/basictypes.h"
#include "vzbase/base/noncoypable.h"

#include "vzbase/thread/thread.h"

#include "dispatcher/sync/dpclient_c.h"

namespace bs {

class CListenMessage : public vzbase::noncopyable,
  public vzbase::MessageHandler {
 protected:
  CListenMessage();
  virtual ~CListenMessage();

 public:
  static CListenMessage *Instance();

  bool  Start();
  void  Stop();

  void  RunLoop();

  vzbase::Thread  *MainThread();
  DPPollHandle     GetDpPollHdl();

 protected:
  static void dpcli_poll_msg_cb(DPPollHandle p_hdl, const DpMessage *dmp, void* p_usr_arg);
  void OnDpMessage(DPPollHandle p_hdl, const DpMessage *dmp);

  static void dpcli_poll_state_cb(DPPollHandle p_hdl, unsigned int n_state, void* p_usr_arg);
  void OnDpState(DPPollHandle p_hdl, unsigned int n_state);

  // �߳���ϢPost,������
  void OnMessage(vzbase::Message* msg);

 protected:
  DPPollHandle      dp_cli_;
  vzbase::Thread   *main_thread_;

 protected:
  struct {
    unsigned int    day_bng_hour_;    // һ�쿪ʼСʱ
    unsigned int    day_bng_minute_;  // һ�쿪ʼ����
    unsigned int    day_end_hour_;    // һ�����Сʱ
    unsigned int    day_end_minute_;  // һ���������

    unsigned int    send_interval_;   // ���ͼ��(minute)
    time_t          last_send_time_;  // �ϴη���ʱ��
  } valid_times_;                     // ��Чʱ��
};

}  // namespace bs
#endif  // _CLISTENMESSAGE_H
