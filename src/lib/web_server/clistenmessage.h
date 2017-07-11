/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description : dispatcher worker,¼àÌýÏûÏ¢
/************************************************************************/
#ifndef _CLISTENMESSAGE_H
#define _CLISTENMESSAGE_H

#include "vzbase/base/basictypes.h"

#include "vzbase/base/noncoypable.h"

#include "dispatcher/sync/dpclient_c.h"

#include "web_server/cwebserver.h"

class CListenMessage : public vzbase::noncopyable {
 protected:
  CListenMessage();
  virtual ~CListenMessage();

public:
  static CListenMessage *Instance();

  bool  Start(const char *s_dp_ip, unsigned short n_dp_port,
              const char *s_http_port, const char *s_http_path);
  void  Stop();

  void  RunLoop();

 protected:
  static void dpcli_poll_msg_cb(DPPollHandle p_hdl, const DpMessage *dmp, void* p_usr_arg);
  void OnDpCliMsg(DPPollHandle p_hdl, const DpMessage *dmp);

  static void dpcli_poll_state_cb(DPPollHandle p_hdl, unsigned int n_state, void* p_usr_arg);
  void OnDpCliState(DPPollHandle p_hdl, unsigned int n_state);

 private:
  DPPollHandle               p_dp_cli_;

  CWebServer                 c_web_srv_;
};

#endif  // _CLISTENMESSAGE_H
