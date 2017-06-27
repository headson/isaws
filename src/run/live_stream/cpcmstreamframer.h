/************************************************************************/
/* Author      : Sober.Peng 17-06-26
/* Description : 
/************************************************************************/
#ifndef LIVE_STREAM_CPCMSTREAMFRAMER_H
#define LIVE_STREAM_CPCMSTREAMFRAMER_H

#include "MediaSink.hh"
#include "FramedSource.hh"

class VShmVideo;
class CPcmAudioStreamFramer : public FramedSource
{
public:
  virtual ~CPcmAudioStreamFramer();

  CPcmAudioStreamFramer(
    UsageEnvironment  &env,
    FramedSource      *inputSource,
    VShmVideo         *p_shm_vdo);

  static CPcmAudioStreamFramer* createNew(
    UsageEnvironment &env,
    FramedSource     *inputSource,
    VShmVideo        *p_shm_vdo);

  virtual void doGetNextFrame();

private:
  VShmVideo       *p_shm_vdo_;
  struct timeval  c_tm_capture_;  // 采集时间
};

#endif  // LIVE_STREAM_CPCMSTREAMFRAMER_H
