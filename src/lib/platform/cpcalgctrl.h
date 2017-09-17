/************************************************************************
*Author      : Sober.Peng 17-09-16
*Description :
************************************************************************/
#ifndef LIBPLATFORM_CPCALGCTRL_H_
#define LIBPLATFORM_CPCALGCTRL_H_

#include "vzbase/base/basictypes.h"
#include "vzbase/base/noncoypable.h"

#include "vzbase/thread/thread.h"

#include "dispatcher/sync/cdpclient.h"
#include "dispatcher/sync/dpclient_c.h"

#include "systemv/shm/vzshm_c.h"
#include "alg/include/sdk_iva_interface.h"

#define MAX_IR_NUM  5

class CAlgCtrl {
 protected:
  static const int POLL_TIMEOUT = 5;          // 5ms
  static const int CATCH_IMAGE  = 0x123456;

 public:
  CAlgCtrl(vzbase::Thread *fast_thread);
  virtual ~CAlgCtrl();

 public:
  bool  Start();
  void  Stop();

  bool  OnImage(void *pimg, unsigned int nimg);

  void  GetAlgVer(DpMessage *dmp);

 protected:
  //智能视频分析回调函数，用于调试
  static void AlgDebugCallback(sdk_iva_debug_info *pDebug);

  //智能视频分析回调函数，用于发送指令
  static void AlgActionCallback(sdk_iva_output_info *pAction);

 protected:
  vzbase::Thread   *fast_thread_;
  CDpClient        *remote_dp_client_;

 protected:
  iva_count_handle  alg_handle_;      // 算法HANDLE
  struct TAG_IRS {
    unsigned int   is_new;
    unsigned int   ir[MAX_IR_NUM];
  };
  TAG_IRS                 ir_local_;
  vzbase::CriticalSection ir_mutex_;
  TAG_IRS                 ir_remote_;
};

#endif  // LIBPLATFORM_CALGCONTROL_H_
