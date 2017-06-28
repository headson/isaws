/************************************************************************/
/* Author      : Sober.Peng 17-06-26
/* Description :
/************************************************************************/
#include "cpcmservermediasubsession.h"
#include <liveMedia.hh>

#include "ch264streamframer.h"
#include "cpcmstreamframer.h"

unsigned audioSamplingFrequency = 8000;
unsigned audioNumChannels       = 1;
unsigned audioOutputBitrate     = 128000;

CPCMAudioServerMediaSubsession*
CPCMAudioServerMediaSubsession::createNew(
  UsageEnvironment &env, Boolean reuseFirstSource, void *p_shm_vdo) {
  return new CPCMAudioServerMediaSubsession(env, reuseFirstSource, p_shm_vdo);
}

CPCMAudioServerMediaSubsession::CPCMAudioServerMediaSubsession(
  UsageEnvironment& env, Boolean reuseFirstSource, void *p_shm_vdo)
  : OnDemandServerMediaSubsession(env, reuseFirstSource)
  , p_shm_vdo_(p_shm_vdo) {

  fEstimatedKbps = (audioSamplingFrequency*8*audioNumChannels + 500)/1000;
}

CPCMAudioServerMediaSubsession::~CPCMAudioServerMediaSubsession() {
}

FramedSource* CPCMAudioServerMediaSubsession::createNewStreamSource(unsigned /*clientSessionId*/, unsigned& estBitrate) {
  estBitrate = fEstimatedKbps;
  return CPcmAudioStreamFramer::createNew(envir(), NULL, p_shm_vdo_);
}

RTPSink* CPCMAudioServerMediaSubsession::createNewRTPSink(Groupsock* rtpGroupsock,
    unsigned char rtpPayloadTypeIfDynamic,
    FramedSource *inputSource) {
  setRTPSinkBufferSize(); // 必须用Video的设置，保证RTPSinkBufferSize足够大来装一些东西，算是原版的BUG吧
  
  audioOutputBitrate = audioSamplingFrequency * 16/*bits-per-sample*/ * audioNumChannels / 2;

  if (audioSamplingFrequency == 16000) {
    return SimpleRTPSink::createNew(envir(), rtpGroupsock, 96,
                                    audioSamplingFrequency, "audio", "PCMU", audioNumChannels);
  } else {
    return SimpleRTPSink::createNew(envir(), rtpGroupsock, 0,
                                    audioSamplingFrequency, "audio", "PCMU", audioNumChannels);
  }
}



