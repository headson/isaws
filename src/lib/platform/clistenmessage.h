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

#include "platform/cpcalgctrl.h"
#include "hi3518ev200/cvideocatch.h"

namespace platform {

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

  vzbase::Thread  *MainThread() {
    return main_thread_;
  }
  DPPollHandle     GetDpPollHdl() {
    return dp_cli_;
  }
  CAlgCtrl        *GetAlgCtrl() {
    return alg_ctrl_;
  }
  CVideoCatch     *GetVdoCatch() {
    return vdo_catch_;
  }

 protected:
  static void dpcli_poll_msg_cb(DPPollHandle p_hdl, const DpMessage *dmp, void* p_usr_arg);
  void OnDpMessage(DPPollHandle p_hdl, const DpMessage *dmp);

  static void dpcli_poll_state_cb(DPPollHandle p_hdl, unsigned int n_state, void* p_usr_arg);
  void OnDpState(DPPollHandle p_hdl, unsigned int n_state);

  // �߳���ϢPost,������
  void OnMessage(vzbase::Message* msg);

 private:
  DPPollHandle      dp_cli_;
  vzbase::Thread   *main_thread_;

  CAlgCtrl         *alg_ctrl_;
  CVideoCatch      *vdo_catch_;
};

}  // namespace platform
#endif  // _CLISTENMESSAGE_H
