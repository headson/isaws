/************************************************************************
*Author      : Sober.Peng 17-08-17
*Description :
************************************************************************/
#ifndef LIBPLATFORM_CENCUSERDATA_H
#define LIBPLATFORM_CENCUSERDATA_H

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

class CEncUserData {
 public:
  CEncUserData(VENC_CHN chn);
  ~CEncUserData();

  int  Start(const char* skey, int shm_size, 
             unsigned int width, unsigned int height, unsigned int stride);
  void Stop();

 protected:
   static void *ThreadProcess(void *pArg);
   void OnProcess();

 private:
  const VENC_CHN    venc_chn_;

  pthread_t         dbg_yuv_pid_;

  CShareBuffer      shm_dbg_img_;
  unsigned int      w_sec_, w_usec_;

  VB_BLK            VbBlk;
  HI_U32            u32PhyAddr;
  HI_U8             *pVirAddr;
  HI_U32            u32Stride;
  HI_U32            u32Width;
  HI_U32            u32Height;
  VIDEO_FRAME_INFO_S vdo_frm_info_;
};

#endif  // LIBPLATFORM_CENCUSERDATA_H
