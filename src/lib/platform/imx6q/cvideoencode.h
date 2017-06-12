/************************************************************************/
/* 作者: SoberPeng 17-05-23
/* 描述:
/************************************************************************/
#ifndef LIBPLATFORM_CVIDEOENCODE_H
#define LIBPLATFORM_CVIDEOENCODE_H

#include "inc/vtypes.h"

#include <string>

#include "vpu.h"
#include "v4l2.h"

class CVideoEncode {
 public:
  CVideoEncode();
  virtual ~CVideoEncode();

  int32_t Start();
  void    Stop();

 public:
  virtual void GetIFrame() {
    m_cVpu.bGetIFrame = true;
  }

 public:
  // 视频输入
  virtual void SetVideo(std::string sVideo) {
    m_cV4l2.sVideo = sVideo;
  }
  virtual void SetViSize(uint16_t nWidth, uint16_t nHeight) {
    m_cV4l2.nWidth = nWidth;
    m_cV4l2.nHeight = nHeight;
  }
  virtual void SetInput(int32_t nInput) { // MEM IC-MEM
    m_cV4l2.nInput = nInput;
  }
  virtual void SetFrmFreq(int32_t nFreq) { // 帧率
    m_cV4l2.nFrmFreq = nFreq;
  }

  // 视频编码
  virtual void SetEncSize(uint16_t nWidth, uint16_t nHeight) {
    m_cVpu.nWidth = nWidth;
    m_cVpu.nHeight = nHeight;
  }

  virtual void SetCodeFormat(int32_t eFormat) { // 编码格式
    m_cVpu.eEncFormat = STD_AVC;
  }

  virtual void SetBitrate(int32_t nBitrate) { // 码流
    m_cVpu.nBitrate = nBitrate;
  }

  virtual void SetIFrmFreq(int32_t nIGop) {      // I帧间隔
    m_cVpu.nIGopSize = nIGop;
  }

  void Process();

 protected:
  int32_t     n_chn_;
  int         m_i2c;

 protected:
  int8_t*     m_pPkgBuffer;

 private:
  CVpu        m_cVpu;         // 编码
  CV4l2       m_cV4l2;        // V4L2
};
#endif  // LIBPLATFORM_CVIDEOENCODE_H