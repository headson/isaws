/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description : dispatcher worker,监听消息
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

  bool  Start();
  void  Stop();

  void  RunLoop();

 protected:
  static void dpcli_poll_msg_cb(DPPollHandle p_hdl,
                                const DpMessage *dmp, void* p_usr_arg);
  void OnDpMessage(DPPollHandle p_hdl, const DpMessage *dmp);

  static void dpcli_poll_state_cb(DPPollHandle p_hdl,
                                  unsigned int n_state, void* p_usr_arg);
  void OnDpState(DPPollHandle p_hdl, unsigned int n_state);

 public:
  void GetHwInfo();                           // 获取硬件信息

  bool GetDevInfo(Json::Value &j_body);       // 获取设备信息
  bool SetDevInfo(const Json::Value &j_body); // 设置设备信息

 private:
  Json::Value     hw_json_;       // 硬件参数json

 private:
  DPPollHandle    dp_cli_;
  vzbase::Thread *thread_fast_;   // 快速线程
  vzbase::Thread *thread_slow_;   // 耗时线程

 private:
  CNetCtrl       *net_ctrl_;      // 网络控制
  CHwclock       *hw_clock_;      // 时间设置
};

}  // namespace sys

#endif  // _CLISTENMESSAGE_H
