/************************************************************************/
/* Author      : SoberPeng 2017-06-09
/* Description :
/************************************************************************/
#include "vshmvideo.h"

#include <string.h>

#include "base/stdafx.h"

VShmVideo::VShmVideo() {

}

VShmVideo::~VShmVideo() {
  v_shm_.Close();
}

int32_t VShmVideo::Create(uint32_t n_shm_size) {
  int32_t n_ret = Open(n_shm_size);
  if (n_ret == 0) {
    v_sem_w_.Signal();
    v_sem_r_.Signal();
  }
  return n_ret;
}

int32_t VShmVideo::Open(uint32_t n_shm_size) {
  int32_t n_ret = 0;

  // 共享内存
  n_ret = v_shm_.Open(DEF_SHM_VIDEO_0, n_shm_size);
  if (n_ret != 0) {
    printf("shm open failed.%d.", n_ret);
    return n_ret;
  }

  n_ret = v_sem_w_.Open(DEF_SEM_VIDEO_0_W);
  if (n_ret != 0) {
    printf("sem open failed.%d.\n", n_ret);
    return n_ret;
  }

  n_ret = v_sem_r_.Open(DEF_SEM_VIDEO_0_R);
  if (n_ret != 0) {
    printf("sem open failed.%d.\n", n_ret);
    return n_ret;
  }
  return 0;
}

int32_t VShmVideo::Read(int8_t* p_data, uint32_t n_data, struct timeval* p_tm) {
  if (p_data == NULL || n_data <= 0 || p_tm == NULL) {
    printf("param is error.\n");
    return -1;
  }
  if (!v_shm_.IsValid() || !v_sem_w_.IsValid()) {
    printf("shm or sem is invalid.\n");
    return -1;
  }

  TAG_SHM_VIDEO* p_shm = (TAG_SHM_VIDEO*)v_shm_.GetData();
  if (p_shm) { // 读优先
#if 1
    v_sem_r_.Wait();
    p_shm->n_read_count++;
    if (p_shm->n_read_count == 1)
      v_sem_w_.Wait();
    v_sem_r_.Signal();

    int n_size = 0;
    if (p_shm->c_tm_capture.tv_sec != p_tm->tv_sec
        || p_shm->c_tm_capture.tv_usec != p_tm->tv_usec) {
      n_size = n_data > p_shm->n_video ?
               p_shm->n_video : n_data;
      memcpy(p_data, p_shm->p_video, n_size);

      p_tm->tv_sec = p_shm->c_tm_capture.tv_sec;
      p_tm->tv_usec = p_shm->c_tm_capture.tv_usec;
    }

    v_sem_r_.Wait();
    p_shm->n_read_count--;
    if (p_shm->n_read_count == 0)
      v_sem_w_.Signal();
    v_sem_r_.Signal();
#else
    v_sem_w_.Wait();
    int n_size = 0;
    if (p_shm->c_tm_capture.tv_sec != p_tm->tv_sec
        || p_shm->c_tm_capture.tv_usec != p_tm->tv_usec) {
      n_size = n_data > p_shm->n_video ?
               p_shm->n_video : n_data;
      memcpy(p_data, p_shm->p_video, n_size);

      p_tm->tv_sec = p_shm->c_tm_capture.tv_sec;
      p_tm->tv_usec = p_shm->c_tm_capture.tv_usec;
    }
    v_sem_w_.Signal();
#endif
    return n_size;
  }
  return 0;
}

int32_t VShmVideo::Write(const int8_t* p_data, uint32_t n_data, const struct timeval* p_tm) {
  if (p_data == NULL || n_data <= 0 || p_tm == NULL) {
    printf("param is error.\n");
    return -1;
  }
  if (!v_shm_.IsValid() || !v_sem_w_.IsValid()) {
    printf("shm or sem is invalid.\n");
    return -1;
  }

  v_sem_w_.Wait();

  TAG_SHM_VIDEO* p_shm = (TAG_SHM_VIDEO*)v_shm_.GetData();
  if (p_shm) {
    p_shm->n_video = n_data;
    memcpy(p_shm->p_video, p_data, n_data);
    p_shm->n_read_count = 1;

    p_shm->c_tm_capture.tv_sec  = p_tm->tv_sec;
    p_shm->c_tm_capture.tv_usec = p_tm->tv_usec;
  }

  v_sem_w_.Signal();
  return 0;
}

