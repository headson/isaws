#include "vshm.h"

#include <stdio.h>

namespace base {

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
  int32_t VShm::Open(const ShmKey* key, ShmSize size) {
    if (!key || size == 0) {
      return -1;
    }
    strncpy((char*)shm_key_, key, 31);

    // open
    shm_hdl_ = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, shm_key_);
    if (shm_hdl_ == SHM_NULL) {
      shm_hdl_ = ::CreateFileMapping(INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        size + 1,
        shm_key_);
    }
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

    if (shm_hdl_ != SHM_NULL) {
      if (FALSE == ::CloseHandle(shm_hdl_)) {
        printf("CloseHandle %s failed.\n", shm_key_);
      }
      shm_hdl_ = SHM_NULL;
    }
  }

#else  // WIN32

  int32_t VShm::Open(const ShmKey* key, ShmSize size) {
    if (!key || size == 0) {
      return -1;
    }
    strncpy(shm_key_, key, 31);

    // open
    shm_hdl_ = shm_open(shm_key_, O_RDWR | O_CREAT, 0777);
    if (shm_hdl_ == SHM_NULL) {
      printf("shm_open %s failed.\n", shm_key_);
      return -2;
    }

    int ret = ftruncate(shm_hdl_, size);
    if (-1 == ret) {
      printf("ftruncate %s failed.\n", shm_key_);
      return -3;
    }

    // mmap
    shm_ptr_ = mmap(NULL, size,
      PROT_READ | PROT_WRITE,
      MAP_SHARED, shm_hdl_, SEEK_SET);
    if (shm_ptr_ == NULL) {
      printf("mmap %s failed.\n", shm_key_);
      return -4;
    }
    shm_size_ = size;
    return 0;
  }

  void VShm::Close() {
    if (shm_ptr_) {
      if (0 != munmap(shm_ptr_, shm_size_)) {
        printf("munmap %s failed.\n", shm_key_);
      }
      shm_ptr_ = NULL;
    }

    if (shm_hdl_ != SHM_NULL) {
      if (0 != shm_unlink(shm_key_)) {
        printf("shm_unlink %s failed %d.\n", shm_key_, shm_hdl_);
      }
      shm_hdl_ = SHM_NULL;
    }
  }
#endif  // WIN32

}  // namespace base
