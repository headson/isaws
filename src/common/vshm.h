/*
 * 描述：线程私有数据
 */

#ifndef ISAW_VSHM_H
#define ISAW_VSHM_H

#include <stdint.h>

#ifdef WIN32
#include <windows.h>
typedef HANDLE              ShmHdl;
typedef DWORD               ShmSize;
typedef LPVOID              ShmPtr;
#define INVALID_SHM_HANDLE  NULL
#else
typedef int                 ShmHdl;
typedef uint32_t            ShmSize;
typedef void*               ShmPtr;
#define INVALID_SHM_HANDLE  -1
#endif

typedef const char*         ShmKey;

namespace isaw {

class VShm {
 public:
  VShm();
  virtual ~VShm();

  int32_t   Open(ShmKey key, ShmSize size);
  void      Close();

  void*     GetData() const {
    return shm_ptr_;
  }
  uint32_t  GetSize() const {
    return shm_size_;
  }

 private:
  char      shm_key_[32];

 private:
  ShmHdl    shm_hdl_;     //
  ShmPtr    shm_ptr_;     //
  ShmSize   shm_size_;    //
};

}

#endif // ISAW_VSHM_H
