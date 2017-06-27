#include "vshm.h"

#include <stdio.h>
#include <errno.h>
#include <string.h>

#include "stdafx.h"

static void  create_file(char *filename) {
  if(access(filename, 0) == 0) {
    return;
  }
	
  if (creat(filename, 0755) < 0) {
    printf("create file %s failure!\n", filename);
    exit(EXIT_FAILURE);
  } else {
    printf("create file %s success!\n", filename);
  }
}

VShm::VShm() {
  shm_hdl_ = SHM_NULL;
  shm_ptr_ = NULL;
  shm_size_ = 0;
}

VShm::~VShm() {
  Close();
}

#ifdef WIN32
VSem::VSem()
  : sem_(NULL) {
}

VSem::~VSem() {
}

int32 VSem::Open(const SemKey key) {
  uint32 lInitialCount = 1;
  uint32 lMaximumCount = 1;
  sem_ = CreateSemaphore(NULL, lInitialCount, lMaximumCount, (LPCTSTR)key);
  if (sem_ == NULL) {
    printf("CreateSemaphore failed.\n");
    return -1;
  }
  return 0;
}

bool VSem::IsValid() {
  if (sem_ == NULL) {
    return false;
  }
  return true;
}

int32 VSem::Wait(uint32 timeout /*= INFINITE*/) {
  if (sem_ == NULL) {
    return -1;
  }

  return WaitForSingleObject(sem_, timeout);
}

bool VSem::Signal() {
  if (sem_ == NULL) {
    return false;
  }
  uint32 lReleaseCount = 1;
  uint32 *lpPreviousCount = NULL;
  return (ReleaseSemaphore(sem_, lReleaseCount, (LPLONG)lpPreviousCount) == TRUE);
}

//////////////////////////////////////////////////////////////////////////
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

VSem::VSem()
  : sem_(-1) {
}

VSem::~VSem() {
}

int32 VSem::Open(const SemKey key) {
  create_file(key);
  
  key_t k = ftok((char*)key, 0);
  if (k < 0) {
    LOG_ERROR("ftok failed.\n");
    return -1;
  }

  sem_ = semget(k, 1, IPC_CREAT|IPC_EXCL|0666);
  if(sem_< 0 ) {
    sem_ = semget(k, 0, 0666);
  }
  if(sem_ < 0 ) {
    LOG(L_ERROR) << "semget failed."<<key;
    return -1;
  }
  LOG_INFO("sem id:%d %d %d\n", sem_, key, k);
  return 0;
}

bool VSem::IsValid() {
  if (sem_ < 0) {
    return false;
  }
  return true;
}

int32 VSem::Wait(uint32 n_msec) {
  struct sembuf sem_op = {0, -1, 0};
  sem_op.sem_num = 0;
  sem_op.sem_flg = IPC_NOWAIT;
  if (n_msec = (uint32)-1) {
    if ((semop(sem_, &sem_op, 1)) == -1) {
      return -1;
    }
  } else {
    struct timespec ts;
    ts.tv_sec = n_msec / 1000;
    ts.tv_nsec = (n_msec % 1000) * 1000;
    if (semtimedop(sem_, &sem_op, 1, &ts) == -1) {
      return -1;
    }
  }
  return 0;
}

bool VSem::Signal() {
  struct sembuf sem_op = {0, 1, 0};
  sem_op.sem_num = 0;
  sem_op.sem_flg = IPC_NOWAIT;
  if( (semop(sem_, &sem_op, 1)) == -1) {
    return -1;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////
int32 VShm::Open(const ShmKey key, ShmSize size) {
  if (key == NULL || size == 0) {
    return -1;
  }
  
  create_file(key);
  key_t k = ftok((char*)key, 0);
  if (k < 0) {
    LOG_ERROR("ftok failed %s.\n", key);
    return -1;
  }

  shm_hdl_ = shmget(k, size, IPC_CREAT | 0660);
  if(shm_hdl_ < 0) {
    shm_hdl_ = shmget(k, 0, 0);
    if(shm_hdl_ < 0) {
      LOG_ERROR("shared memory create failed %s-0x%x.\n", key, k);
      return -1;
    }
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

