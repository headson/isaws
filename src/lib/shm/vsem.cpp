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

int32 VSem::Open(const SemKey key) {
  sem_ = sem_open(key, O_CREAT, 0777, 1);
  if(sem_ == NULL) {
    sem_ = sem_open(key, O_CREAT, 0777, 1);
  }
  if(sem_ == NULL) {
    return -1;
  }
  LOG_INFO("sem id:0x%0x %s %d\n", sem_, key, k);
  return 0;
}

bool VSem::IsValid() {
  if (sem_ == NULL){
    return false;
  }
  return true;
}

int32 VSem::Wait(uint32 timeout) {
  if (sem_ != NULL) {
    sem_wait(sem_);
    return 0;
  }
  return -1;
}

bool VSem::Signal() {
  if (sem_ != NULL) {
    sem_post(sem_);
    return 0;
  }
  return 0;
}

#endif
