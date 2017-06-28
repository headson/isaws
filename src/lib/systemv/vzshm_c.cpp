/************************************************************************/
/* Author      : SoberPeng 2017-06-09
/* Description :
/************************************************************************/
#include "vzshm_c.h"

#include <string.h>

#include "vz_shm.h"

class CShareMemory {
 public:
  CShareMemory()
    : hdl_shm_(NULL)
    , hdl_sem_w_(NULL)
    , hdl_sem_r_(NULL) {
  }

  ~CShareMemory() {
  }

  int32 Create(const char *s_path, unsigned int n_size) {
    int32 n_ret = Open(s_path, n_shm_size);
    if (n_ret == 0) {
      v_sem_w_.Signal();
    }
    return n_ret;
  }

  int32 Open(const char *s_file, unsigned int n_size) {
    // 共享内存
    hdl_shm_ = vzShmOpen(s_file, n_size);
    if (hdl_shm_ == HDL_NULL) {
      printf("shm open failed.");
      return n_ret;
    }

    char sem_name[64] = { 0 };
    snprintf(sem_name, 63, "%s_sem_w", s_file);
    hdl_sem_w_ = vzSemOpen(sem_name);
    if (hdl_sem_w_ == HDL_NULL) {
      printf("sem open failed %s.\n", sem_name);
      return n_ret;
    }
    return 0;
  }

  int32 Read(int8* p_data, uint32 n_data, struct timeval* p_tm) {
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

  int32 Write(const int8           *p_data,
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
    if (p_shm && (p_shm->n_buffer + n_data) < n_vdo_size_) {
      p_shm->n_buffer = n_data;
      memcpy(p_shm->p_buffer, p_data, n_data);
      p_shm->n_read_count = 1;

      p_shm->c_tm_capture.tv_sec = p_tm->tv_sec;
      p_shm->c_tm_capture.tv_usec = p_tm->tv_usec;
    }
    v_sem_w_.Signal();
    return 0;
  }

  ///分批写入buffer[h264: head + body]///////////////////////////////////////
  int32 WriteBegin() {
    if (!v_shm_.IsValid() || !v_sem_w_.IsValid()) {
      printf("shm or sem is invalid.\n");
      return -1;
    }
    int32 n_ret = v_sem_w_.Wait(40);
    if (n_ret == 0) {
      TAG_SHM_DATA* p_shm = (TAG_SHM_DATA*)v_shm_.GetData();
      if (p_shm) {
        p_shm->n_buffer = 0
                          p_shm->n_read_count = 0;
      }
    }
    return n_ret;
  }

  int32 WriteOffset(const int8 *p_data, uint32 n_data) {
    TAG_SHM_DATA* p_shm = (TAG_SHM_DATA*)v_shm_.GetData();
    if (p_shm && (p_shm->n_buffer + n_data) < n_vdo_size_) {
      memcpy(p_shm->p_buffer + p_shm->n_buffer, p_data, n_data);
      p_shm->n_buffer += n_data;
    }
  }

  int32 WriteEnd(const struct timeval *p_tm) {
    TAG_SHM_DATA* p_shm = (TAG_SHM_DATA*)v_shm_.GetData();
    if (p_shm) {
      p_shm->c_tm_capture.tv_sec = p_tm->tv_sec;
      p_shm->c_tm_capture.tv_usec = p_tm->tv_usec;
    }
    v_sem_w_.Signal();
  }

 private:
  HANDLE  hdl_shm_;
  HANDLE  hdl_sem_w_;
  HANDLE  hdl_sem_r_;
};



void * Shm_Create(const char* s_key, unsigned int n_size) {

}
