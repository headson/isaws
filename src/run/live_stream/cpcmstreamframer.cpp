/************************************************************************/
/* Author      : Sober.Peng 17-06-26
/* Description :
/************************************************************************/
#include "cpcmstreamframer.h"

#include "ch264streamframer.h"

#include "GroupsockHelper.hh"

#include "sharemem/vshmvideo.h"

CPcmAudioStreamFramer::CPcmAudioStreamFramer(UsageEnvironment &env,
    FramedSource *inputSource,
    VShmVideo *p_shm_vdo)
  : FramedSource(env)
  , p_shm_vdo_(p_shm_vdo) {
}

CPcmAudioStreamFramer::~CPcmAudioStreamFramer() {
  printf("%s[%d]\n", __FUNCTION__, __LINE__);
  fflush(stdout);
}

CPcmAudioStreamFramer* CPcmAudioStreamFramer::createNew(UsageEnvironment &env,
    FramedSource *inputSource,
    VShmVideo *p_shm_vdo) {
  setRTPSinkBufferSize();

  CPcmAudioStreamFramer* fr =
    new CPcmAudioStreamFramer(env, inputSource, p_shm_vdo);
  return fr;
}

void CPcmAudioStreamFramer::doGetNextFrame() {
  gettimeofday(&fPresentationTime, NULL);

  fFrameSize = p_shm_vdo_->Read((int8_t*)fTo, fMaxSize, &c_tm_capture_);
  fNumTruncatedBytes = 0;

  fPresentationTime.tv_sec = c_tm_capture_.tv_sec;
  fPresentationTime.tv_usec = c_tm_capture_.tv_usec;
  //usleep(2);

  // 赋值nextTask() 当应用退出时,可销毁此schedule,防止应用处理脏afterGetting
  nextTask() = envir().taskScheduler().scheduleDelayedTask(
                 5 * 1000, (TaskFunc*)afterGetting, this);
}

