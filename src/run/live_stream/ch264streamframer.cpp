#include "ch264streamframer.h"

#include "GroupsockHelper.hh"

#include "systemv/vzshm_c.h"
//#include "vzbase/helper/stdafx.h"

// CamH264VideoStreamFramer *********************************************************
CH264VideoStreamFramer::CH264VideoStreamFramer(UsageEnvironment& env,
    FramedSource* inputSource, void* p_shm_vdo)
  : H264VideoStreamFramer(env, inputSource, False, False)
  , p_shm_vdo_(p_shm_vdo) {
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

// �� H264FramedLiveSource::doGetNextFrame() �У�
// ��Ҫ���͵����ݸ��Ƶ�fTo�����ΪfMaxSize��fFrameSizeָʾʵ�ʷ��͵������Ƕ����ֽڡ�
void CH264VideoStreamFramer::doGetNextFrame() {
  gettimeofday(&fPresentationTime, NULL);

  int n_r_size = 0;
  if (p_shm_vdo_) {
    n_r_size = Shm_Read(p_shm_vdo_,
                        (char*)fTo, fMaxSize,
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

  // ��ֵnextTask() ��Ӧ���˳�ʱ,�����ٴ�schedule,��ֹӦ�ô�����afterGetting
  nextTask() = envir().taskScheduler().scheduleDelayedTask(5*1000, (TaskFunc*)afterGetting, this);
}
