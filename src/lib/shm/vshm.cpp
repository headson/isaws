#include "vshm.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

<<<<<<< HEAD
#include "stdafx.h"
=======
#include "base/stdafx.h"
>>>>>>> 8c471fd87910ea7f532ac1fc43711c32142c523a

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
int32 VShm::Open(const ShmKey key, ShmSize size) {
  if (!key || size == 0) {
    return -1;
  }

  // open
  shm_hdl_ = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, (LPCSTR)key);
  if (shm_hdl_ == SHM_NULL) {
    // open
    shm_hdl_ = ::CreateFileMapping(INVALID_HANDLE_VALUE,
                                   NULL,
                                   PAGE_READWRITE,
                                   0,
                                   size + 1,
                                   (LPCSTR)key);
  }
  if (shm_hdl_ == SHM_NULL) {
    LOG_ERROR("CreateFileMapping %s failed.%d.\n", key, GetLastError());
    return -2;
  }
  LOG_INFO("shared memory id:0x%x\n", shm_hdl_);

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
      LOG_ERROR("UnmapViewOfFile 0x%x failed.\n", (uint32)shm_hdl_);
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

int32 VShm::Open(const ShmKey key, ShmSize size) {
  if (key == 0 || size == 0) {
    return -1;
  }

  key_t k = ftok(key, 0);
  if (k < 0) {
    LOG_ERROR("ftok failed.\n");
    return -1;
  }

  shm_hdl_ = shmget(k, 0, 0);
  if(shm_hdl_ < 0) {
    shm_hdl_ = shmget(k, size, IPC_CREAT | 0660);
  }
  if(shm_hdl_ < 0) {
    LOG_ERROR("shared memory open failed %s-0x%x.\n", key, k);
    return -1;
  }
  LOG_INFO("shared memory id:%d %s 0x%x\n", shm_hdl_, key, k);

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

