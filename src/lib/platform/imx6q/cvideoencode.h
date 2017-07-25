/************************************************************************/
/* ����: SoberPeng 17-05-23
/* ����:
/************************************************************************/
#ifndef LIBPLATFORM_CVIDEOENCODE_H
#define LIBPLATFORM_CVIDEOENCODE_H

#include <string>

#include "vpu.h"
#include "v4l2.h"

#include "systemv/shm/vzshm_c.h"

class CVideoEncode {
 public:
  CVideoEncode();
  virtual ~CVideoEncode();

  bool Start(const char* s_key, unsigned int n_size);
  void Stop();

 public:
  virtual void GetIFrame() {
    vpu_.bGetIFrame = true;
  }

 public:
  // ��Ƶ����
  virtual void SetVideo(std::string sVideo) {
    v4l2_.sVideo = sVideo;
  }
  virtual void SetViSize(unsigned int nWidth, unsigned int nHeight) {
    v4l2_.nWidth = nWidth;
    v4l2_.nHeight = nHeight;
  }
  virtual void SetInput(int nInput) { // MEM IC-MEM
    v4l2_.nInput = nInput;
  }
  virtual void SetFrmFreq(int nFreq) { // ֡��
    v4l2_.nFrmFreq = nFreq;
  }

  // ��Ƶ����
  virtual void SetEncSize(unsigned int nWidth, unsigned int nHeight) {
    vpu_.nWidth =  nWidth;
    vpu_.nHeight = nHeight;
  }

  virtual void SetCodeFormat(int eFormat) { // �����ʽ
    vpu_.eEncFormat = STD_AVC;
  }

  virtual void SetBitrate(int nBitrate) { // ����
    vpu_.nBitrate = nBitrate;
  }

  virtual void SetIFrmFreq(int nIGop) {      // I֡���
    vpu_.nIGopSize = nIGop;
  }

  void Process();

 protected:
  int         chn_;
  int         i2c_;

 protected:
  CVpu        vpu_;     // ����
  CV4l2       v4l2_;    // V4L2
  CShareVideo shm_vdo_;
};
#endif  // LIBPLATFORM_CVIDEOENCODE_H