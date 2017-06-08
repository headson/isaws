
#include "vshmvideo.h"

VShmVideo::VShmVideo() {

}

VShmVideo::~VShmVideo() {
  v_shm_.Close();
  v_sem_w_.Close();
  v_sem_r_.Close();
}

int32_t VShmVideo::Open(const int8_t *p_name, uint32_t n_shm_size) {
  int32_t n_ret = 0;
  int8_t  s_name[32];

  // 共享内存
  n_ret = v_shm_.Open((ShmKey*)p_name, n_shm_size);
  if (n_ret != 0) {
    n_ret = v_shm_.Create((ShmKey*)p_name, n_shm_size);
    if (n_ret == 0) {  // 创建,初始化值;打开不初始化
      TAG_SHM_VIDEO* p_shm = (TAG_SHM_VIDEO*)v_shm_.GetData();
      if (p_shm) {
        p_shm->n_video = 0;
        p_shm->n_read_count = 0;
      }
    }
  }
  if (n_ret != 0) {
    printf("shm open failed.%d.", n_ret);
    return n_ret;
  }

  memset(s_name, 0, 32);
  sprintf((char*)s_name, "%s_w", p_name);
  n_ret = v_sem_w_.Open(1, 1, (int8_t*)s_name);
  if (n_ret != 0) {
    printf("sem open failed.%d.\n", n_ret);
    return n_ret;
  }

  memset(s_name, 0, 32);
  sprintf((char*)s_name, "%s_r", p_name);
  n_ret = v_sem_r_.Open(1, 1, (int8_t*)s_name);
  if (n_ret != 0) {
    printf("sem open failed.%d.\n", n_ret);
    return n_ret;
  }
  return 0;
}

int32_t VShmVideo::Read(int8_t* p_data, uint32_t n_data, struct timeval* p_tm) {
  if (p_data == NULL || n_data <= 0 || p_tm == NULL){
    printf("param is error.\n");
    return -1;
  }
  if (!v_shm_.IsValid() || !v_sem_w_.IsValid()) {
    printf("shm or sem is invalid.\n");
    return -1;
  }

  TAG_SHM_VIDEO* p_shm = (TAG_SHM_VIDEO*)v_shm_.GetData();
  if (p_shm) {
    v_sem_r_.Wait();
    p_shm->n_read_count++;
    if (p_shm->n_read_count == 1)
      v_sem_w_.Wait();
    v_sem_r_.Release();

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
      v_sem_w_.Release();
    v_sem_r_.Release();

    //v_sem_w_.Wait();
    //int n_size = 0;
    //if (p_shm->n_read_count == 1) {
    //  n_size = n_data > p_shm->n_video ? p_shm->n_video : n_data;
    //  memcpy(p_data, p_shm->p_video, n_size);
    //  p_shm->n_read_count = 0;
    //}
    //v_sem_w_.Release();
    return n_size;
  }
  return 0;
}

int32_t VShmVideo::Write(const int8_t* p_data, uint32_t n_data, const struct timeval* p_tm) {
  if (p_data == NULL || n_data <= 0 || p_tm == NULL){
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

    static int i = 0;
    printf("read video %d \t %d.\n", i++, n_data);
  }

  v_sem_w_.Release();
  return 0;
}

