/************************************************************************/
/* Author      : SoberPeng 2017-06-09
/* Description :
/************************************************************************/
#include "vzshm_c.h"

#include <stdio.h>
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

  int Create(const char *s_path, unsigned int n_size) {
    int n_ret = Open(s_path, n_size);
    if (n_ret == 0) {
      vzSemUnLock(hdl_sem_r_);
      vzSemUnLock(hdl_sem_w_);
    }
    return n_ret;
  }

  int Open(const char *s_file, unsigned int n_size) {
    // 共享内存
    hdl_shm_ = vzShmOpen(s_file, n_size);
    if (hdl_shm_ == HDL_NULL) {
      printf("shm open failed.");
      return -1;
    }
    p_mem_ptr_ = vzShmAt(hdl_shm_);
    if (p_mem_ptr_ == NULL) {
      printf("shm memory point failed.\n");
      return -2;
    }
    TAG_SHM *p_shm = (TAG_SHM*)p_mem_ptr_;
    p_shm->n_size = n_size;
    p_shm->n_data = 0;
    p_shm->n_w_sec = p_shm->n_w_usec = 0;
    printf("shm handle 0x%x, 0x%x.\n", hdl_shm_, p_mem_ptr_);

    // 信号
    char s_sem_key[64] = { 0 };
    // W
    memset(s_sem_key, 0, 64);
    snprintf(s_sem_key, 63, "%s_sem_w", s_file);
    hdl_sem_w_ = vzSemOpen(s_sem_key);
    if (hdl_sem_w_ == HDL_NULL) {
      printf("sem open failed %s.", s_sem_key);
      return -3;
    }
    printf("sem w handle 0x%x.\n", hdl_sem_w_);
    // R
    memset(s_sem_key, 0, 64);
    snprintf(s_sem_key, 63, "%s_sem_r", s_file);
    hdl_sem_r_ = vzSemOpen(s_sem_key);
    if (hdl_sem_r_ == HDL_NULL) {
      printf("sem open failed %s.", s_sem_key);
      return -4;
    }
    printf("sem r handle 0x%x.\n", hdl_sem_r_);
    return 0;
  }

  int Read(char* p_data, unsigned int n_data,
           unsigned int *n_sec, unsigned int *n_usec) {
    if (p_data == NULL || n_data == 0 ||
        n_sec == NULL || n_usec == NULL) {
      printf("param is error.");
      return -1;
    }
    if (hdl_shm_ == HDL_NULL ||
        p_mem_ptr_ == NULL    ||
        hdl_sem_r_ == HDL_NULL ||
        hdl_sem_w_ == HDL_NULL) {
      printf("shm or sem is invalid.");
      return -2;
    }

    TAG_SHM *p_shm = (TAG_SHM*)p_mem_ptr_;
    if (!p_shm || n_data < p_shm->n_data) { // 读优先
      return -3;
    }
    if (vzSemLock(hdl_sem_w_, -1) != 0) {
      return -4;
    }
    if (*n_sec == p_shm->n_w_sec
        && *n_usec == p_shm->n_w_usec) {
      vzSemUnLock(hdl_sem_w_);
      return 0;
    }

    n_data = p_shm->n_data;
    memcpy(p_data, p_shm->p_data, n_data);
    //printf("read data length %d.\n", n_data);

    *n_sec = p_shm->n_w_sec;
    *n_usec = p_shm->n_w_usec;
    vzSemUnLock(hdl_sem_w_);
    return n_data;
  }

  int Write(const char  *p_data, unsigned int n_data,
            unsigned int n_sec, unsigned int n_usec) {
    if (p_data == NULL || n_data == 0) {
      printf("param is error.\n");
      return -1;
    }
    if (hdl_shm_ == HDL_NULL ||
        p_mem_ptr_ == NULL    ||
        hdl_sem_r_ == HDL_NULL ||
        hdl_sem_w_ == HDL_NULL) {
      printf("shm or sem is invalid.");
      return -2;
    }

    TAG_SHM *p_shm = (TAG_SHM*)p_mem_ptr_;
    if (!p_shm || n_data > p_shm->n_size) {
      return -3;
    }

    if (vzSemLock(hdl_sem_w_, -1) != 0) {
      return -4;
    }

    memcpy(p_shm->p_data, p_data, p_shm->n_data);
    p_shm->n_w_sec  = n_sec;
    p_shm->n_w_usec = n_usec;
    p_shm->n_data   = n_data;

    vzSemUnLock(hdl_sem_w_);
    return p_shm->n_data;
  }

