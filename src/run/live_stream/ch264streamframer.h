/************************************************************************/
/* Author      : Sober.Peng 17-06-26
/* Description : 
/************************************************************************/
#ifndef LIVE_STREAM_CH264STREAMFRAMER_H
#define LIVE_STREAM_CH264STREAMFRAMER_H

#include "MediaSink.hh"
#include "H264VideoStreamFramer.hh"

class VShmVideo;
//*********************************************************************
class CH264VideoStreamFramer: public H264VideoStreamFramer {
 public:
  virtual ~CH264VideoStreamFramer();

  CH264VideoStreamFramer(
    UsageEnvironment  &env,
    FramedSource      *inputSource,
    VShmVideo         *p_shm_vdo);

  static CH264VideoStreamFramer* createNew(
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

#endif  // LIVE_STREAM_CH264STREAMFRAMER_H
