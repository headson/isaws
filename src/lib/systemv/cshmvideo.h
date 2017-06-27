/************************************************************************/
/* Author      : Sober.Peng 17-06-26
/* Description : 
/************************************************************************/
#ifndef _VSHMVIDEO_H_
#define _VSHMVIDEO_H_

#ifdef WIN32
#include <winsock2.h>  // vs2010; struct timeval
#endif 
#include "basictypes.h"

#include "vshm.h"

class VShmVideo {
 public:
  VShmVideo();
  virtual ~VShmVideo();

  int32 Create(const uint8 *s_path, uint32 n_shm_size);
  int32 Open(const uint8 *s_path, uint32 n_shm_size);

  int32 Read(int8* p_data, uint32 n_data, struct timeval* p_tm);
  int32 Write(const int8* p_data, uint32 n_data, const struct timeval* p_tm);
  
  // ����д��buffer[h264: head + body]
  int32 WriteBegin();
  int32 Write(const int8* p_data, uint32 n_data);
  int32 WriteEnd(const struct timeval* p_tm);

 protected:
  uint32   n_vdo_size_;   // ��Ƶ�ռ�
  VShm     v_shm_;        // �����ڴ�
  VSem     v_sem_w_;      // д��
};


#endif  // _VSHMVIDEO_H_
