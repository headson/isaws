/************************************************************************/
/* Author      : Sober.Peng 17-06-26
/* Description : 
/************************************************************************/
#ifndef LIVE_STREAM_CPCMSTREAMFRAMER_H
#define LIVE_STREAM_CPCMSTREAMFRAMER_H

#include "MediaSink.hh"
#include "FramedSource.hh"

#include "systemv/shm/vzshm_c.h"

class CPcmAudioStreamFramer : public FramedSource
{
public:
  virtual ~CPcmAudioStreamFramer();

  CPcmAudioStreamFramer(
    UsageEnvironment  &env,
    FramedSource      *inputSource,
    void              *shm_ado);

  static CPcmAudioStreamFramer* createNew(
    UsageEnvironment &env,
    FramedSource     *inputSource,
    void              *p_shm_vdo);

  virtual void doGetNextFrame();

private:
  CShareBuffer     *shm_ado_;
  struct timeval    c_tm_capture_;  // 采集时间
};

#endif  // LIVE_STREAM_CPCMSTREAMFRAMER_H
