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

#include "mpp/include/sample_comm.h"

  extern HI_VOID *SAMPLE_VENC_720P_CLASSIC(HI_VOID *p);

#ifdef __cplusplus
};
#endif

class CVideoCatch {
 public:
  CVideoCatch();
  virtual ~CVideoCatch();

  int32 Start();

 public:
  HI_S32 GetOneFrame(HI_S32 n_chn, VENC_STREAM_S *p_stream);

 private:
  pthread_t    id;
  void        *p_shm_vdo_;
};

#endif  // LIBPLATFORM_CVIDEO_H
