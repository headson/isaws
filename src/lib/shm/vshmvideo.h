/************************************************************************/
/* Author      : Sober.Peng 17-06-05
/* Description :
/************************************************************************/
#ifndef SHM_CAMERA_H264_VSHMVIDEO_H
#define SHM_CAMERA_H264_VSHMVIDEO_H

#include <stdint.h>

#include "vsem.h"
#include "vshm.h"

typedef struct {
  uint32_t        n_read_count; // ������
  struct timeval  c_tm_capture; // �ɼ�ʱ��

  uint32_t        n_video;
  int8_t          p_video[1024*1024];  // 1MB
} TAG_SHM_VIDEO;

class VShmVideo {
 public:
  VShmVideo();
  virtual ~VShmVideo();

  int32_t Open(const int8_t *p_name, uint32_t n_shm_size);

  int32_t Read(int8_t* p_data, uint32_t n_data, struct timeval* p_tm);
  int32_t Write(const int8_t* p_data, uint32_t n_data, const struct timeval* p_tm);

 protected:
  VShm     v_shm_;    // �����ڴ�
  VSem     v_sem_w_;  // д��
  VSem     v_sem_r_;  // ����
};


#endif  // SHM_CAMERA_H264_VSHMVIDEO_H
