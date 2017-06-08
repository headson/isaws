#include "clivestream.h"

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
  
  // 赋值nextTask() 当应用退出时,可销毁此schedule,防止应用处理脏afterGetting
  nextTask() = envir().taskScheduler().scheduleDelayedTask(4, (TaskFunc*)afterGetting, this);
}

// H264LiveVideoServerMediaSubsession *********************************************************
H264LiveVideoServerMediaSubsession *H264LiveVideoServerMediaSubsession::createNew(
  UsageEnvironment  &env,
  Boolean           reuseFirstSource,
  VShmVideo         *p_shm_vdo) {
  return new H264LiveVideoServerMediaSubsession(env, reuseFirstSource, p_shm_vdo);
}

H264LiveVideoServerMediaSubsession::H264LiveVideoServerMediaSubsession(
  UsageEnvironment  &env,
  Boolean           reuseFirstSource,
  VShmVideo         *p_shm_vdo)
  : OnDemandServerMediaSubsession(env, reuseFirstSource)
  , p_shm_vdo_(p_shm_vdo) {
}

H264LiveVideoServerMediaSubsession::~H264LiveVideoServerMediaSubsession() {
}

FramedSource* H264LiveVideoServerMediaSubsession::createNewStreamSource(
  unsigned,
  unsigned& estBitrate) {
  //estBitrate = 96;
  estBitrate = 1000000; // kbps, estimate
  // Create the video source:
  return CamH264VideoStreamFramer::createNew(envir(), NULL, p_shm_vdo_);
}

RTPSink* H264LiveVideoServerMediaSubsession::createNewRTPSink(
  Groupsock     *rtpGroupsock,
  unsigned char rtpPayloadTypeIfDynamic,
  FramedSource  *inputSource) {
  return H264VideoRTPSink::createNew(envir(), rtpGroupsock, 96);
}

char const* H264LiveVideoServerMediaSubsession::sdpLines() {
  return fSDPLines =
           "m=video 0 RTP/AVP 96\r\n"
           "c=IN IP4 0.0.0.0\r\n"
           "b=AS:96\r\n"
           "a=rtpmap:96 H264/90000\r\n"
           "a=fmtp:96 packetization-mode=1;profile-level-id=000000;sprop-parameter-sets=H264\r\n"
           "a=control:track1\r\n";
}
