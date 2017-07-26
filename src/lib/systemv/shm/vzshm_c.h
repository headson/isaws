/************************************************************************
*Author      : Sober.Peng 17-06-28
*Description :
************************************************************************/
#ifndef _VZSHM_C_H_
#define _VZSHM_C_H_

#include "vz_shm.h"

// 视频通道0
#define SHM_VIDEO_0       "/dev/shm/video_0"
#define SHM_IMAGE_0       "/dev/shm/image_0"

#define SHM_AUDIO_0       "/dev/shm/audio_0"
#define SHM_AUDIO_0_SIZE  1024

#define SHM_HEAD_SIZE     128

class CShareBuffer {
public:
  typedef struct {
    unsigned int  n_w_sec;        // 写秒
    unsigned int  n_w_usec;       // 写微妙

    unsigned int  n_width;
    unsigned int  n_height;

    unsigned int  n_head_1;       // SPS长度
    unsigned int  n_head_2;       // PPS长度
    unsigned char s_head[SHM_HEAD_SIZE]; // SPS_PPS

    unsigned int  n_size;         // 数据buffer长度
    unsigned int  n_data;         // 写长度
    unsigned char p_data[0];      // 数据指针
  } TAG_SHM_VDO;

public:
  CShareBuffer();
  ~CShareBuffer();

  bool Create(const char *s_path, unsigned int n_size);

  bool Open(const char *s_file, unsigned int n_size);

  int ReadHead(char* p_data, unsigned int n_data,
               int *n_sps, int *n_pps);
  int Read(char* p_data, unsigned int n_data,
           unsigned int *n_sec, unsigned int *n_usec);

  int WriteSps(const char *p_data, unsigned int n_data);
  int WritePps(const char *p_data, unsigned int n_data);

  int Write(const char *p_data, unsigned int n_data,
            unsigned int n_sec, unsigned int n_usec);

  void GetVdoSize(unsigned int *w, unsigned int *h);
  void SetWidth(unsigned int w);
  void SetHeight(unsigned int h);

private:
  HANDLE        hdl_shm_;
  void         *mem_ptr_;

  HANDLE        hdl_sem_;
};

#endif  // _VZSHM_C_H_

