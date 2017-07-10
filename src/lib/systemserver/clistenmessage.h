/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description : dispatcher worker,监听消息
/************************************************************************/
#ifndef _CLISTENMESSAGE_H
#define _CLISTENMESSAGE_H

#include "vzbase/base/basictypes.h"

#include "json/json.h"
#include "vzbase/thread/thread.h"
#include "vzbase/base/noncoypable.h"
#include "dispatcher/sync/dpclient_c.h"

class CListenMessage : public vzbase::noncopyable {
 protected:
  CListenMessage();
  virtual ~CListenMessage();

 public:
  static CListenMessage *Instance();

  bool  Start(const unsigned char *s_dp_ip, unsigned short n_dp_port);
  void  Stop();

  void  RunLoop();

 protected:
  static void dpcli_poll_msg_cb(DPPollHandle p_hdl, const DpMessage *dmp, void* p_usr_arg);
  void OnDpCliMsg(DPPollHandle p_hdl, const DpMessage *dmp);

  static void dpcli_poll_state_cb(DPPollHandle p_hdl, unsigned int n_state, void* p_usr_arg);
  void OnDpCliState(DPPollHandle p_hdl, unsigned int n_state);

 public:
  void GetHwInfo();                           // 获取硬件信息

  bool GetDevInfo(Json::Value &j_body);       // 获取设备信息
  bool SetDevInfo(const Json::Value &j_body); // 设置设备信息

 private:
  DPPollHandle               p_dp_cli_;
};

#endif  // _CLISTENMESSAGE_H
