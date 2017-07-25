/************************************************************************
*Author      : Sober.Peng 17-06-28
*Description :
************************************************************************/
#ifndef _VZSHM_C_H_
#define _VZSHM_C_H_

#include "vz_shm.h"

// ��Ƶͨ��0
#define SHM_VIDEO_0       "/dev/shm/video_0"
#define SHM_VIDEO_0_SIZE  1024*1024

#define SHM_AUDIO_0       "/dev/shm/audio_0"
#define SHM_AUDIO_0_SIZE  1024

class CShareVideo {
 public:
  typedef struct {
    unsigned int  n_w_sec;        // д��
    unsigned int  n_w_usec;       // д΢��

    unsigned int  n_width;        // ͼ���
    unsigned int  n_height;       // ͼ���

    unsigned int  n_sps;
    unsigned int  n_pps;          // SPS_PPS����
    unsigned char sps_pps[128];   // SPS_PPS

    unsigned int  n_size;         // ����buffer����
    unsigned int  n_data;         // д����
    unsigned char p_data[0];      // ����ָ��
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

