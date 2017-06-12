#include "ch264streamframer.h"

#include "GroupsockHelper.hh"

#include "shm/vshmvideo.h"

// CamH264VideoStreamFramer *********************************************************
CamH264VideoStreamFramer::CamH264VideoStreamFramer(UsageEnvironment& env,
    FramedSource* inputSource, VShmVideo* p_shm_vdo)
  : H264VideoStreamFramer(env, inputSource, False, False)
  , p_shm_vdo_(p_shm_vdo) {
  fFrameRate = 25.0;
}

CamH264VideoStreamFramer::~CamH264VideoStreamFramer() {
  printf("%s[%d]\n", __FUNCTION__, __LINE__);
  fflush(stdout);
}

CamH264VideoStreamFramer* CamH264VideoStreamFramer::createNew(
  UsageEnvironment  &env,
  FramedSource      *inputSource,
  VShmVideo         *p_shm_vdo) {
  setRTPSinkBufferSize();

  CamH264VideoStreamFramer* fr =
    new CamH264VideoStreamFramer(env, inputSource, p_shm_vdo);
  return fr;
}

// 在 H264FramedLiveSource::doGetNextFrame() 中，
// 将要发送的内容复制到fTo，最大为fMaxSize，fFrameSize指示实际发送的内容是多少字节。
void CamH264VideoStreamFramer::doGetNextFrame() {
  gettimeofday(&fPresentationTime, NULL);

  fFrameSize = p_shm_vdo_->Read((int8_t*)fTo, fMaxSize, &c_tm_capture_);
  fNumTruncatedBytes = 0;

  fPresentationTime.tv_sec = c_tm_capture_.tv_sec;
  fPresentationTime.tv_usec = c_tm_capture_.tv_usec;
  //usleep(2);

  // 赋值nextTask() 当应用退出时,可销毁此schedule,防止应用处理脏afterGetting
  nextTask() = envir().taskScheduler().scheduleDelayedTask(5*1000, (TaskFunc*)afterGetting, this);
}
