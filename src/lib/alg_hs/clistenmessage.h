/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description : dispatcher worker,监听消息
/************************************************************************/
#ifndef _CLISTENMESSAGE_H
#define _CLISTENMESSAGE_H

#include "vzbase/base/basictypes.h"
#include "vzbase/base/noncoypable.h"

#include "vzbase/thread/thread.h"

#include "dispatcher/sync/cdpclient.h"
#include "dispatcher/sync/dpclient_c.h"

#include "vzbase/system/vshm.h"
#include "alg_hs/include/iva_interface.h"

#ifdef _LINUX
#include "dev_sunxi_v40/cvideocodec.h"
#endif

namespace alg {
#define MAX_IR_NUM  5

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
  bool create_share_memory();
  bool create_alg_handle();

 protected:
  static void dpcli_poll_msg_cb(DPPollHandle p_hdl, const DpMessage *dmp, void* p_usr_arg);
  void OnDpMessage(DPPollHandle p_hdl, const DpMessage *dmp);

  static void dpcli_poll_state_cb(DPPollHandle p_hdl, unsigned int n_state, void* p_usr_arg);
  void OnDpState(DPPollHandle p_hdl, unsigned int n_state);

  // 线程消息Post,处理函数
  virtual void OnMessage(vzbase::Message* msg);

 protected:
  //智能视频分析回调函数，用于调试
  static void AlgDebugCallback(IVA_DEBUG_OUTPUT *pDebug);

  //智能视频分析回调函数，用于发送指令
  static void AlgActionCallback(IVA_ACTION_OUTPUT *pAction);

 public:
#ifdef _LINUX
  static int video_callback(VencOutputBuffer *output_buffer, void *usr_arg) {
    int res = -1;
    if (usr_arg) {
      res = ((CListenMessage*)usr_arg)->OnVideo(output_buffer);
    }
    return res;
  }
  int OnVideo(VencOutputBuffer *output_buffer);
#endif

 protected:
  DPPollHandle      dp_cli_;
  vzbase::Thread   *main_thread_;

 protected:
  IVA_HANDLE        alg_handle_;

 protected:
  struct SHM_IMG {
    unsigned int    w_sec;
    unsigned int    w_usec;

    vzbase::VShm    shm_img;
    TAG_SHM_IMG    *shm_img_ptr;
  } shm_img_[MAX_SHM_IMG];

 public:
#ifdef _LINUX
  CVideoCodec      *img_codec_;
  CVideoCodec      *vdo_codec_;   // 调试图像编码
#endif
  vzbase::VShm      shm_vdo_;
  TAG_SHM_VDO      *shm_vdo_ptr_;
};

}  // namespace alg
#endif  // _CLISTENMESSAGE_H
