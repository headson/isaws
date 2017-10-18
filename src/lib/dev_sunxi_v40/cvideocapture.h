/************************************************************************
*Author      : Sober.Peng 17-10-17
*Description :
************************************************************************/
#ifndef LIBDEVICE_CVIDEOCAPTURE_H
#define LIBDEVICE_CVIDEOCAPTURE_H

#include "vzbase/system/vshm.h"

#include "ccamera.h"
#include "cvideocodec.h"

#include "vzbase/thread/thread.h"
#include "vzbase/base/boost_settings.hpp"

class CVideoCapture :
  public vzbase::noncopyable,
  public vzbase::Runnable {
 public:
  CVideoCapture();
  virtual ~CVideoCapture();

  int Start(int dev_num);
  void Stop();

 protected:
  static int camera_callback(const void *vir_addr,
                             unsigned int phy_addr,
                             int width, int height,
                             void *usr_arg) {
    int res = -1;
    if (usr_arg) {
      res = ((CVideoCapture*)usr_arg)->OnCamera(vir_addr,
            phy_addr,
            width, height);
    }
    return res;
  }
  int OnCamera(const void *vir_addr,
               unsigned int phy_addr,
               int width, int height);

  static int video_callback(VencOutputBuffer *output_buffer, void *usr_arg) {
    int res = -1;
    if (usr_arg) {
      res = ((CVideoCapture*)usr_arg)->OnVideo(output_buffer);
    }
    return res;
  }
  int OnVideo(VencOutputBuffer *output_buffer);

  int SetSpsPps(const char *sps_pps, int sps_pps_size);

 protected:
  void Run(vzbase::Thread* thread);

 private:
  vzbase::Thread *thread_;
  unsigned int    is_exit_;

 private:
  CCamera        *camera_;
  CVideoCodec    *vdo_code_;

 private:
  vzbase::VShm    shm_vdo_;
  TAG_SHM_VDO    *shm_vdo_ptr_;

  vzbase::VShm    shm_img_;
  TAG_SHM_IMG    *shm_img_ptr_;
};

#endif  // LIBDEVICE_CVIDEOCAPTURE_H
