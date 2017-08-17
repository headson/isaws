/************************************************************************/
/* Author      : Sober.Peng 17-06-01
/* Description :
/************************************************************************/
#ifndef LIBPLATFORM_CVIDEO_H
#define LIBPLATFORM_CVIDEO_H

#include "vzbase/base/basictypes.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif

#include "common/sample_comm.h"

#ifdef __cplusplus
};
#endif

#include "systemv/shm/vzshm_c.h"
#include "systemv/flvmux/cflvmux.h"

#include "cencuserdata.h"

class CVideoCatch {
 public:
  CVideoCatch();
  virtual ~CVideoCatch();

  int32 Start();

 public:
  HI_S32 GetOneFrame(HI_S32 n_chn, VENC_STREAM_S *p_stream);
  static void* GetYUVThread(void* pArg);
  
 private:
  pthread_t     enc_pid_;
  pthread_t     osd_pid_;
  pthread_t     yuv_pid_;

  RGN_HANDLE    rgn_hdl_;
  CEncUserData  enc_user_data_;

  CShareBuffer  shm_video_[3];
  CShareBuffer  shm_image_;
};

#endif  // LIBPLATFORM_CVIDEO_H
