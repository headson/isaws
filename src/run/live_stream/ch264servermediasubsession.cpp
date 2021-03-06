#include "ch264servermediasubsession.h"

#include "GroupsockHelper.hh"

#include "ch264streamframer.h"

#include "systemv/shm/vzshm_c.h"
#include "vzbase/helper/stdafx.h"

CH264LiveVideoServerMediaSubsession *CH264LiveVideoServerMediaSubsession::createNew(
  UsageEnvironment  &env,
  Boolean           reuseFirstSource,
  void             *p_shm_vdo) {
  return new CH264LiveVideoServerMediaSubsession(env, reuseFirstSource, p_shm_vdo);
}

CH264LiveVideoServerMediaSubsession::CH264LiveVideoServerMediaSubsession(
  UsageEnvironment  &env,
  Boolean           reuseFirstSource,
  void             *p_shm_vdo)
  : OnDemandServerMediaSubsession(env, reuseFirstSource)
  , p_shm_vdo_(p_shm_vdo) {
}

CH264LiveVideoServerMediaSubsession::~CH264LiveVideoServerMediaSubsession() {
}

FramedSource* CH264LiveVideoServerMediaSubsession::createNewStreamSource(
  unsigned,
  unsigned& estBitrate) {
  //estBitrate = 96;
  estBitrate = 1000000; // kbps, estimate
  // Create the video source:
  return CH264VideoStreamFramer::createNew(envir(), NULL, p_shm_vdo_);
}

RTPSink* CH264LiveVideoServerMediaSubsession::createNewRTPSink(Groupsock     *rtpGroupsock,
    unsigned char rtpPayloadTypeIfDynamic,
    FramedSource  *inputSource) {
  return H264VideoRTPSink::createNew(envir(), rtpGroupsock, 96);
}

char const* CH264LiveVideoServerMediaSubsession::sdpLines() {
  return fSDPLines =
           "m=video 0 RTP/AVP 96\r\n"
           "c=IN IP4 0.0.0.0\r\n"
           "b=AS:96\r\n"
           "a=rtpmap:96 H264/90000\r\n"
           "a=fmtp:96 packetization-mode=1;profile-level-id=000000;sprop-parameter-sets=H264\r\n"
           "a=control:track1\r\n";
}
