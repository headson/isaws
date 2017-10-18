/************************************************************************
*Author      : Sober.Peng 17-10-18
*Description :
************************************************************************/
#include "vshm.h"
#include <stdio.h>

namespace vzbase {

#ifdef _WIN32
bool VShm::Open(const char* name, int size) {
  shm_id_ = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, (LPCSTR)name);
  if (shm_id_ == NULL) {
    shm_id_ = ::CreateFileMapping(INVALID_HANDLE_VALUE,
                                  NULL,
                                  PAGE_READWRITE,
                                  0,
                                  size,
                                  (LPCSTR)name);
  }
  if (shm_id_ == NULL) {
    perror("OpenFileMapping failed.\n");
    return false;
  }
  printf("shm_id = 0x%x\n", shm_id_);

  shm_ptr_ = vzShmAt();
  if (shm_ptr_ != NULL) {
    shm_size_ = size;
    return true;
  }

  shm_id_ = HDL_NULL;
  return false;
}

void *VShm::vzShmAt() {
  if (shm_id_ == HDL_NULL) {
    return NULL;
  }
  void *ptr = ::MapViewOfFile(shm_id_, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  if (ptr == NULL) {
    printf("MapViewOfFile failed.\n");
    return NULL;
  }
  return ptr;
}

void VShm::vzShmDt(void *ptr) {
  if (ptr) {
    if (FALSE == ::UnmapViewOfFile(ptr)) {
      printf("UnmapViewOfFile failed.\n");
    }
    ptr = NULL;
  }
}

#else

int VShm::create_file(const char *filename) {
  if (access(filename, 0) == 0) {
    return 0;
  }

  if (creat(filename, 0755) < 0) {
    printf("create file %s failure!\n", filename);
    return -1;
  }

  printf("create file %s success!\n", filename);
  return 0;
}

bool VShm::Open(const char* name, int size) {
  create_file(name);
  key_t k = ftok(name, 1);
  if (k < 0) {
    return false;
  }

  shm_id_ = shmget(k, size, IPC_CREAT | 0660);
  if (shm_id_ < 0) {
    shm_id_ = shmget(k, 0, 0);
  }
  if (shm_id_ < 0) {
    return false;
  }

  printf("shm_id = %d, %d\n", shm_id_, k);
  shm_ptr_ = vzShmAt();
  if (shm_ptr_ != NULL) {
    shm_size_ = size;
    return true;
  }

  shm_id_ = HDL_NULL;
  return false;
}

void *VShm::vzShmAt() {
  if (shm_id_ == HDL_NULL) {
    return NULL;
  }
  void *ptr = shmat(shm_id_, 0, 0);
  if (ptr == NULL) {
    printf("shmat failed.\n");
    return NULL;
  }
  return ptr;
}

void VShm::vzShmDt(void *ptr) {
  if (ptr) {
    shmdt(ptr);
    ptr = NULL;
  }
}
#endif

int VShm::GetData(void *pdata, int ndata) {
  void *pshm = vzShmAt();
  if (pshm) {
    int nn = shm_size_ > ndata ? ndata : shm_size_;
    memcpy(pdata, pshm, nn);
    vzShmDt(pshm);
    return nn;
  }
  return -1;
}

int VShm::SetData(const void *pdata, int ndata) {
  void *pshm = vzShmAt();
  if (pshm) {
    int nn = shm_size_ > ndata ? ndata : shm_size_;
    memcpy(pshm, pdata, nn);
    vzShmDt(pshm);
    return nn;
  }
  return -1;
}

}


