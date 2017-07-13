/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description : dispatcher worker,监听消息
/************************************************************************/
#ifndef _CLISTENMESSAGE_H
#define _CLISTENMESSAGE_H

#include "vzbase/base/basictypes.h"

#include "vzbase/base/noncoypable.h"

#include "dispatcher/sync/dpclient_c.h"

#include "web_for_dp/cwebserver.h"

namespace web {

extern const char *K_METHOD_SET[MAX_METHOD_COUNT];

class CListenMessage : public vzbase::noncopyable,
  public vzbase::MessageHandler {
 protected:
  CListenMessage();
  virtual ~CListenMessage();

 public:
  static CListenMessage *Instance();

  bool  Start(const char *s_dp_ip, unsigned short n_dp_port,
              unsigned short n_http_port, const char *s_http_path);

  // 重启dp_client,更换IP地址
  bool  Restart(const char *s_dp_ip, unsigned short n_dp_port);

  void  Stop();

  void  RunLoop();

  vzbase::Thread  *MainThread();
  DPPollHandle     GetDpPollHdl();

  bool  AddListenMessage(std::string s_method);
  bool  DelListenMessage(std::string s_method);

 protected:
  static void dpcli_poll_msg_cb(DPPollHandle p_hdl, const DpMessage *dmp, void* p_usr_arg);
  void OnDpCliMsg(DPPollHandle p_hdl, const DpMessage *dmp);

  static void dpcli_poll_state_cb(DPPollHandle p_hdl, unsigned int n_state, void* p_usr_arg);
  void OnDpCliState(DPPollHandle p_hdl, unsigned int n_state);

  // 线程消息Post,处理函数
  void OnMessage(vzbase::Message* msg);

 private:
  vzconn::EventService      *p_evt_srv_;

  DPPollHandle               p_dp_cli_;
  CWebServer                 c_web_srv_;
};

}  // namespace web
#endif  // _CLISTENMESSAGE_H
