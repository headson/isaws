/************************************************************************/
/* Author      : SoberPeng 2017-06-09
/* Description :
/************************************************************************/
#include "vshmvideo.h"

#include <string.h>

#include "stdafx.h"

VShmVideo::VShmVideo() 
  : v_shm_()
  , v_sem_w_() {
}

VShmVideo::~VShmVideo() {
  v_shm_.Close();
}

int32 VShmVideo::Create(const uint8 *s_path, uint32 n_shm_size) {
  int32 n_ret = Open(s_path, n_shm_size);
  if (n_ret == 0) {
    v_sem_w_.Signal();
  }
  return n_ret;
}

int32 VShmVideo::Open(const uint8 *s_path, uint32 n_shm_size) {
  int32 n_ret = 0;

  // 共享内存
  n_ret = v_shm_.Open((ShmKey)s_path, n_shm_size);
  if (n_ret != 0) {
    printf("shm open failed.%d.", n_ret);
    return n_ret;
  }
  n_vdo_size_ = n_shm_size - sizeof(TAG_SHM_DATA);   // 剩下为数据大小

  char sem_name[64] = {0};
  snprintf(sem_name, 63, "%s_sem_w", s_path);
  n_ret = v_sem_w_.Open((SemKey)sem_name);
  if (n_ret != 0) {
    printf("sem open failed.%d, %s.\n", n_ret, sem_name);
    return n_ret;
  }
  return 0;
}

int32 VShmVideo::Read(int8* p_data, uint32 n_data, struct timeval* p_tm) {
  if (p_data == NULL || n_data <= 0 || p_tm == NULL) {
    printf("param is error.\n");
    return -1;
  }
  if (!v_shm_.IsValid() || !v_sem_w_.IsValid()) {
    printf("shm or sem is invalid.\n");
    return -1;
  }

  TAG_SHM_DATA* p_shm = (TAG_SHM_DATA*)v_shm_.GetData();
  if (p_shm) { // 读优先
    int n_size = 0;
    if (v_sem_w_.Wait(5) == 0) {
      if (p_shm->c_tm_capture.tv_sec != p_tm->tv_sec
          || p_shm->c_tm_capture.tv_usec != p_tm->tv_usec) {
        n_size = n_data > p_shm->n_buffer ?
          p_shm->n_buffer : n_data;
        memcpy(p_data, p_shm->p_buffer, n_size);

        p_tm->tv_sec = p_shm->c_tm_capture.tv_sec;
        p_tm->tv_usec = p_shm->c_tm_capture.tv_usec;
      }
      v_sem_w_.Signal();
    }
    return n_size;
  }
  return 0;
}

int32 VShmVideo::Write(const int8           *p_data, 
                       uint32                n_data, 
                       const struct timeval *p_tm) {
  if (p_data == NULL || n_data <= 0 || p_tm == NULL) {
    printf("param is error.\n");
    return -1;
  }
  if (!v_shm_.IsValid() || !v_sem_w_.IsValid()) {
    printf("shm or sem is invalid.\n");
    return -1;
  }

  v_sem_w_.Wait();
  TAG_SHM_DATA* p_shm = (TAG_SHM_DATA*)v_shm_.GetData();
  if (p_shm && (p_shm->n_buffer+n_data) < n_vdo_size_) {
    p_shm->n_buffer  = n_data;
    memcpy(p_shm->p_buffer, p_data, n_data);
    p_shm->n_read_count = 1;

    p_shm->c_tm_capture.tv_sec  = p_tm->tv_sec;
    p_shm->c_tm_capture.tv_usec = p_tm->tv_usec;
  }
  v_sem_w_.Signal();
  return 0;
}

///分批写入buffer[h264: head + body]///////////////////////////////////////
int32 VShmVideo::WriteBegin() {
  if (!v_shm_.IsValid() || !v_sem_w_.IsValid()) {
    printf("shm or sem is invalid.\n");
    return -1;
  }
  int32 n_ret = v_sem_w_.Wait(40);
  if (n_ret == 0) {
    TAG_SHM_DATA* p_shm = (TAG_SHM_DATA*)v_shm_.GetData();
    if (p_shm) {
      p_shm->n_buffer     = 0
      p_shm->n_read_count = 0;
    }
  }
  return n_ret;
}

int32 VShmVideo::WriteOffset(const int8 *p_data, uint32 n_data) {
  TAG_SHM_DATA* p_shm = (TAG_SHM_DATA*)v_shm_.GetData();
  if (p_shm && (p_shm->n_buffer+n_data) < n_vdo_size_) {
    memcpy(p_shm->p_buffer+p_shm->n_buffer, p_data, n_data);
    p_shm->n_buffer += n_data;
  }
}

int32 VShmVideo::WriteEnd(const struct timeval *p_tm) {
  TAG_SHM_DATA* p_shm = (TAG_SHM_DATA*)v_shm_.GetData();
  if (p_shm) {
    p_shm->c_tm_capture.tv_sec  = p_tm->tv_sec;
    p_shm->c_tm_capture.tv_usec = p_tm->tv_usec;
  }
  v_sem_w_.Signal();
}


