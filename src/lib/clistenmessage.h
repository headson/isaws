/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description : dispatcher worker,¼àÌýÏûÏ¢
/************************************************************************/
#ifndef _CLISTENMESSAGE_H
#define _CLISTENMESSAGE_H

#include "vzbase/base/basictypes.h"
#include "vzbase/base/noncoypable.h"
#include "dispatcher/sync/dpclient_c.h"

class CListenMessage : public vzbase::noncopyable {
 protected:
  CListenMessage();
  virtual ~CListenMessage();

public:
  static CListenMessage *Instance();

  bool  Start(const uint8 *s_dp_ip, uint16 n_dp_port);
  void  Stop();

  void  RunLoop();

 protected:
  static void dpcli_poll_msg_cb(DPPollHandle p_hdl, const DpMessage *dmp, void* p_usr_arg);
  void OnDpCliMsg(DPPollHandle p_hdl, const DpMessage *dmp);

  static void dpcli_poll_state_cb(DPPollHandle p_hdl, uint32 n_state, void* p_usr_arg);
  void OnDpCliState(DPPollHandle p_hdl, uint32 n_state);

 private:
  DPPollHandle               p_dp_cli_;
};

#endif  // _CLISTENMESSAGE_H
