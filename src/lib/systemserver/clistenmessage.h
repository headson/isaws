/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description : dispatcher worker,������Ϣ
/************************************************************************/
#ifndef _CLISTENMESSAGE_H
#define _CLISTENMESSAGE_H

#include "vzbase/base/basictypes.h"

#include "json/json.h"
#include "vzbase/thread/thread.h"

#include "network/cnetctrl.h"
#include "hwclock/chwclock.h"

#include "vzbase/base/noncoypable.h"
#include "dispatcher/sync/dpclient_c.h"

#include "systemserver/base/pkghead.h"

namespace sys {

class CListenMessage : public vzbase::noncopyable {
 protected:
  CListenMessage();
  virtual ~CListenMessage();

 public:
  static CListenMessage *Instance();

  bool  Start(const char *s_dp_ip, unsigned short n_dp_port);
  void  Stop();

  void  RunLoop();

 protected:
  static void dpcli_poll_msg_cb(DPPollHandle p_hdl, const DpMessage *dmp, void* p_usr_arg);
  void OnDpCliMsg(DPPollHandle p_hdl, const DpMessage *dmp);

  static void dpcli_poll_state_cb(DPPollHandle p_hdl, unsigned int n_state, void* p_usr_arg);
  void OnDpCliState(DPPollHandle p_hdl, unsigned int n_state);

 public:
  void GetHwInfo();                           // ��ȡӲ����Ϣ

  bool GetDevInfo(Json::Value &j_body);       // ��ȡ�豸��Ϣ
  bool SetDevInfo(const Json::Value &j_body); // �����豸��Ϣ

 private:
  TAG_SYS_INFO    sys_info_;    // ϵͳ��Ϣ

  DPPollHandle    dp_cli_;
  CNetCtrl       *net_ctrl_;    // �������
  CHwclock       *hw_clock_;    // ʱ������

  vzbase::Thread *thread_fast_;  // �����߳�
  vzbase::Thread *thread_slow_;  // ��ʱ�߳�
};

}  // namespace sys

#endif  // _CLISTENMESSAGE_H
