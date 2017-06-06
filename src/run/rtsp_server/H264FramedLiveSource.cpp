/*
 * Filename:  H264FramedLiveSource.cpp
 * Auther:  mlj
 * Create date: 2013/ 1/22
 */

#include "H264FramedLiveSource.h"

H264FramedLiveSource::H264FramedLiveSource( UsageEnvironment& env,
    char const* fileName,
    unsigned preferredFrameSize,
    unsigned playTimePerFrame )
  : FramedSource(env) {
  //fp = fopen( fileName, "rb" );

  v_shm_vdo_.Open((int8_t*)"video_0", sizeof(TAG_SHM_VIDEO));
}

H264FramedLiveSource* H264FramedLiveSource::createNew( UsageEnvironment& env,
    char const* fileName,
    unsigned preferredFrameSize /*= 0*/,
    unsigned playTimePerFrame /*= 0*/ ) {
  setVideoRTPSinkBufferSize();
  H264FramedLiveSource* newSource = 
    new H264FramedLiveSource(env, fileName, preferredFrameSize, playTimePerFrame);

  return newSource;
}

H264FramedLiveSource::~H264FramedLiveSource() {
  //fclose(fp);
}

//unsigned H264FramedLiveSource::maxFrameSize() const {
//  return 1024*1024;
//}

void H264FramedLiveSource::doGetNextFrame() {

  fFrameSize = v_shm_vdo_.Read((int8_t*)fTo, fMaxSize);
  fNumTruncatedBytes = 0;  

  static int i = 0;
  printf("read video %d \t %d.\n", i++, fFrameSize);

  nextTask() = envir().taskScheduler().scheduleDelayedTask(20,
               (TaskFunc*)FramedSource::afterGetting, this);//��ʾ�ӳ�0�����ִ�� afterGetting ����
  return;
}

// �� H264FramedLiveSource::doGetNextFrame() �У�
// ��Ҫ���͵����ݸ��Ƶ�fTo�����ΪfMaxSize��fFrameSizeָʾʵ�ʷ��͵������Ƕ����ֽڡ�
// ������ʱ���Ǵ��ļ�����Ϊ���ԡ�
