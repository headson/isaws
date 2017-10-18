/************************************************************************
*Author      : Sober.Peng 17-10-18
*Description :
************************************************************************/
#ifndef _VSHM_H_
#define _VSHM_H_

#ifdef WIN32
#include <windows.h>
#define HDL_NULL  NULL
#else   // LINUX
#include <fcntl.h>
#include <unistd.h>
typedef int       HANDLE;
#define HDL_NULL  -1
#endif  // LINUX

namespace vzbase {

class VShm {
 public:
  VShm() {
    shm_id_ = HDL_NULL;
    shm_ptr_ = NULL;
    shm_size_ = 0;
  }
  virtual ~VShm() {
    if (shm_ptr_) {
      vzShmDt(shm_ptr_);
      shm_ptr_ = NULL;
    }
  }

#ifdef _WIN32
  bool Open(const char* name, int size);

 protected:
  void *vzShmAt();
  void vzShmDt(void *p_ptr);

#else
  int create_file(const char *filename);
  bool Open(const char* name, int size);

 protected:
  void *vzShmAt();
  void vzShmDt(void *p_ptr);
#endif
 public:
  void *GetData() const {
    return shm_ptr_;
  }
  int GetSize() const {
    return shm_size_;
  }

  int GetData(void *pdata, int ndata);
  int SetData(const void *pdata, int ndata);

 private:
  HANDLE  shm_id_;
  void *  shm_ptr_;     //
  int     shm_size_;    //
};

}

#endif  // _VSHM_H_
