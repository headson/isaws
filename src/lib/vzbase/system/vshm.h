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

}  // namespace vzbase

typedef struct TAG_SHM_ARG {
  const char  *dev_name;    // 设备名
  int          width;       // 宽
  int          height;      // 高
  int          shm_size;    // 共享大小
} TAG_SHM_ARG;

extern TAG_SHM_ARG g_shm_img[2];
extern TAG_SHM_ARG g_shm_avdo[4];

#define MAX_SPS_PPS       128
typedef struct {
  unsigned int  wsec;       // 写秒
  unsigned int  wusec;      // 写微妙

  unsigned int  width;
  unsigned int  height;

  unsigned int  nhead;
  unsigned char shead[MAX_SPS_PPS]; // SPS_PPS

  unsigned int  nsize;      // 数据buffer长度
  unsigned int  ndata;      // 写长度
  unsigned char pdata[0];   // 数据指针
} TAG_SHM_VDO;

typedef struct {
  unsigned int  wsec;       // 写秒
  unsigned int  wusec;      // 写微妙

  unsigned int  width;
  unsigned int  height;

  unsigned int  nsize;      // 数据buffer长度
  unsigned int  ndata;      // 写长度
  unsigned char pdata[0];   // 数据指针
} TAG_SHM_IMG;

#endif  // _VSHM_H_
