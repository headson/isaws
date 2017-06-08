/************************************************************************/
/* Author      : Sober.Peng 17-06-05
/* Description :
/************************************************************************/
#ifndef SHM_CAMERA_H264_VSEM_H
#define SHM_CAMERA_H264_VSEM_H

#include <stdio.h>
#include <stdint.h>

#include <windows.h>

class VSem {
 public:
  VSem() {
  }

  virtual ~VSem() {
    Close();
  }

 public:
  int32_t Open(uint32_t lInitialCount = 1, uint32_t lMaximumCount = 1, const int8_t *lpName = NULL) {
    sem_ = CreateSemaphore(NULL, lInitialCount, lMaximumCount, (LPCTSTR)lpName);
    if (sem_ == NULL) {
      printf("CreateSemaphore failed.\n");
      return -1;
    }
    return 0;
  }
  void Close() {
    if (sem_ != NULL) {
      CloseHandle(sem_);
      sem_ = NULL;
    }
  }

  bool IsValid() {
    if (sem_ == NULL) {
      return false;
    }
    return true;
  }

  int32_t Wait(uint32_t timeout = INFINITE) {
    if (sem_ == NULL) {
      return -1;
    }

    return WaitForSingleObject(sem_, timeout);
  }

  bool Release(uint32_t lReleaseCount = 1, uint32_t *lpPreviousCount = NULL) {
    if (sem_ == NULL) {
      return false;
    }
    return (ReleaseSemaphore(sem_, lReleaseCount, (LPLONG)lpPreviousCount) == TRUE);
  }

private:
  HANDLE    sem_;
};


#endif  // SHM_CAMERA_H264_VSEM_H
