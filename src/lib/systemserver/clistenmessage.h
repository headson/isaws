/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description : dispatcher worker,监听消息
/************************************************************************/
#ifndef _CLISTENMESSAGE_H
#define _CLISTENMESSAGE_H

#include "vzbase/base/basictypes.h"

#include "json/json.h"
#include "vzbase/thread/thread.h"

#include "system/csystem.h"
#include "network/cnetwork.h"
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
   bool GetSystemAndNetwork(Json::Value &jbody);
   bool SetSystemAndNetwork(const Json::Value &jbody);

 private:
  DPPollHandle    dp_cli_;
  vzbase::Thread *thread_fast_;   // 快速线程
  vzbase::Thread *thread_slow_;   // 耗时线程

 private:
  CSystem        *system_;        // 系统信息
  CHwclock       *hwclock_;       // 时间设置
  CNetwork       *network_;       // 网络控制
};

}  // namespace sys

#endif  // _CLISTENMESSAGE_H
