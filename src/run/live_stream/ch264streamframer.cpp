#include "ch264streamframer.h"

#include "GroupsockHelper.hh"

//#include "vzbase/helper/stdafx.h"

// CamH264VideoStreamFramer *********************************************************
CH264VideoStreamFramer::CH264VideoStreamFramer(UsageEnvironment& env,
    FramedSource* inputSource, void* shm_vdo)
  : H264VideoStreamFramer(env, inputSource, False, False)
  , shm_vdo_((CShareBuffer*)shm_vdo) {
  fFrameRate = 16.0;
}

CH264VideoStreamFramer::~CH264VideoStreamFramer() {
  printf("%s[%d]\n", __FUNCTION__, __LINE__);
  fflush(stdout);
}

CH264VideoStreamFramer* CH264VideoStreamFramer::createNew(
  UsageEnvironment  &env,
  FramedSource      *inputSource,
  void              *p_shm_vdo) {
  setRTPSinkBufferSize();

  CH264VideoStreamFramer* fr =
    new CH264VideoStreamFramer(env, inputSource, p_shm_vdo);
  return fr;
}

// 在 H264FramedLiveSource::doGetNextFrame() 中，
// 将要发送的内容复制到fTo，最大为fMaxSize，fFrameSize指示实际发送的内容是多少字节。
void CH264VideoStreamFramer::doGetNextFrame() {
  gettimeofday(&fPresentationTime, NULL);

  int n_r_size = 0;
  if (shm_vdo_) {
    n_r_size = shm_vdo_->ReadH264((char*)fTo+n_r_size, fMaxSize-n_r_size,
                                  (unsigned int*)&c_tm_capture_.tv_sec,
                                  (unsigned int*)&c_tm_capture_.tv_usec);
  }
  if (n_r_size < 0) {
    n_r_size = 0;
    printf("shm read failed.");
  }
  fFrameSize = n_r_size;
  fNumTruncatedBytes = 0;

  fPresentationTime.tv_sec = c_tm_capture_.tv_sec;
  fPresentationTime.tv_usec = c_tm_capture_.tv_usec;
  //usleep(2);

  // 赋值nextTask() 当应用退出时,可销毁此schedule,防止应用处理脏afterGetting
  nextTask() = envir().taskScheduler().scheduleDelayedTask(5, (TaskFunc*)afterGetting, this);
}
