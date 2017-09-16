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

#include "json/json.h"

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

#include "vpsschnosd.h"

#define MAX_ENC_CHN   3

class CVideoCatch {
 public:
  CVideoCatch();
  virtual ~CVideoCatch();

  bool Start();
  void Stop();

  void SetOsdChn2(const char *osd);

  bool OSDAdjust(const Json::Value &jchn);

 public:
  HI_S32 GetOneFrame(HI_S32 n_chn, VENC_STREAM_S *p_stream);

  int    GetOneImage(void *pdata, unsigned int ndata);
  void   EncUsrImage(const void *pdata, unsigned int ndata,
                     unsigned int nwidth, unsigned int nheight);

 public:
  typedef struct {
    VENC_CHN      chn;
  } TAG_USR_ENC;

  typedef struct {
    pthread_t     pid;
    VENC_CHN      chn;
  } TAG_CHN_YUV;

 public:
  struct {
    pthread_t     pid;
    unsigned int  have_start;
    CShareBuffer  shm[MAX_ENC_CHN];
  } sys_enc;

  TAG_USR_ENC     usr_enc;
  TAG_CHN_YUV     chn1_yuv;

  TAG_OSD         enc_osd_;
};

#endif  // LIBPLATFORM_CVIDEO_H
