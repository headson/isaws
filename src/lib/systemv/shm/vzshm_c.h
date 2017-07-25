/************************************************************************
*Author      : Sober.Peng 17-06-28
*Description :
************************************************************************/
#ifndef _VZSHM_C_H_
#define _VZSHM_C_H_

#include "vz_shm.h"

// 视频通道0
#define SHM_VIDEO_0       "/dev/shm/video_0"
#define SHM_VIDEO_0_SIZE  1024*1024

#define SHM_AUDIO_0       "/dev/shm/audio_0"
#define SHM_AUDIO_0_SIZE  1024

class CShareVideo {
 public:
  typedef struct {
    unsigned int  n_w_sec;        // 写秒
    unsigned int  n_w_usec;       // 写微妙

    unsigned int  n_width;        // 图像宽
    unsigned int  n_height;       // 图像高

    unsigned int  n_sps;
    unsigned int  n_pps;          // SPS_PPS长度
    unsigned char sps_pps[128];   // SPS_PPS

    unsigned int  n_size;         // 数据buffer长度
    unsigned int  n_data;         // 写长度
    unsigned char p_data[0];      // 数据指针
  } ShmVdo;

 public:
  CShareVideo();
  ~CShareVideo();

  bool Create(const char *s_key, unsigned int n_size);
  bool Open(const char *s_key, unsigned int n_size);

  int SetSps(const char *p_data, unsigned int n_data);
  int SetPps(const char *p_data, unsigned int n_data);

  int GetSpsPps(char* p_data, unsigned int n_data,
               int *n_sps, int *n_pps);

  unsigned int GetWidth();
  void SetWidth(unsigned int w);

  unsigned int GetHeight();
  void SetHeight(unsigned int h);

 public:
  bool Lock();
  bool Unlock();

  ShmVdo *GetData();

 private:
  HANDLE   hdl_sem_;

  HANDLE   hdl_shm_;
  void    *mem_ptr_;
};

#endif  // _VZSHM_C_H_

