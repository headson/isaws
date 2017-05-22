/************************************************************************/
/* ����: SoberPeng 2017-05-20
/* ����: �����ڴ�-����ԭʼͼ��YUV
/************************************************************************/
#ifndef SRCPROJ_CSHMIMAGE_H
#define SRCPROJ_CSHMIMAGE_H

#include "vshm.h"

namespace base {

  typedef struct {
    uint32_t n_width;     // ͼ���
    uint32_t n_height;    // ͼ���

    uint32_t n_size;      // ���ݴ�С
    uint8_t  a_data[0];   // ����
  } TAG_SHM_IMAGE;

  class CShmImage
  {
  public:
    CShmImage(void);
    ~CShmImage(void);

    int32_t Open(const ShmKey* s_key, ShmSize n_size);
    void    Close();

    int32_t Share(const void* p_data, uint32_t n_data);

    const void* GetData();
    uint32_t    GetSize();
    uint32_t    GetWidth();
    uint32_t    GetHeight();

  private:
    VShm        v_shm_; // ��������ʵ��
  };

}  // namespace base

#endif  // SRCPROJ_CSHMIMAGE_H

