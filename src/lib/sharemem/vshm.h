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

typedef uint8*         ShmKey;

class VShm {
 public:
  VShm();
  virtual ~VShm();

  int32       Create(const ShmKey key, ShmSize size);
  int32       Open(const ShmKey key, ShmSize size);
  void        Close();

  bool          IsValid() {
    if (shm_size_ > 0
        && shm_ptr_ != NULL) {
      return true;
    }
    return false;
  }

  void*     GetData() const {
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
