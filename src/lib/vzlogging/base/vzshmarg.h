/************************************************************************/
/* Copyright@ 2008 vzenith.com
/* All rights reserved.
/* ----------------------------------------------------------------------
/* Author      : Sober.Peng
/* Date        : 19:5:2017
/* Description : 通过共享内存实现进程间日志参数传递
/************************************************************************/
#ifndef SRC_LIB_VZLOGGING_BASE_VZSHMARG_H_
#define SRC_LIB_VZLOGGING_BASE_VZSHMARG_H_

#include "vzlogging/base/vzlogdef.h"

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
typedef HANDLE              ShmHdl;
typedef DWORD               ShmSize;
typedef LPVOID              ShmPtr;
#define SHM_NULL            NULL
#else
#include <netdb.h>
typedef int                 ShmHdl;
typedef unsigned int        ShmSize;
typedef void*               ShmPtr;
#define SHM_NULL            -1
#endif

typedef const char*         ShmKey;

namespace vzlogging {

/**VShm******************************************************************/
class VShm {
 public:
  VShm();
  virtual ~VShm();

  int           Open(ShmKey key, ShmSize size);
  void          Close();

  void*         GetData() const {
    return shm_ptr_;
  }
  unsigned int  GetSize() const {
    return shm_size_;
  }

 private:
  char      shm_key_[32];

 private:
  ShmHdl    shm_hdl_;     //
  ShmPtr    shm_ptr_;     //
  ShmSize   shm_size_;    //
};

/**共享内存-参数*********************************************************/
typedef struct {
  unsigned int        snd_level;      // 日志打印级别
  struct sockaddr_in  sock_addr;      //

  unsigned int        checksum;       // 校验;校验共享内存为被串改
} TAG_SHM_ARG;

class CVzShmArg {
 public:
  CVzShmArg();
  virtual ~CVzShmArg();

  // 打开
  int   Open();
  void  Close();

  // 共享
  int   Share(const void* data, unsigned int size);

  // 获取参数
  unsigned int        GetSendLevel() const;
  struct sockaddr_in* GetSockAddr() const;

 private:
  VShm        vshm_;
  bool        valid_;   // 有意义的传参
};

}  // namespace vzlogging

#endif  // SRC_LIB_VZLOGGING_BASE_VZSHMARG_H_
