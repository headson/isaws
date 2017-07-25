/************************************************************************/
/* Author      : SoberPeng 2017-06-09
/* Description :
/************************************************************************/
#include "vzshm_c.h"

#include <stdio.h>
#include <string.h>

CShareVideo::CShareVideo()
  : hdl_sem_(NULL)
  , hdl_shm_(NULL)
  , mem_ptr_(NULL) {
}

CShareVideo::~CShareVideo() {
}

bool CShareVideo::Create(const char *s_path, unsigned int n_size) {
  bool ret = Open(s_path, n_size);
  if (ret == true) {
    // vzSemUnLock(hdl_sem_);

    ShmVdo *p_shm = (ShmVdo*)mem_ptr_;
    p_shm->n_w_sec = p_shm->n_w_usec = 0;
    p_shm->n_sps = p_shm->n_pps = 0;
    p_shm->n_data = 0;
    p_shm->n_size = n_size - sizeof(ShmVdo);
  }
  return ret;
}

bool CShareVideo::Open(const char *s_file, unsigned int n_size) {
  // ¹²ÏíÄÚ´æ
  hdl_shm_ = vzShmOpen(s_file, n_size);
  if (hdl_shm_ == HDL_NULL) {
    printf("shm open failed.");
    return false;
  }
  mem_ptr_ = vzShmAt(hdl_shm_);
  if (mem_ptr_ == NULL) {
    printf("shm memory point failed.\n");
    return false;
  }
  printf("shm handle 0x%x, 0x%x.\n", hdl_shm_, mem_ptr_);

  // ÐÅºÅ
  char s_sem_key[64] = {0};
  // W
  memset(s_sem_key, 0, 64);
  snprintf(s_sem_key, 63, "%s_sem", s_file);
  hdl_sem_ = vzSemOpen(s_sem_key);
  if (hdl_sem_ == HDL_NULL) {
    printf("sem open failed %s.", s_sem_key);
    return false;
  }
  printf("sem handle 0x%x.\n", hdl_sem_);
  return true;
}

unsigned int CShareVideo::GetWidth() {
  ShmVdo *p_shm = (ShmVdo*)mem_ptr_;
  if (p_shm) {
    p_shm->n_width;
  }
  return 0;
}

void CShareVideo::SetWidth(unsigned int w) {
  ShmVdo *p_shm = (ShmVdo*)mem_ptr_;
  if (p_shm) {
    p_shm->n_width = w;
  }
}

unsigned int CShareVideo::GetHeight() {
  ShmVdo *p_shm = (ShmVdo*)mem_ptr_;
  if (p_shm) {
    p_shm->n_height;
  }
  return 0;
}

void CShareVideo::SetHeight(unsigned int h) {
  ShmVdo *p_shm = (ShmVdo*)mem_ptr_;
  if (p_shm) {
    p_shm->n_height = h;
  }
}

int CShareVideo::SetSps(const char *p_data, unsigned int n_data) {
  ShmVdo *p_shm = (ShmVdo*)mem_ptr_;
  if (p_shm == NULL     ||
      p_shm->n_sps > 0  ||
      p_data == NULL    ||
      n_data > 1024) {
    printf("-------------------sps %d data .\n",
           p_shm->n_sps, n_data);
    return -1;
  }

  memcpy(p_shm->sps_pps, p_data, n_data);
  p_shm->n_sps = n_data;
  printf("------------------- sps length %d.\n", p_shm->n_sps);
  return n_data;
}

int CShareVideo::SetPps(const char *p_data, unsigned int n_data) {
  ShmVdo *p_shm = (ShmVdo*)mem_ptr_;
  if (p_shm == NULL ||
      p_shm->n_pps > 0 ||
      p_shm->n_sps < 0 ||
      p_data == NULL ||
      (n_data + p_shm->n_sps) > 1024) {
    return -1;
  }

  memcpy(p_shm->sps_pps+p_shm->n_sps, p_data, n_data);
  p_shm->n_pps = n_data;
  printf("------------------- pps length %d.\n", p_shm->n_pps);
  return n_data;
}

int CShareVideo::GetSpsPps(char* p_data, unsigned int n_data, int *n_sps, int *n_pps) {
  ShmVdo *p_shm = (ShmVdo*)mem_ptr_;
  if (p_shm == NULL ||
      p_data == NULL ||
      p_shm->n_sps <= 0 ||
      p_shm->n_pps <= 0 ||
      n_data < (p_shm->n_sps + p_shm->n_pps)) {
    return -1;
  }

  memcpy(p_data, p_shm->sps_pps, (p_shm->n_sps + p_shm->n_pps));
  *n_sps = p_shm->n_sps;
  *n_pps = p_shm->n_pps;
  return (p_shm->n_sps + p_shm->n_pps);
}

bool CShareVideo::Lock() {
  if (vzSemLock(hdl_sem_, -1) != 0) {
    return false;
  }
  return true;
}

bool CShareVideo::Unlock() {
  if (vzSemUnLock(hdl_sem_) != 0) {
    return false;
  }
  return true;
}

CShareVideo::ShmVdo * CShareVideo::GetData() {
  ShmVdo *p_shm = (ShmVdo*)mem_ptr_;
  if (p_shm) {
    return p_shm;
  }
  return NULL;
}
