/************************************************************************/
/* Author      : Sober.Peng 17-06-26
/* Description :
/************************************************************************/
#ifndef LIVE_STREAM_CPCMSERVERMEDIASUBSESSION_H
#define LIVE_STREAM_CPCMSERVERMEDIASUBSESSION_H

#include <OnDemandServerMediaSubsession.hh>

class VShmVideo;
class CPCMAudioServerMediaSubsession : public OnDemandServerMediaSubsession {
 public:
  static CPCMAudioServerMediaSubsession *createNew(
    UsageEnvironment  &env, Boolean reuseFirstSource, VShmVideo *p_shm_vdo);

 private:
  CPCMAudioServerMediaSubsession(
    UsageEnvironment &env, Boolean reuseFirstSource, VShmVideo *p_shm_vdo);
  // called only by createNew();
  virtual ~CPCMAudioServerMediaSubsession();

 private:
  // redefined virtual functions
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId, unsigned& estBitrate);

  virtual RTPSink* createNewRTPSink(Groupsock     *rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
                                    FramedSource  *inputSource);
 protected:
  unsigned             fSamplingFrequency;
  unsigned             fNumChannels;
  unsigned             fEstimatedKbps;
  VShmVideo           *p_shm_vdo_;
};

#endif  // LIVE_STREAM_CPCMSERVERMEDIASUBSESSION_H
