/************************************************************************/
/* 作者: SoberPeng 2017-05-20
/* 描述: 共享内存-共享原始图像YUV
/************************************************************************/
#ifndef SRCPROJ_CSHMIMAGE_H
#define SRCPROJ_CSHMIMAGE_H

#include "vshm.h"

namespace base {

  typedef struct {
    uint32_t n_width;     // 图像宽
    uint32_t n_height;    // 图像高

    uint32_t n_size;      // 数据大小
    uint8_t  a_data[0];   // 数据
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
    VShm        v_shm_; // 共享数据实例
  };

}  // namespace base

#endif  // SRCPROJ_CSHMIMAGE_H

