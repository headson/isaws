/************************************************************************/
/* Author      : SoberPeng 2017-06-09
/* Description :
/************************************************************************/
#ifndef CH264SERVERMEDIASUBSESSION_H
#define CH264SERVERMEDIASUBSESSION_H

#include "H264VideoRTPSink.hh"
#include "ByteStreamFileSource.hh"
#include "H264VideoStreamFramer.hh"
#include "FileServerMediaSubsession.hh"

class CH264LiveVideoServerMediaSubsession: public OnDemandServerMediaSubsession {
 public:
  static CH264LiveVideoServerMediaSubsession *createNew(
    UsageEnvironment  &env,
    Boolean           reuseFirstSource,
    void             *p_shm_vdo);

 private:
  CH264LiveVideoServerMediaSubsession(UsageEnvironment &env,
                                     Boolean            reuseFirstSource,
                                     void              *p_shm_vdo);
  // called only by createNew();
  virtual ~CH264LiveVideoServerMediaSubsession();

 private:
  // redefined virtual functions
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
      unsigned& estBitrate);

  virtual RTPSink* createNewRTPSink(Groupsock     *rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
                                    FramedSource  *inputSource);
 protected:
  virtual char const  *sdpLines();
  void                *p_shm_vdo_;
};

#endif  // CH264SERVERMEDIASUBSESSION_H
