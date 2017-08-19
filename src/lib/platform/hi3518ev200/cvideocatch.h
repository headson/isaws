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

#include "platform/hi3518ev200/common/sample_comm.h"

#include "platform/hi3518ev200/include/mpi_vgs.h"
#include "platform/hi3518ev200/include/hi_comm_vgs.h"

#ifdef __cplusplus
};
#endif

#include "systemv/shm/vzshm_c.h"
#include "systemv/flvmux/cflvmux.h"

#define MAX_ENC_CHN   3
#define MAX_OSD_SIZE  32

class CVideoCatch {
 public:
  CVideoCatch();
  virtual ~CVideoCatch();

  bool Start();
  void Stop();

  void SetOsdChn2(const char *osd);

 public:
  HI_S32 GetOneFrame(HI_S32 n_chn, VENC_STREAM_S *p_stream);

  static void* GetYUVThread(void* pArg);

 public:
  typedef struct {
    pthread_t     pid;
    CShareBuffer  shm;
    VENC_CHN      chn;
  } TAG_USR_ENC;

  typedef struct {
    pthread_t     pid;
    CShareBuffer  shm;
    VENC_CHN      chn;
  } TAG_CHN_YUV;

  typedef struct {
    pthread_t     pid;
    char          ch1[MAX_OSD_SIZE];
    char          ch2[MAX_OSD_SIZE];
    char          ch3[MAX_OSD_SIZE];
  } TAG_OSD;

 private:
  struct {
    pthread_t     pid;
    CShareBuffer  shm[MAX_ENC_CHN];
  } sys_enc;

  TAG_USR_ENC     usr_enc;
  TAG_CHN_YUV     chn1_yuv;

  TAG_OSD         osd_;
};

#endif  // LIBPLATFORM_CVIDEO_H
