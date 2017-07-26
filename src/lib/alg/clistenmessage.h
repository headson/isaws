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

#include "iva/include/iva_interface.h"

namespace iva {

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
protected:
  //������Ƶ�����ص����������ڵ���
  static void DebugCallback(IVA_DEBUG_OUTPUT *pDebug) {
    if (pDebug && pDebug->user_arg) {
      ((CListenMessage*)pDebug->user_arg)->OnDebug(pDebug);
    }
  }

  void OnDebug(IVA_DEBUG_OUTPUT *pDebug);

  //������Ƶ�����ص����������ڷ���ָ��
  static void ActionCallback(IVA_ACTION_OUTPUT *pAction) {
    if (pAction && pAction->user_arg) {
      ((CListenMessage*)pAction->user_arg)->OnAction(pAction);
    }
  }

  void OnAction(IVA_ACTION_OUTPUT *pAction);

 private:
  DPPollHandle      dp_cli_;
  vzbase::Thread   *main_thread_;

  IVA_HANDLE        iva_Handle_;      // �㷨HANDLE
};

}  // namespace iva
#endif  // _CLISTENMESSAGE_H
