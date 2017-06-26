/************************************************************************/
/* Author      : Sober.Peng 17-06-01
/* Description :
/************************************************************************/
#ifndef LIBPLATFORM_CVIDEO_H
#define LIBPLATFORM_CVIDEO_H

#include "basictypes.h"

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

  extern HI_VOID *SAMPLE_VENC_1080P_CLASSIC(HI_VOID *p);

#ifdef __cplusplus
};
#endif

#include "sharemem/vshmvideo.h"

class CVideoCatch {
 public:
  CVideoCatch();
  virtual ~CVideoCatch();

  int32 Start();

 public:
  HI_S32 GetOneFrame(HI_S32 n_chn, VENC_STREAM_S *p_stream);

 private:
  pthread_t id;
  
  VShmVideo c_shm_vdo_;
  char*     p_vdo_data_;
};

#endif  // LIBPLATFORM_CVIDEO_H
