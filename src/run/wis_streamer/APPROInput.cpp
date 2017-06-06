/*
* Copyright (C) 2005-2006 WIS Technologies International Ltd.
*
* Permission is hereby granted, free of charge, to any person obtaining a
* copy of this software and the associated README documentation file (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
* OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
// An interface to the WIS GO7007 capture device.
// Implementation

#include "APPROInput.hh"

#include "Err.hh"
#include "vshmvideo.h"

#include <time.h>
#include <fcntl.h>

#ifdef WIN32
inline int32_t gettimeofday(struct timeval *tp, void *tzp) {
  time_t clock;
  struct tm tm;
  SYSTEMTIME wtm;

  GetLocalTime(&wtm);
  tm.tm_year = wtm.wYear - 1900;
  tm.tm_mon = wtm.wMonth - 1;
  tm.tm_mday = wtm.wDay;
  tm.tm_hour = wtm.wHour;
  tm.tm_min = wtm.wMinute;
  tm.tm_sec = wtm.wSecond;
  tm.tm_isdst = -1;
  clock = mktime(&tm);
  tp->tv_sec = (long)clock;
  tp->tv_usec = wtm.wMilliseconds * 1000;
  return (0);
}
#endif

class OpenFileSource: public FramedSource {
 protected:
  OpenFileSource(UsageEnvironment& env, APPROInput& input);
  virtual ~OpenFileSource();

  //重载的虚函数，用于额外的停止操作
  virtual void  doStopGettingFrames();
  virtual int   readFromFile() = 0;

 private: // redefined virtual functions:
  virtual void  doGetNextFrame();

 private:
  static void incomingDataHandler(OpenFileSource* source);
  void        incomingDataHandler1();

 protected:
  APPROInput& fInput;
  Boolean     m_bIsCheckKeyFrame;
};

////////// VideoOpenFileSource definition //////////
class VideoOpenFileSource: public OpenFileSource {
 public:
  VideoOpenFileSource(UsageEnvironment& env, APPROInput& input);
  virtual ~VideoOpenFileSource();

 protected:
  // redefined virtual functions:
  virtual void  doStopGettingFrames();
  virtual int   readFromFile();

 private:
  int             IsStart;
  struct timeval  fPresentationTimePre;

  VShmVideo       c_shm_video_;
};

////////// AudioOpenFileSource definition //////////
class AudioOpenFileSource: public OpenFileSource {
 public:
  AudioOpenFileSource(UsageEnvironment& env, APPROInput& input);
  virtual ~AudioOpenFileSource();

 protected: // redefined virtual functions:
  virtual void  doStopGettingFrames();
  virtual int   readFromFile();
  int           getAudioData();

  unsigned int IsStart;
  struct timeval fPresentationTimePre;
};

long timevaldiff(struct timeval *starttime, struct timeval *finishtime) {
  long msec;
  msec=(finishtime->tv_sec-starttime->tv_sec)*1000;
  msec+=(finishtime->tv_usec-starttime->tv_usec)/1000;
  return msec;
}

void printErr(UsageEnvironment& env, char const* str = NULL) {
  if (str != NULL) err(env) << str;
  env << ": " << strerror(env.getErrno()) << "\n";
}

////////// APPROInput implementation //////////
APPROInput* APPROInput::createNew(UsageEnvironment& env, int n_video_chn) {
  return new APPROInput(env, n_video_chn);
}

FramedSource* APPROInput::videoSource(int e_type) {
  if (fOurVideoSource == NULL) {
    fOurVideoSource = new VideoOpenFileSource(envir(), *this);
  }

  return fOurVideoSource;
}

FramedSource* APPROInput::audioSource() {
  if (fOurAudioSource == NULL) {
    fOurAudioSource = new AudioOpenFileSource(envir(), *this);
  }

  return fOurAudioSource;
}

APPROInput::APPROInput(UsageEnvironment& env, int n_video_chn)
  : Medium(env)
  , fOurVideoSource(NULL)
  , fOurAudioSource(NULL) {
}

APPROInput::~APPROInput() {
  if( fOurVideoSource ) {
    printf("~APPROInput\n");
    delete (VideoOpenFileSource *)fOurVideoSource;
    fOurVideoSource = NULL;
  }

  if( fOurAudioSource ) {
    delete (AudioOpenFileSource *)fOurAudioSource;
    fOurAudioSource = NULL;
  }
}

#include <stdio.h>
#include <stdlib.h>
FILE *pFile = NULL;
void OpenFileHdl(void) {
  if( pFile == NULL ) {
    pFile = fopen("test.264", "rb");
    if( pFile == NULL ) {
      fprintf(stderr,"h264 open file fail!!\n");
    }
  }
}

void CloseFileHdl(void) {
  if( pFile != NULL ) {
    fclose(pFile);
    pFile = NULL;
  }
}


char FrameBuff[1024*1024];
int NAL_Search(int offsetNow) {
  unsigned long testflg = 0;
  int IsFail = 0;

  for(;;) {
    fseek(pFile, offsetNow, SEEK_SET);
    if( fread(&testflg, sizeof(testflg), 1, pFile) <=  0 ) {
      IsFail = -1;
      break;
    }

    //printf("testflg=0x%x \n",(int)testflg );

    if( testflg == 0x01000000 ) {
      break;
    }

    offsetNow++;
  }
  if( IsFail != 0 )
    return IsFail;

  return offsetNow;
}

void *GetFileFrame(int *pSize,int IsReset) {
  static int offset = 0;
  int offset1 = 0;
  int offset2 = 0;
  int framesize = 0;

  *pSize = 0;

  if( pFile == NULL )
    return NULL;

  if( IsReset == 1 ) {
    offset = 0;
    fseek(pFile, 0, SEEK_SET);
  }

  if( pFile ) {
    fseek(pFile, offset, SEEK_SET);

    offset1 = NAL_Search(offset);
    offset2 = NAL_Search(offset1+4);

    framesize = offset2 - offset1;

    /*reset position*/
    fseek(pFile, offset1, SEEK_SET);
    fread(FrameBuff, framesize, 1, pFile);

    offset = offset2;

    *pSize = framesize;
  }

  return FrameBuff;
}

