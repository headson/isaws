/************************************************************************/
/* Author      : Sober.Peng 17-06-05
/* Description :
/************************************************************************/
#ifndef SHM_CAMERA_H264_VSEM_H
#define SHM_CAMERA_H264_VSEM_H

#include <stdio.h>
#include "basictypes.h"

#ifdef WIN32
#include <windows.h>
#else
#include <semaphore.h>
#endif

typedef uint8*         SemKey;

class VSem {
 public:
  VSem();
  virtual ~VSem();

 public:
  int32   Open(const SemKey key);

  bool    IsValid();

  int32   Wait(uint32 timeout = 0xFFFFFFFF);
  bool    Signal();

 private:
#ifdef WIN32
  HANDLE    sem_;
#else
  int       sem_;
#endif
};

#endif  // SHM_CAMERA_H264_VSEM_H
