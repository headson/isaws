/************************************************************************/
/* Copyright@ 2008 vzenith.com                                          */
/* All rights reserved.                                                 */
/* -------------------------------------------------------------------- */
/* Author      : Sober.Peng                                             */
/* Date        : 19:5:2017                                              */
/* Description : 通过共享内存实现进程间日志参数传递                       */
/************************************************************************/
#include "vzlogging/base/vzshmarg.h"

#include <stdio.h>
#include <string.h>

#ifdef WIN32
#include <process.h>
#else  // WIN32
#include <sys/mman.h>
#include <sys/stat.h>        /* For mode constants */
#include <fcntl.h>           /* For O_* constants */
#include <unistd.h>
#endif  // WIN32

namespace vzlogging {

/**VShm******************************************************************/
VShm::VShm() {
  //VZ_PRINT("%s[%d].\n", __FUNCTION__, __LINE__);
  memset(shm_key_, 0, 32);
  shm_hdl_  = SHM_NULL;
  shm_ptr_  = NULL;
  shm_size_ = 0;
}

VShm::~VShm() {
  Close();
  //VZ_PRINT("%s[%d].\n", __FUNCTION__, __LINE__);
}

#ifdef WIN32
int VShm::Open(ShmKey key, ShmSize size) {
  if (!key || size == 0) {
    return -1;
  }
  shm_size_ = size;
  strncpy(shm_key_, key, 31);

  // open
  shm_hdl_ = ::OpenFileMapping(FILE_MAP_ALL_ACCESS, 0, shm_key_);
  if (shm_hdl_ == SHM_NULL) {
    shm_hdl_ = ::CreateFileMapping(INVALID_HANDLE_VALUE,
                                   NULL,
                                   PAGE_READWRITE,
                                   0,
                                   size+1,
                                   shm_key_);
  }
  if (shm_hdl_ == SHM_NULL) {
    VZ_ERROR("OpenFileMapping %s failed.\n", shm_key_);
    return -2;
  }

  // mmap
  shm_ptr_ = ::MapViewOfFile(shm_hdl_, FILE_MAP_ALL_ACCESS, 0, 0, 0);
  if (shm_ptr_ == NULL) {
    VZ_ERROR("MapViewOfFile %s failed.\n", shm_key_);
    return -4;
  }
  return 0;
}

void VShm::Close() {
  if (shm_ptr_) {
    if (FALSE == ::UnmapViewOfFile(shm_ptr_)) {
      VZ_ERROR("UnmapViewOfFile %s failed.\n", shm_key_);
    }
    shm_ptr_ = NULL;
  }

  // 不释放,其他应用还在使用
  // if (shm_hdl_ != SHM_NULL) {
  //   if (FALSE == ::CloseHandle(shm_hdl_)) {
  //     VZ_PRINT("CloseHandle %s failed.\n", shm_key_);
  //   }
  //   shm_hdl_ = SHM_NULL;
  // }
}

#else  // WIN32

#if 0
int32 VShm::Open(ShmKey key, ShmSize size) {
  if (!key || size == 0) {
    return -1;
  }
  shm_size_ = size;
  strncpy(shm_key_, key, 31);

  // open
  shm_hdl_ = shm_open(shm_key_, O_RDWR | O_CREAT, 0777);
  if (shm_hdl_ == SHM_NULL) {
    VZ_ERROR("shm_open %s failed.\n", shm_key_);
    return -2;
  }

  // 分配共享内存大小
  int ret = ftruncate(shm_hdl_, shm_size_);
  if (-1 == ret) {
    VZ_ERROR("ftruncate %s failed.\n", shm_key_);
    return -3;
  }

  // mmap
  shm_ptr_ = mmap(NULL, shm_size_,
                  PROT_READ | PROT_WRITE,
                  MAP_SHARED, shm_hdl_, SEEK_SET);
  if (shm_ptr_ == NULL) {
    VZ_ERROR("mmap %s failed.\n", shm_key_);
    return -4;
  }
  return 0;
}

void VShm::Close() {
  if (shm_ptr_) {
    if (0 != munmap(shm_ptr_, shm_size_)) {
      VZ_ERROR("munmap %s failed.\n", shm_key_);
    }
    shm_ptr_ = NULL;
  }

  // 不释放,其他应用还在使用
  // if (shm_hdl_ != SHM_NULL) {
  //   if (0 != shm_unlink(shm_key_)) {
  //     VZ_PRINT("shm_unlink %s failed %d.\n", shm_key_, shm_hdl_);
  //   }
  //   shm_hdl_ = SHM_NULL;
  // }
}
#else
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>

int VShm::Open(ShmKey key, ShmSize size) {
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
  if(shm_hdl_ < 0) {
    shm_hdl_ = shmget(n_key, 0, 0);
  }
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

#endif
#endif  // WIN32

/**共享内存-参数*********************************************************/
CVzShmArg::CVzShmArg() {
  valid_ = false;
}

CVzShmArg::~CVzShmArg() {
  Close();
}

int CVzShmArg::Open() {
  int ret = vshm_.Open("log_shm_arg", sizeof(TAG_SHM_ARG)+1);
  return ret;
}


void CVzShmArg::Close() {
  vshm_.Close();
}

int CVzShmArg::Share(const void* data, unsigned int size) {
  if (vshm_.GetData() &&
      size <= vshm_.GetSize()) {
    memcpy(vshm_.GetData(), data, size);
    return 0;
  }
  return -1;
}

unsigned int CVzShmArg::GetSendLevel() const {
  TAG_SHM_ARG* pArg = (TAG_SHM_ARG*)vshm_.GetData();
  if (pArg) {
    return pArg->snd_level;
  }
  return 3;
}

struct sockaddr_in* CVzShmArg::GetSockAddr() const {
  TAG_SHM_ARG* pArg = (TAG_SHM_ARG*)vshm_.GetData();
  if (pArg) {
    return &pArg->sock_addr;
  }
  return NULL;
}

int CVzShmArg::GetShmArg(TAG_SHM_ARG *arg) {
  TAG_SHM_ARG* pArg = (TAG_SHM_ARG*)vshm_.GetData();
  if (pArg) {
    memcpy(arg, pArg, sizeof(TAG_SHM_ARG));
    return 0;
  }
  return -1;
}

}  // namespace vzlogging
