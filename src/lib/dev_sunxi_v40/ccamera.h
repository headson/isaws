/************************************************************************
*Author      : Sober.Peng 17-10-17
*Description :
************************************************************************/
#ifndef LIBDEVICE_CCAMERA_H_
#define LIBDEVICE_CCAMERA_H_

#include <string>

#include "vzbase/base/boost_settings.hpp"

typedef struct TAG_CAP_DATA {
  void         *start;
  size_t        length;
  unsigned int  phy_addr;
} TAG_CAP_DATA;
typedef int (*CameraCallback)(const void *vir_addr,
                              unsigned int phy_addr,
                              int width, int height,
                              void *usr_arg);

class CCamera : public vzbase::noncopyable {
 protected:
  CCamera(const char *dev_name,
          int width, int height);
  virtual ~CCamera();

 public:
  static CCamera *Create(const char *dev_name,
                         int width, int height);

  int GetFrame(bool *bexit, CameraCallback callback, void *usr_arg);

 protected:
  int capture_init();
  int capture_quit();

 private:
  int             videofd;

 private:
  std::string     dev_name_;
  int             cap_w_;       // supported capture size
  int             cap_h_;

  int             cap_fps_;     // capture framerate
  int             cap_fmt_;     // capture format V4L2_PIX_FMT_NV12\V4L2_PIX_FMT_YUV420

  int             buf_cnt_;     // request buffers count
  TAG_CAP_DATA   *buffers_;
};

#endif  // LIBDEVICE_CCAMERA_H_
