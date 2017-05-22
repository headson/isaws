//
// Created by SoberPeng on 2017/5/16 0016.
//

#ifndef SRCPROJ_CSHMVIDEO_H
#define SRCPROJ_CSHMVIDEO_H

#include "vshm.h"

#include <stdint.h>
#include <string>

namespace isaw {

typedef struct {
  uint32_t  n_size;
  uint8_t   a_data[0];
} TAG_SHM_VIDEO;

class CShmVideo {
 public:
  CShmVideo();
  virtual ~CShmVideo();

  int32_t   Open(ShmKey key, ShmSize size);

  //
  int32_t   Share(const void* data, uint32_t size);

  // 获取数据
  void      *GetData();
  // 获取数据长度
  uint32_t  GetSize();

 private:
  VShm      vshm_;
};

}

#endif //SRCPROJ_CSHMVIDEO_H
