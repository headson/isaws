/************************************************************************/
/* ����: SoberPeng 17-05-23
/* ����:
/************************************************************************/
#ifndef LIBPLATFORM_CVIDEOENCODE_H
#define LIBPLATFORM_CVIDEOENCODE_H

#include <string>

#include "vzbase/thread/thread.h"

#include "vpu.h"
#include "v4l2.h"

#include "systemv/shm/vzshm_c.h"

class CVideoEncode : public vzbase::Runnable {
 public:
  CVideoEncode();
  virtual ~CVideoEncode();

  bool Start(const char *s_vdo_key, unsigned int n_vdo_size,
             const char *s_img_key, unsigned int n_img_size);
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
  virtual void SetViSize(unsigned int nWidth, unsigned int nHeight);
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

  virtual void Run(vzbase::Thread* thread);

 protected:
  int         chn_;
  int         i2c_;

 protected:
  CVpu          vpu_;       // ����
  CV4l2         v4l2_;      // V4L2

  CShareBuffer  shm_vdo_;   // �����������
  CShareBuffer  shm_img_;   // ����ԭʼ����

 public:
  void SetPCount(unsigned int pnum, unsigned int nnum) {
    POSITIVE_NUMBER = pnum;
    NEGATIVE_NUMBER = nnum;
  }

  unsigned int  POSITIVE_NUMBER;
  unsigned int  NEGATIVE_NUMBER;
};
#endif  // LIBPLATFORM_CVIDEOENCODE_H