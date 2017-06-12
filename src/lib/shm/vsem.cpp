/************************************************************************/
/* Author      : SoberPeng 2017-06-09
/* Description :
/************************************************************************/
#include "vsem.h"
#include "inc/vdefine.h"

#ifdef WIN32
VSem::VSem()
  : sem_(NULL) {
}

VSem::~VSem() {
}

int32_t VSem::Open(const SemKey key) {
  uint32_t lInitialCount = 1;
  uint32_t lMaximumCount = 1;
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

int32_t VSem::Wait(uint32_t timeout /*= INFINITE*/) {
  if (sem_ == NULL) {
    return -1;
  }

  return WaitForSingleObject(sem_, timeout);
}

bool VSem::Signal() {
  if (sem_ == NULL) {
    return false;
  }
  uint32_t lReleaseCount = 1;
  uint32_t *lpPreviousCount = NULL;
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
  key_t k = ftok(".", key);
  if (k < 0) {
    LOG_ERROR("ftok failed.\n");
    return -1;
  }

  sem_ = semget(k, 0, 0666);
  if(sem_ < 0) {
    sem_ = semget(k, 1, IPC_CREAT|IPC_EXCL|0666);
  }
  if(sem_ < 0) {
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
  struct sembuf sb;
  sb.sem_num   = 0;
  sb.sem_op    = -1;
  sb.sem_flg   = SEM_UNDO;
  if( (semtimedop(sem_, &sb, 1, NULL)) == -1) {
    return -1;
  }
  return 0;
}

bool VSem::Signal() {
  struct sembuf sb;
  sb.sem_num   = 0;
  sb.sem_op    = 1;
  sb.sem_flg   = SEM_UNDO;
  if( (semtimedop(sem_, &sb, 1, NULL)) == -1) {
    return -1;
  }
  return 0;
}

#endif
