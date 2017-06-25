/************************************************************************/
/* Author      : SoberPeng 2017-06-09
/* Description :
/************************************************************************/
#include "vsem.h"

#include "stdafx.h"

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

#else // LINUX

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

VSem::VSem()
  : sem_(-1) {
}

VSem::~VSem() {
}

int32_t VSem::Open(const SemKey key) {
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

int32_t VSem::Wait(uint32_t timeout) {
  struct sembuf sem_op = {0, -1, 0};
  sem_op.sem_num = 0;
  sem_op.sem_flg = IPC_NOWAIT;
  if( (semop(sem_, &sem_op, 1)) == -1) {
    return -1;
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

#endif
