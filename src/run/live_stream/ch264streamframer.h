/************************************************************************/
/* Author      : SoberPeng 2017-06-09
/* Description :
/************************************************************************/
#ifndef CH264STREAMFRAMER_H
#define CH264STREAMFRAMER_H

#include "MediaSink.hh"
#include "H264VideoStreamFramer.hh"

class VShmVideo;
//*********************************************************************
class CamH264VideoStreamFramer: public H264VideoStreamFramer {
 public:
  virtual ~CamH264VideoStreamFramer();

  CamH264VideoStreamFramer(
    UsageEnvironment  &env,
    FramedSource      *inputSource,
    VShmVideo         *p_shm_vdo);

  static CamH264VideoStreamFramer* createNew(
    UsageEnvironment &env,
    FramedSource     *inputSource,
    VShmVideo        *p_shm_vdo);

  virtual void doGetNextFrame();

 private:
  VShmVideo       *p_shm_vdo_;
  struct timeval  c_tm_capture_;  // 采集时间
};
// Functions to set the optimal buffer size for RTP sink objects.
// These should be called before each RTPSink is created.
#define RTP_MAX_FRAME_SIZE    400000
inline void setRTPSinkBufferSize() { OutPacketBuffer::maxSize = RTP_MAX_FRAME_SIZE; }
#endif  // CH264STREAMFRAMER_H
