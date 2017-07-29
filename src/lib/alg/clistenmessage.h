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


#include "systemv/shm/vzshm_c.h"
#include "alg/include/sdk_iva_interface.h"

namespace alg {

class CListenMessage : public vzbase::noncopyable,
  public vzbase::MessageHandler {
 protected:
  static const int POLL_TIMEOUT = 5;          // 5ms
  static const int CATCH_IMAGE  = 0x123456;

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

  // 线程消息Post,处理函数
  virtual void OnMessage(vzbase::Message* msg);

 protected:
  //智能视频分析回调函数，用于调试
  static void AlgDebugCallback(sdk_iva_debug_info *pDebug);

  //智能视频分析回调函数，用于发送指令
  static void AlgActionCallback(sdk_iva_output_info *pAction);

 protected:
  DPPollHandle      dp_cli_;
  vzbase::Thread   *main_thread_;

 protected:
  iva_count_handle  alg_handle_;      // 算法HANDLE

 protected:
  CShareBuffer      share_image_;

  char             *image_buffer_;
  unsigned int      last_read_sec_;
  unsigned int      last_read_usec_;
};

}  // namespace alg
#endif  // _CLISTENMESSAGE_H
