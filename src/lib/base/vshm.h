/************************************************************************/
/* 作者: SoberPeng 17-05-23
/* 描述: 
/************************************************************************/
#ifndef LIBBASE_VSHM_H
#define LIBBASE_VSHM_H

#include <stdint.h>

#ifdef WIN32
#include <windows.h>
typedef HANDLE        ShmHdl;
typedef DWORD         ShmSize;
typedef LPVOID        ShmPtr;
#define SHM_NULL      NULL
#else
#include <netdb.h>
typedef int           ShmHdl;
typedef uint32_t      ShmSize;
typedef void*         ShmPtr;
#define SHM_NULL      -1
#endif

typedef char          ShmKey;

namespace yhbase {

  class VShm {
  public:
    VShm();
    virtual ~VShm();

    int32_t       Open(const ShmKey* key, ShmSize size);
    void          Close();

    bool          IsValid() {
      if (shm_size_ > 0
        && shm_ptr_ != NULL) {
        return true;
      }
      return false;
    }

    void*         GetData() const {
      return shm_ptr_;
    }
    uint32_t  GetSize() const {
      return shm_size_;
    }

  private:
    char      shm_key_[32]; // 

  private:
    ShmHdl    shm_hdl_;     // 
    ShmPtr    shm_ptr_;     //
    ShmSize   shm_size_;    //
  };
}  // namespace base

#endif  // LIBBASE_VSHM_H
