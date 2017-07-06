/************************************************************************/
/* Author      : Sober.Peng 17-06-26
/* Description :
/************************************************************************/
#include "cpcmstreamframer.h"

#include "ch264streamframer.h"
#include "GroupsockHelper.hh"

#include "systemv/shm/vzshm_c.h"
//#include "vzbase/helper/stdafx.h"

CPcmAudioStreamFramer::CPcmAudioStreamFramer(UsageEnvironment &env,
    FramedSource *inputSource,
    void         *p_shm_vdo)
  : FramedSource(env)
  , p_shm_vdo_(p_shm_vdo) {
}

CPcmAudioStreamFramer::~CPcmAudioStreamFramer() {
  printf("%s[%d]\n", __FUNCTION__, __LINE__);
  fflush(stdout);
}

CPcmAudioStreamFramer* CPcmAudioStreamFramer::createNew(UsageEnvironment &env,
    FramedSource *inputSource,
    void         *p_shm_vdo) {
  setRTPSinkBufferSize();

  CPcmAudioStreamFramer* fr =
    new CPcmAudioStreamFramer(env, inputSource, p_shm_vdo);
  return fr;
}

void CPcmAudioStreamFramer::doGetNextFrame() {
  gettimeofday(&fPresentationTime, NULL);

  int n_r_size = Shm_Read(p_shm_vdo_, 
                          (char*)fTo, fMaxSize,
                          (unsigned int*)&c_tm_capture_.tv_sec, 
                          (unsigned int*)&c_tm_capture_.tv_usec);
  if (n_r_size <= 0) {
    n_r_size = 0;
    printf("shm read failed.");
  }
  fFrameSize = n_r_size;
  fNumTruncatedBytes = 0;

  fPresentationTime.tv_sec = c_tm_capture_.tv_sec;
  fPresentationTime.tv_usec = c_tm_capture_.tv_usec;

  // 赋值nextTask() 当应用退出时,可销毁此schedule,防止应用处理脏afterGetting
  nextTask() = envir().taskScheduler().scheduleDelayedTask(
                 5 * 1000, (TaskFunc*)afterGetting, this);
}

