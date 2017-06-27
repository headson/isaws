/************************************************************************/
/* 作者: SoberPeng 17-05-23
/* 描述:
/************************************************************************/
#ifndef LIBBASE_VSHM_H
#define LIBBASE_VSHM_H

#include <stdio.h>
#include "basictypes.h"

#ifdef WIN32
#include <windows.h>
typedef HANDLE        ShmHdl;
typedef DWORD         ShmSize;
typedef LPVOID        ShmPtr;
#define SHM_NULL      NULL
#else
#include <netdb.h>
typedef int           ShmHdl;
typedef uint32        ShmSize;
typedef void*         ShmPtr;
#define SHM_NULL      -1
#endif

typedef uint8*        ShmKey;
typedef uint8*        SemKey;

typedef struct {
  uint32          n_read_count; // 读优先
  struct timeval  c_tm_capture; // 采集时间

  uint32          n_buffer;     // 数据偏移
  int8            p_buffer[0];  // 数据地址
} TAG_SHM_DATA;

class VSem {
 public:
  VSem();
  virtual ~VSem();

 public:
  int32   Open(const SemKey key);

  bool    IsValid();

  int32   Wait(uint32 n_msec = (uint32)-1);
  bool    Signal();

 private:
#ifdef WIN32
  HANDLE    sem_;
#else
  int       sem_;
#endif
};

class VShm {
 public:
  VShm();
  virtual ~VShm();

  int32       Open(const ShmKey key, ShmSize size);
  void        Close();

  bool          IsValid() {
    if (shm_size_ > 0
        && shm_ptr_ != NULL) {
      return true;
    }
    return false;
  }

  void*   GetData() const {
    return shm_ptr_;
  }
  uint32  GetSize() const {
    return shm_size_;
  }

 private:
  ShmHdl    shm_hdl_;     //
  ShmPtr    shm_ptr_;     //
  ShmSize   shm_size_;    //
};

#endif  // LIBBASE_VSHM_H
