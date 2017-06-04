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
               (TaskFunc*)FramedSource::afterGetting, this);//表示延迟0秒后再执行 afterGetting 函数
  return;
}

// 在 H264FramedLiveSource::doGetNextFrame() 中，
// 将要发送的内容复制到fTo，最大为fMaxSize，fFrameSize指示实际发送的内容是多少字节。
// 这里暂时还是从文件读作为测试。
