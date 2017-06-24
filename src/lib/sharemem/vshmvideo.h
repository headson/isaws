/************************************************************************/
/* Author      : Sober.Peng 17-06-05
/* Description :
/************************************************************************/
#ifndef SHM_CAMERA_H264_VSHMVIDEO_H
#define SHM_CAMERA_H264_VSHMVIDEO_H

#ifdef WIN32
#include <winsock2.h>  // vs2010; struct timeval
#endif 
#include "basictypes.h"

#include "vsem.h"
#include "vshm.h"

typedef struct {
  uint32          n_read_count; // 读优先
  struct timeval  c_tm_capture; // 采集时间

  uint32          n_video;
  int8            p_video[1024*1024];  // 1MB
} TAG_SHM_VIDEO;

class VShmVideo {
 public:
  VShmVideo();
  virtual ~VShmVideo();

  int32 Create(const uint8 *s_path, uint32 n_shm_size);
  int32 Open(const uint8 *s_path, uint32 n_shm_size);

  int32 Read(int8* p_data, uint32 n_data, struct timeval* p_tm);
  int32 Write(const int8* p_data, uint32 n_data, const struct timeval* p_tm);

 protected:
  VShm     v_shm_;    // 共享内存
  VSem     v_sem_w_;  // 写者
  VSem     v_sem_r_;  // 读者
};


#endif  // SHM_CAMERA_H264_VSHMVIDEO_H
