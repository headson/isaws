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

// �� H264FramedLiveSource::doGetNextFrame() �У�
// ��Ҫ���͵����ݸ��Ƶ�fTo�����ΪfMaxSize��fFrameSizeָʾʵ�ʷ��͵������Ƕ����ֽڡ�
void CamH264VideoStreamFramer::doGetNextFrame() {
  gettimeofday(&fPresentationTime, NULL);

  fFrameSize = p_shm_vdo_->Read((int8_t*)fTo, fMaxSize, &c_tm_capture_);
  fNumTruncatedBytes = 0;

  fPresentationTime.tv_sec = c_tm_capture_.tv_sec;
  fPresentationTime.tv_usec = c_tm_capture_.tv_usec;
  //usleep(2);

  // ��ֵnextTask() ��Ӧ���˳�ʱ,�����ٴ�schedule,��ֹӦ�ô�����afterGetting
  nextTask() = envir().taskScheduler().scheduleDelayedTask(5*1000, (TaskFunc*)afterGetting, this);
}
