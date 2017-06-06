#include "vshm.h"

#include <stdio.h>
#include <errno.h>

VShm::VShm() {
  printf("%s[%d].\n", __FUNCTION__, __LINE__);
  memset(shm_key_, 0, 32);
  shm_hdl_ = SHM_NULL;
  shm_ptr_ = NULL;
  shm_size_ = 0;
}

VShm::~VShm() {
  Close();
  printf("%s[%d].\n", __FUNCTION__, __LINE__);
}

#ifdef WIN32
int32_t VShm::Create(const ShmKey* key, ShmSize size) {
  if (!key || size == 0) {
    return -1;
  }
  strncpy((char*)shm_key_, key, 31);

  // open
  shm_hdl_ = ::CreateFileMapping(INVALID_HANDLE_VALUE,
                                 NULL,
                                 PAGE_READWRITE,
                                 0,
                                 size + 1,
                                 shm_key_);
  if (shm_hdl_ == SHM_NULL) {
    printf("CreateFileMapping %s failed.%d.\n", shm_key_, GetLastError());
    return -2;
  }

  // mmap
  shm_ptr_ = ::MapViewOfFile(shm_hdl_, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  if (shm_ptr_ == NULL) {
    printf("MapViewOfFile %s failed.\n", shm_key_);
    return -4;
  }
  shm_size_ = size;
  return 0;
}

int32_t VShm::Open(const ShmKey* key, ShmSize size) {
  if (!key || size == 0) {
    return -1;
  }
  strncpy((char*)shm_key_, key, 31);

  // open
  shm_hdl_ = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, shm_key_);
  if (shm_hdl_ == SHM_NULL) {
    printf("OpenFileMapping %s failed.\n", shm_key_);
    return -2;
  }

  // mmap
  shm_ptr_ = ::MapViewOfFile(shm_hdl_, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  if (shm_ptr_ == NULL) {
    printf("MapViewOfFile %s failed.\n", shm_key_);
    return -4;
  }
  shm_size_ = size;
  return 0;
}

void VShm::Close() {
  if (shm_ptr_) {
    if (FALSE == ::UnmapViewOfFile(shm_ptr_)) {
      printf("UnmapViewOfFile %s failed.\n", shm_key_);
    }
    shm_ptr_ = NULL;
  }

  //if (shm_hdl_ != SHM_NULL) {
  //  if (FALSE == ::CloseHandle(shm_hdl_)) {
  //    printf("CloseHandle %s failed.\n", shm_key_);
  //  }
  //  shm_hdl_ = SHM_NULL;
  //}
}

#else  // WIN32

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int32_t VShm::Create(const ShmKey* key, ShmSize size) {
  if (!key || size == 0) {
    return -1;
  }
  shm_size_ = size;
  strncpy(shm_key_, key, 31);

  key_t n_key = ftok(key, 0xAA55AA);
  if (n_key != -1) {
    VZ_ERROR("ftok new key failed.\n");
    return -1;
  }

  shm_hdl_ = shmget(n_key, size, IPC_CREAT | 0660);
  VZ_PRINT("shared memory id:%d\n", shm_hdl_);
  if(shm_ptr_ < 0) {
    VZ_ERROR("shared memory open failed\n");
    return -1;
  }

  shm_ptr_ = shmat(shm_hdl_, 0, 0);
  if (shm_ptr_ == NULL) {
    VZ_ERROR("shmat %s failed.\n", shm_key_);
    return -4;
  }
  return 0;
}

int32_t VShm::Open(ShmKey key, ShmSize size) {
  if (!key || size == 0) {
    return -1;
  }
  shm_size_ = size;
  strncpy(shm_key_, key, 31);

  key_t n_key = ftok(key, 0xAA55AA);
  if (n_key != -1) {
    VZ_ERROR("ftok new key failed.\n");
    return -1;
  }

  shm_hdl_ = shmget(n_key, 0, 0);
  VZ_PRINT("shared memory id:%d\n", shm_hdl_);
  if(shm_ptr_ < 0) {
    VZ_ERROR("shared memory open failed\n");
    return -1;
  }

  shm_ptr_ = shmat(shm_hdl_, 0, 0);
  if (shm_ptr_ == NULL) {
    VZ_ERROR("shmat %s failed.\n", shm_key_);
    return -4;
  }
  return 0;
}

void VShm::Close() {
  if (shm_ptr_) {
    shmdt(shm_ptr_);
    shm_ptr_ = NULL;
  }
}

#endif  // WIN32

