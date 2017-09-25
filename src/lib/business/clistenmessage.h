/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description : dispatcher worker,监听消息
/************************************************************************/
#ifndef _CLISTENMESSAGE_H
#define _CLISTENMESSAGE_H

#include "vzbase/base/basictypes.h"
#include "vzbase/base/boost_settings.hpp"

#include "vzbase/thread/thread.h"

#include "dispatcher/sync/dpclient_c.h"

#include "cdatabase.h"

namespace bs {

class CListenMessage : public boost::noncopyable,
  public vzbase::MessageHandler {
 protected:
  CListenMessage();
  virtual ~CListenMessage();

 public:
  static CListenMessage *Instance();

  bool  Start(const char *db_path);
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

 protected:
  DPPollHandle      dp_cli_;
  vzbase::Thread   *main_thread_;

 protected:
  struct {
    unsigned int    day_bng_hour_;    // 一天开始小时
    unsigned int    day_bng_minute_;  // 一天开始分钟
    unsigned int    day_end_hour_;    // 一天结束小时
    unsigned int    day_end_minute_;  // 一天结束分钟

    unsigned int    send_interval_;   // 发送间隔(minute)
    time_t          last_send_time_;  // 上次发送时间
  } valid_times_;                     // 有效时间

  CDataBase         database_;
};

}  // namespace bs
#endif  // _CLISTENMESSAGE_H
