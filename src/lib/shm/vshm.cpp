#include "vshm.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "inc/vdefine.h"

VShm::VShm() {
  printf("%s[%d].\n", __FUNCTION__, __LINE__);
  shm_hdl_ = SHM_NULL;
  shm_ptr_ = NULL;
  shm_size_ = 0;
}

VShm::~VShm() {
  Close();
  printf("%s[%d].\n", __FUNCTION__, __LINE__);
}

#ifdef WIN32
int32_t VShm::Create(const ShmKey key, ShmSize size) {
  if (!key || size == 0) {
    return -1;
  }

  // open
  shm_hdl_ = ::CreateFileMapping(INVALID_HANDLE_VALUE,
                                 NULL,
                                 PAGE_READWRITE,
                                 0,
                                 size + 1,
                                 key);
  if (shm_hdl_ == SHM_NULL) {
    LOG_ERROR("CreateFileMapping %s failed.%d.\n", key, GetLastError());
    return -2;
  }

  // mmap
  shm_ptr_ = ::MapViewOfFile(shm_hdl_, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  if (shm_ptr_ == NULL) {
    LOG_ERROR("MapViewOfFile %s failed.\n", key);
    return -4;
  }
  shm_size_ = size;
  return 0;
}

int32_t VShm::Open(const ShmKey key, ShmSize size) {
  if (!key || size == 0) {
    return -1;
  }

  // open
  shm_hdl_ = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, key);
  if (shm_hdl_ == SHM_NULL) {
    LOG_ERROR("OpenFileMapping %s failed.\n", key);
    return -2;
  }

  // mmap
  shm_ptr_ = ::MapViewOfFile(shm_hdl_, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  if (shm_ptr_ == NULL) {
    LOG_ERROR("MapViewOfFile %s failed.\n", key);
    return -4;
  }
  shm_size_ = size;
  return 0;
}

void VShm::Close() {
  if (shm_ptr_) {
    if (FALSE == ::UnmapViewOfFile(shm_ptr_)) {
      LOG_ERROR("UnmapViewOfFile 0x%x failed.\n", (uint32_t)shm_hdl_);
    }
    shm_ptr_ = NULL;
  }

  //if (shm_hdl_ != SHM_NULL) {
  //  if (FALSE == ::CloseHandle(shm_hdl_)) {
  //    LOG_ERROR("CloseHandle %s failed.\n", shm_key_);
  //  }
  //  shm_hdl_ = SHM_NULL;
  //}
}

#else  // WIN32

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int32_t VShm::Create(const ShmKey key, ShmSize size) {
  if (key == 0 || size == 0) {
    return -1;
  }

  shm_hdl_ = shmget(key, size, IPC_CREAT | 0660);
  LOG_INFO("shared memory id:%d\n", shm_hdl_);
  if(shm_ptr_ < 0) {
    LOG_ERROR("shared memory open failed\n");
    return -1;
  }

  shm_ptr_ = shmat(shm_hdl_, 0, 0);
  if (shm_ptr_ == NULL) {
    LOG_ERROR("shmat %d failed.\n", key);
    return -4;
  }
  shm_size_ = size;
  return 0;
}

int32_t VShm::Open(const ShmKey key, ShmSize size) {
  if (key == 0 || size == 0) {
    return -1;
  }

  shm_hdl_ = shmget(key, 0, 0);
  LOG_INFO("shared memory id:%d\n", shm_hdl_);
  if(shm_ptr_ < 0) {
    LOG_ERROR("shared memory open failed\n");
    return -1;
  }

  shm_ptr_ = shmat(shm_hdl_, 0, 0);
  if (shm_ptr_ == NULL) {
    LOG_ERROR("shmat %s failed.\n", key);
    return -4;
  }
  shm_size_ = size;
  return 0;
}

void VShm::Close() {
  if (shm_ptr_) {
    shmdt(shm_ptr_);
    shm_ptr_ = NULL;
  }
}

#endif  // WIN32

