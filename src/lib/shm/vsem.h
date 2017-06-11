/************************************************************************/
/* Author      : Sober.Peng 17-06-05
/* Description :
/************************************************************************/
#ifndef SHM_CAMERA_H264_VSEM_H
#define SHM_CAMERA_H264_VSEM_H

#include <stdio.h>
#include <stdint.h>

#ifdef WIN32
#include <windows.h>
typedef char*         SemKey;
#else
typedef uint32_t      SemKey;
#endif

class VSem {
 public:
  VSem();
  virtual ~VSem();

 public:
  int32_t Open(const SemKey key);

  bool    IsValid();

  int32_t Wait(uint32_t timeout = 0xFFFFFFFF);
  bool    Signal();

 private:
#ifdef WIN32
  HANDLE    sem_;
#else
  int       sem_;
#endif
};

#endif  // SHM_CAMERA_H264_VSEM_H
