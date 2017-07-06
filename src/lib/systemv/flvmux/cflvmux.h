/************************************************************************
*Author      : Sober.Peng 17-07-04
*Description :
************************************************************************/
#ifndef _CFLVMUX_H
#define _CFLVMUX_H

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#include "flv.h"

typedef struct _FLV_TAG {
  uint32_t n_max;    // 最大小
  uint32_t w_pos;    // 写偏移
  uint8_t  data[0];  // 数据
} FLV_DATA;

class CFlvMux {
 private:
  FILE  *p_file;

 public:
  CFlvMux(FILE *file=NULL);
  virtual ~CFlvMux();

 public:
  int32_t InitHeadTag0(uint8_t *p_dst, uint32_t n_dst,
                       uint32_t n_width, uint32_t n_height);

  int32_t VideoPacket(uint8_t *p_dst, uint32_t n_dst,
                      bool b_key_frame, uint64_t n_pts,
                      const uint8_t *p_src, uint32_t n_src);

  int32_t AudioPacket(uint8_t *p_dst, uint32_t n_dst,
                      uint64_t n_pts,
                      const uint8_t *p_src, uint32_t n_src);
};

#endif  // _CFLVMUX_H

