/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description : dispatcher worker,监听消息
/************************************************************************/
#ifndef _CLISTENMESSAGE_H
#define _CLISTENMESSAGE_H

#include "vzbase/base/basictypes.h"
#include "vzbase/base/noncoypable.h"

#include "vzbase/thread/thread.h"

#include "dispatcher/sync/dpclient_c.h"

#include "web_server/cwebserver.h"

namespace web {

class CListenMessage : public vzbase::noncopyable,
  public vzbase::MessageHandler {
 protected:
  CListenMessage();
  virtual ~CListenMessage();

public:
  static CListenMessage *Instance();

  bool  Start(unsigned short n_http_port, const char *s_http_path);
  void  Stop();

  void  RunLoop();

  vzbase::Thread  *MainThread();
  DPPollHandle     GetDpPollHdl();

 protected:
  static void dpcli_poll_msg_cb(DPPollHandle p_hdl, const DpMessage *dmp, void* p_usr_arg);
  void OnDpMessage(DPPollHandle p_hdl, const DpMessage *dmp);

  static void dpcli_poll_state_cb(DPPollHandle p_hdl, unsigned int n_state, void* p_usr_arg);
  void OnDpState(DPPollHandle p_hdl, unsigned int n_state);

  // 线程消息Post,处理函数
  void OnMessage(vzbase::Message* msg);

 private:
  DPPollHandle               p_dp_cli_;
  vzbase::Thread            *p_main_thread_;

  CWebServer                 c_web_srv_;
};

}  // namespace web 
#endif  // _CLISTENMESSAGE_H
