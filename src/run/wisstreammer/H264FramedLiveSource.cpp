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
  fp = fopen( fileName, "rb" );
}

H264FramedLiveSource* H264FramedLiveSource::createNew( UsageEnvironment& env,
    char const* fileName,
    unsigned preferredFrameSize /*= 0*/,
    unsigned playTimePerFrame /*= 0*/ ) {
  H264FramedLiveSource* newSource = new H264FramedLiveSource(env, fileName, preferredFrameSize, playTimePerFrame);

  return newSource;
}

H264FramedLiveSource::~H264FramedLiveSource() {
  fclose(fp);
}

long filesize(FILE *stream) {
  long curpos, length;
  curpos = ftell(stream);
  fseek(stream, 0L, SEEK_END);
  length = ftell(stream);
  fseek(stream, curpos, SEEK_SET);
  return length;
}

void H264FramedLiveSource::doGetNextFrame() {

REREAD:
  if(filesize(fp) > fMaxSize) {
    fFrameSize = fread(fTo, 1, fMaxSize,fp);
  } else {
    fFrameSize = fread(fTo, 1, filesize(fp), fp);
    fseek(fp, 0, SEEK_SET);
  }
  if (fFrameSize <= 0) {
    fseek(fp, 0, SEEK_SET);
    goto REREAD;
  }

  //fFrameSize = fMaxSize;
  nextTask() = envir().taskScheduler().scheduleDelayedTask( 0,
               (TaskFunc*)FramedSource::afterGetting, this);//��ʾ�ӳ�0�����ִ�� afterGetting ����
  return;
}

// �� H264FramedLiveSource::doGetNextFrame() �У�
// ��Ҫ���͵����ݸ��Ƶ�fTo�����ΪfMaxSize��fFrameSizeָʾʵ�ʷ��͵������Ƕ����ֽڡ�
// ������ʱ���Ǵ��ļ�����Ϊ���ԡ�
