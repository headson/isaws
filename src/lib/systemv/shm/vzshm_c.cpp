/************************************************************************/
/* Author      : SoberPeng 2017-06-09
/* Description :
/************************************************************************/
#include "vzshm_c.h"

#include <stdio.h>
#include <string.h>

#include "spsdecode.h"
#include "vzbase/helper/stdafx.h"

CShareBuffer::CShareBuffer()
  : hdl_shm_(NULL)
  , hdl_sem_(HDL_NULL) {
}

CShareBuffer::~CShareBuffer() {

}

bool CShareBuffer::Create(const char *s_path, unsigned int n_size) {
  bool ret = Open(s_path, n_size);
  if (ret == true) {
    vzSemUnLock(hdl_sem_);
  }

  TAG_SHM_VDO *p_shm = (TAG_SHM_VDO*)mem_ptr_;
  p_shm->w_sec = p_shm->w_usec = 0;
  p_shm->head_1 = p_shm->head_2 = 0;
  p_shm->ndata = 0;
  p_shm->nsize = n_size - sizeof(TAG_SHM_VDO);
  return ret;
}

bool CShareBuffer::Open(const char *s_file, unsigned int n_size) {
  // 共享内存
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

  // 信号
  char s_sem_key[64] = {0};
  // W
  memset(s_sem_key, 0, 64);
  snprintf(s_sem_key, 63, "%s_sem_w", s_file);
  hdl_sem_ = vzSemOpen(s_sem_key);
  if (hdl_sem_ == HDL_NULL) {
    printf("sem open failed %s.", s_sem_key);
    return false;
  }
  printf("sem w handle 0x%x.\n", hdl_sem_);
  return true;
}

int CShareBuffer::ReadHead(char* p_data, unsigned int n_data, int *n_head_1, int *n_head_2) {
  TAG_SHM_VDO *p_shm = (TAG_SHM_VDO*)mem_ptr_;
  if (p_shm == NULL ||
      p_data == NULL ||
      p_shm->head_1 <= 0 ||
      p_shm->head_2 <= 0 ||
      n_data < (p_shm->head_1+p_shm->head_2)) {
    return -1;
  }

  memcpy(p_data, p_shm->shead, (p_shm->head_1+p_shm->head_2));
  *n_head_1 = p_shm->head_1;
  *n_head_2 = p_shm->head_2;

  return (p_shm->head_1+p_shm->head_2);
}

int CShareBuffer::Read(char* p_data, unsigned int n_data,
                       unsigned int *n_sec, unsigned int *n_usec) {
  if (p_data == NULL || n_data == 0 ||
      n_sec == NULL || n_usec == NULL) {
    printf("param is error.");
    return -1;
  }
  if (hdl_shm_ == HDL_NULL ||
      mem_ptr_ == NULL ||
      hdl_sem_ == HDL_NULL) {
    printf("shm or sem is invalid.");
    return -2;
  }

  TAG_SHM_VDO *p_shm = (TAG_SHM_VDO*)mem_ptr_;
  if (!p_shm || n_data < p_shm->ndata) { // 读优先
    return -3;
  }
  //if (vzSemLock(hdl_sem_w_, -1) != 0) {
  //  return -4;
  //}
  if (*n_sec == p_shm->w_sec
      && *n_usec == p_shm->w_usec) {
    return 0;
  }

  n_data = p_shm->ndata;
  memcpy(p_data, p_shm->pdata, n_data);

  *n_sec = p_shm->w_sec;
  *n_usec = p_shm->w_usec;
  //vzSemUnLock(hdl_sem_w_);
  return n_data;
}

int CShareBuffer::ReadH264(char* p_data, unsigned int n_data,
                           unsigned int *n_sec, unsigned int *n_usec) {
  if (p_data == NULL || n_data == 0 ||
      n_sec == NULL || n_usec == NULL) {
    printf("param is error.");
    return -1;
  }
  if (hdl_shm_ == HDL_NULL ||
      mem_ptr_ == NULL ||
      hdl_sem_ == HDL_NULL) {
    printf("shm or sem is invalid.");
    return -2;
  }

  TAG_SHM_VDO *p_shm = (TAG_SHM_VDO*)mem_ptr_;
  if (!p_shm || 
      n_data < (p_shm->ndata+p_shm->head_1+p_shm->head_2)) { // 读优先
    return -3;
  }
  if (*n_sec == p_shm->w_sec
      && *n_usec == p_shm->w_usec) {
    return 0;
  }

  if (vzSemLock(hdl_sem_, -1) != 0) {
    return -4;
  }

  n_data = 0;

  int nal_bng = 0;
  int frm_type = 1;
  nal_split((char*)p_shm->pdata, p_shm->ndata,
            &frm_type, &nal_bng);

  // 如果是I帧,需要把SPS PPS付在帧开始处
  if (frm_type == 5) {
    memcpy(p_data, p_shm->shead, p_shm->head_1+p_shm->head_2);
    n_data = p_shm->head_1+p_shm->head_2;
  }

  memcpy(p_data+n_data, p_shm->pdata, p_shm->ndata);
  n_data += p_shm->ndata;

  *n_sec = p_shm->w_sec;
  *n_usec = p_shm->w_usec;
  vzSemUnLock(hdl_sem_);
  return n_data;
}

int CShareBuffer::WriteSps(const char *p_data, unsigned int n_data) {
  TAG_SHM_VDO *p_shm = (TAG_SHM_VDO*)mem_ptr_;
  if (p_shm == NULL     ||
      p_shm->head_1 > 0  ||
      p_data == NULL    ||
      n_data > 128) {
    return -1;
  }

  memcpy(p_shm->shead, p_data, n_data);
  p_shm->head_1 = n_data;
  printf("------------------- sps length %d.\n", p_shm->head_1);
  return n_data;
}

int CShareBuffer::WritePps(const char *p_data, unsigned int n_data) {
  TAG_SHM_VDO *p_shm = (TAG_SHM_VDO*)mem_ptr_;
  if (p_shm == NULL ||
      p_shm->head_2 > 0 ||
      p_shm->head_1 < 0 ||
      p_data == NULL ||
      (n_data + p_shm->head_1) > 128) {
    return -1;
  }

  memcpy(p_shm->shead+p_shm->head_1, p_data, n_data);
  p_shm->head_2 = n_data;
  printf("------------------- pps length %d.\n", p_shm->head_2);
  return n_data;
}

int CShareBuffer::Write(const char *p_data, unsigned int n_data,
                        unsigned int n_sec, unsigned int n_usec) {
  if (p_data == NULL || n_data == 0) {
    printf("param is error.\n");
    return -1;
  }
  if (hdl_shm_ == HDL_NULL ||
      mem_ptr_ == NULL ||
      hdl_sem_ == HDL_NULL) {
    printf("shm or sem is invalid.");
    return -2;
  }

  TAG_SHM_VDO *p_shm = (TAG_SHM_VDO*)mem_ptr_;
  if (!p_shm || n_data > p_shm->nsize) {
    printf("shm or sem is small. %d-%d\n",
           n_data, p_shm->nsize);
    return -3;
  }

  if (vzSemLock(hdl_sem_, -1) != 0) {
    printf("lock failed.");
    return -4;
  }

  memcpy(p_shm->pdata, p_data, n_data);
  p_shm->w_sec = n_sec;
  p_shm->w_usec = n_usec;
  p_shm->ndata = n_data;

  vzSemUnLock(hdl_sem_);
  return p_shm->ndata;
}


void CShareBuffer::GetVdoSize(unsigned int *w, unsigned int *h) {
  TAG_SHM_VDO *p_shm = (TAG_SHM_VDO*)mem_ptr_;
  if (p_shm) {
    *w = p_shm->width;
    *h = p_shm->height;
  }
}

void CShareBuffer::SetWidth(unsigned int w) {
  TAG_SHM_VDO *p_shm = (TAG_SHM_VDO*)mem_ptr_;
  if (p_shm) {
    p_shm->width = w;
  }
}

void CShareBuffer::SetHeight(unsigned int h) {
  TAG_SHM_VDO *p_shm = (TAG_SHM_VDO*)mem_ptr_;
  if (p_shm) {
    p_shm->height = h;
  }
}
