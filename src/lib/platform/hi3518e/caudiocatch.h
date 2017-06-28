/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#ifndef LIBPLATFORM_CAUDIOCATCH_H
#define LIBPLATFORM_CAUDIOCATCH_H

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

extern HI_S32 StartCaptureAudio(int argc, char *argv[]);

#ifdef __cplusplus
};
#endif

class CAudioCatch {
 public:
  CAudioCatch();
  ~CAudioCatch();

  int32 Start();

public:
  HI_S32 GetOneFrame(HI_U8* p_data, HI_U32 n_data);

private:
  pthread_t id;
  void      *p_shm_ado_;
};

#endif  // LIBPLATFORM_CAUDIOCATCH_H