///分批写入buffer[h264: head + body]///////////////////////////////////////
  int WriteBegin() {
    if (hdl_shm_ == HDL_NULL ||
        p_mem_ptr_ == NULL    ||
        hdl_sem_r_ == HDL_NULL ||
        hdl_sem_w_ == HDL_NULL) {
      printf("shm or sem is invalid.");
      return -1;
    }

    if (vzSemLock(hdl_sem_w_, -1) == 0) {
      return 0;
    }
    return -2;
  }

  int WriteOffset(const char *p_data, unsigned int n_data, unsigned int n_offset) {
    if (p_data == NULL || n_data == 0) {
      printf("param is error.");
      return -1;
    }
    if (hdl_shm_ == HDL_NULL ||
        p_mem_ptr_ == NULL    ||
        hdl_sem_r_ == HDL_NULL ||
        hdl_sem_w_ == HDL_NULL) {
      printf("shm or sem is invalid.");
      return -2;
    }

    TAG_SHM *p_shm = (TAG_SHM*)p_mem_ptr_;
    if (p_shm && (n_data+n_offset) <= p_shm->n_size) { // 读优先
      memcpy(p_shm->p_data+n_offset, p_data, n_data);
      p_shm->n_data = n_offset + n_data;
      return n_data;
    }
    return -3;
  }

  int WriteEnd(unsigned int n_sec, unsigned int n_usec) {
    if (hdl_shm_ == HDL_NULL ||
        p_mem_ptr_ == NULL    ||
        hdl_sem_r_ == HDL_NULL ||
        hdl_sem_w_ == HDL_NULL) {
      printf("shm or sem is invalid.");
      return -1;
    }

    ((TAG_SHM*)p_mem_ptr_)->n_w_sec  = n_sec;
    ((TAG_SHM*)p_mem_ptr_)->n_w_usec = n_usec;
    if (vzSemUnLock(hdl_sem_w_) == 0) {
      return 0;
    }
    return -2;
  }

 private:
  HANDLE        hdl_shm_;
  void         *p_mem_ptr_;

  unsigned int  n_r_sec_;
  unsigned int  n_r_usec_;

  HANDLE        hdl_sem_w_;
  HANDLE        hdl_sem_r_;
};

///C API//////////////////////////////////////////////////////////////////
void *Shm_Create(const char* s_key, unsigned int n_size) {
  CShareMemory* p_shm = NULL;

  p_shm = new CShareMemory();
  if (p_shm) {
    if (p_shm->Create(s_key, n_size) != 0) {
      delete p_shm;
      p_shm = NULL;
    }
  }
  return p_shm;
}

void Shm_Release(void *p_hdl) {
  if (p_hdl) {
    delete (CShareMemory*)p_hdl;
  }
}

int Shm_Write(void *p_hdl,
              const char* p_data, unsigned int n_data,
              unsigned int n_sec, unsigned int n_usec) {
  CShareMemory *p_shm = NULL;
  p_shm = (CShareMemory*)p_hdl;
  if (p_shm) {
    return p_shm->Write(p_data, n_data, n_sec, n_usec);
  }
  printf("shm write failed.");
  return -4;
}

int Shm_Read(void *p_hdl,
             char *p_data, unsigned int n_data,
             unsigned int *n_sec, unsigned int *n_usec) {
  CShareMemory *p_shm = NULL;
  p_shm = (CShareMemory*)p_hdl;
  if (p_shm) {
    return p_shm->Read(p_data, n_data, n_sec, n_usec);
  }
  printf("shm read failed.");
  return -4;
}

int Shm_W_Begin(void *p_hdl) {
  CShareMemory *p_shm = NULL;
  p_shm = (CShareMemory*)p_hdl;
  if (p_shm) {
    return p_shm->WriteBegin();
  }
  printf("shm w begin failed.");
  return -3;
}

int Shm_W_Write(void *p_hdl, const char* p_data, unsigned int n_data, unsigned int n_offset) {
  CShareMemory *p_shm = NULL;
  p_shm = (CShareMemory*)p_hdl;
  if (p_shm) {
    return p_shm->WriteOffset(p_data, n_data, n_offset);
  }
  printf("shm w offet failed.");
  return -4;
}

int Shm_W_End(void *p_hdl, unsigned int n_sec, unsigned int n_usec) {
  CShareMemory *p_shm = NULL;
  p_shm = (CShareMemory*)p_hdl;
  if (p_shm) {
    return p_shm->WriteEnd(n_sec, n_usec);
  }
  printf("shm w end failed.");
  return -3;
}
