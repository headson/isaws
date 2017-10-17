/************************************************************************
*Author      : Sober.Peng 17-10-17
*Description :
************************************************************************/
#ifndef LIBDEVICE_CVIDEOCODEC_H_
#define LIBDEVICE_CVIDEOCODEC_H_

#include "vencoder.h"
#include "vzbase/base/boost_settings.hpp"

class CVideoCodec : vzbase::noncopyable {
 protected:
  CVideoCodec();
 public:
  virtual ~CVideoCodec();

 public:
  CVideoCodec *Create();

 protected:

 private:
  VideoEncoder* video_encoder_;
};


#endif  // LIBDEVICE_CVIDEOCODEC_H_
