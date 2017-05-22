/*
 * 描述：线程私有数据
 */

#include "vshm.h"
#include <cstdio>
#include <cstring>

#ifdef WIN32
#include <Windows.h>
#include <process.h>
#else
#include <stdio.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#endif // WIN32

namespace isaw {

VShm::VShm() {
  memset(shm_key_, 0, 32);
  shm_hdl_  = INVALID_SHM_HANDLE;
  shm_ptr_  = NULL;
  shm_size_ = 0;
}

VShm::~VShm() {
  Close();
}

#ifdef WIN32
int32_t VShm::Open(ShmKey key, ShmSize size) {
  if (!key || size == 0) {
    return -1;
  }

  shm_size_ = size;
  strncpy((char*)shm_key_, key, 31);

  return 0;
}
#else
int32_t VShm::Open(ShmKey key, ShmSize size) {
  if (!key || size == 0) {
    return -1;
  }

  shm_size_ = size;
  strncpy(shm_key_, key, 31);

  shm_hdl_ = shm_open(shm_key_, O_RDWR|O_CREAT, 0777);
  if(shm_hdl_ == INVALID_SHM_HANDLE) {
    perror("shm open failed.\n");
    return -2;
  }

  // 调整确定文件共享内存的空间
  int ret = ftruncate(shm_hdl_, shm_size_);
  if(-1 == ret)
  {
    perror("ftruncate faile: ");
    return -3;
  }

  // mmap
  shm_ptr_ = mmap(NULL, shm_size_,
                  PROT_READ | PROT_WRITE,
                  MAP_SHARED, shm_hdl_, SEEK_SET);
  if (shm_ptr_ == NULL) {
    perror("mmap failed.\n");
    return -4;
  }
  return 0;
}

void VShm::Close()
{
  if (shm_ptr_) {
    munmap(shm_ptr_, shm_size_);
    shm_ptr_ = NULL;
  }

  if (shm_hdl_) {
    shm_unlink(shm_key_);
    shm_hdl_ = INVALID_SHM_HANDLE;
  }
}
#endif

}