////////// OpenFileSource implementation //////////
OpenFileSource::OpenFileSource(UsageEnvironment& env, APPROInput& input)
  : FramedSource(env)
  , fInput(input) {
  // printf("OpenFileSource\n");
}

OpenFileSource::~OpenFileSource() {
  printf("~OpenFileSource() 0x%08x\n", (unsigned)this);
  CloseFileHdl();
}

void OpenFileSource::doStopGettingFrames() {
  envir().taskScheduler().unscheduleDelayedTask(nextTask());	//取消当前可能存在的延迟调度任务
  nextTask() = NULL;
}

void OpenFileSource::doGetNextFrame() {
  incomingDataHandler(this);
}

void OpenFileSource::incomingDataHandler(OpenFileSource* source) {
  source->incomingDataHandler1();
}

void OpenFileSource::incomingDataHandler1() {
  if (!isCurrentlyAwaitingData())
    return; // we're not ready for the data yet

  int ret = readFromFile();
  if (ret < 0) {
    printf("In Grab Image, the source stops being readable 0x%08x\n", (unsigned)this);
    handleClosure(this);
  } else if(ret == 0) {
    nextTask() = envir().taskScheduler().scheduleDelayedTask(10000, (TaskFunc*)incomingDataHandler, this);
  } else {
    nextTask() = envir().taskScheduler().scheduleDelayedTask(0, (TaskFunc*)afterGetting, this);
  }
}

////////// VideoOpenFileSource implementation //////////
VideoOpenFileSource::VideoOpenFileSource(UsageEnvironment& env, APPROInput& input)
  : OpenFileSource(env, input)
  , IsStart(1) {
  c_shm_video_.Open((int8_t*)"video_0", sizeof(TAG_SHM_VIDEO));
}

VideoOpenFileSource::~VideoOpenFileSource() {
  printf("~VideoOpenFileSource\n");
  fInput.fOurVideoSource = NULL;
}

void VideoOpenFileSource::doStopGettingFrames() {
  OpenFileSource::doStopGettingFrames();
}

int VideoOpenFileSource::readFromFile() {
  fFrameSize = c_shm_video_.Read((int8_t*)fTo, fMaxSize);
  fNumTruncatedBytes = 0;

  /*fNumTruncatedBytes = fFrameSize - fMaxSize;
  fFrameSize = fMaxSize;*/

  // Note the timestamp and size:
  //gettimeofday(&fPresentationTime, NULL);

  if (fFrameSize > 0) {
    return 1;
  } else {
    return 0;
  }
}

////////// AudioOpenFileSource implementation //////////
extern int      audio_enable;
extern unsigned audioNumChannels;
extern unsigned audioSamplingFrequency;

AudioOpenFileSource::AudioOpenFileSource(UsageEnvironment& env, APPROInput& input)
  : OpenFileSource(env, input)
  , IsStart(1) {
}

AudioOpenFileSource::~AudioOpenFileSource() {
  fInput.fOurAudioSource = NULL;
}

void AudioOpenFileSource::doStopGettingFrames() {
  OpenFileSource::doStopGettingFrames();
}

int AudioOpenFileSource::getAudioData() {
  return 0;
}

int AudioOpenFileSource::readFromFile() {
  unsigned int timeinc;

  if (!audio_enable)
    return 0;

  // Read available audio data:
  int ret = getAudioData();
  if (ret <= 0)
    return 0;

  fFrameSize          = (unsigned)ret;
  fNumTruncatedBytes  = 0;
  timeinc = fFrameSize*1000 / audioNumChannels / (audioSamplingFrequency/1000);
  if(IsStart) {
    IsStart = 0;
    fPresentationTimePre = fPresentationTime;
    fDurationInMicroseconds = timeinc;
  } else {
    fDurationInMicroseconds = timevaldiff(&fPresentationTimePre, &fPresentationTime )*1000;
    fPresentationTimePre    = fPresentationTime;
  }

  if( fDurationInMicroseconds < timeinc) {
    unsigned long msec;
    //printf("1.fPresentationTime.tv_sec = %d fPresentationTime.tv_usec = %d \n",fPresentationTime.tv_sec,fPresentationTime.tv_usec);
    msec = fPresentationTime.tv_usec;
    msec += (timeinc - fDurationInMicroseconds);
    fPresentationTime.tv_sec += msec/1000000;
    fPresentationTime.tv_usec = msec%1000000;
    //printf("2.fPresentationTime.tv_sec = %d fPresentationTime.tv_usec = %d \n",fPresentationTime.tv_sec,fPresentationTime.tv_usec);
    fDurationInMicroseconds = timeinc;
    fPresentationTimePre = fPresentationTime;
  }

  return 1;
}

