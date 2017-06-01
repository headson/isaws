/************************************************************************/
/* Author      : Sober.Peng 17-06-01
/* Description : 
/************************************************************************/
#ifndef LIBPLATFORM_CVIDEO_H
#define LIBPLATFORM_CVIDEO_H
#include "inc/vtypes.h"

class CVideo
{
public:
  CVideo();
  virtual ~CVideo();

  int32_t VideoCapture(int32_t n_chn);

};

#endif  // LIBPLATFORM_CVIDEO_H
