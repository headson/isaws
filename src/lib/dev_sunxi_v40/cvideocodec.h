/************************************************************************
*Author      : Sober.Peng 17-10-17
*Description :
************************************************************************/
#ifndef LIBDEVICE_CVIDEOCODEC_H_
#define LIBDEVICE_CVIDEOCODEC_H_

#include "vencoder.h"
#include "memoryAdapter.h"

#include "vzbase/base/boost_settings.hpp"

typedef int (*VideoCallback)(VencOutputBuffer *output_buffer, void *usr_arg);

class CVideoCodec : vzbase::noncopyable {
 protected:
  CVideoCodec(int codec,
              int src_w, int src_h,
              int dst_w, int dst_h);

 public:
  virtual ~CVideoCodec();

 public:
  static CVideoCodec *Create(int codec,
                             int src_w, int src_h,
                             int dst_w, int dst_h);

  int GetIFrame();
  int SetBitRate(int bit_rate);
  int SetFrameRate(int frame_rate);
  int SetMaxKeyFrame(int max_key_frame);
  VencHeaderData &GetHeadData() {
    return sps_pps_data;
  }

  int EncodeInit();
  int EncodeExit();

  int EncodeFrame(unsigned int phy_addr,
                  int width, int height,
                  VideoCallback callback, void *usr_arg);

  int EncodeFrame(const char *vir_addr,
                  int width, int height,
                  VideoCallback callback, void *usr_arg);

 private:
  VideoEncoder*       pVideoEnc;
  VENC_CODEC_TYPE     encode_format;

  int                 src_width;
  int                 src_height;
  int                 dst_width;
  int                 dst_height;

 private:
  EXIFInfo            exifinfo;
  VencH264Param       h264Param;

  VencBaseConfig      baseConfig;
  VencHeaderData      sps_pps_data;
};


#endif  // LIBDEVICE_CVIDEOCODEC_H_
