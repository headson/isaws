/************************************************************************/
/* Author      : Sober.Peng 17-06-08
/* Description :
/************************************************************************/
#ifndef LIVECAMERA_CH264LIVEVIDEO_H
#define LIVECAMERA_CH264LIVEVIDEO_H

#include "H264VideoStreamFramer.hh"

class VShmVideo;
//*********************************************************************
class CamH264VideoStreamFramer: public H264VideoStreamFramer {
 public:
  virtual ~CamH264VideoStreamFramer();

  CamH264VideoStreamFramer(
    UsageEnvironment  &env,
    FramedSource      *inputSource,
    VShmVideo         *p_shm_vdo);

  static CamH264VideoStreamFramer* createNew(
    UsageEnvironment &env,
    FramedSource     *inputSource,
    VShmVideo        *p_shm_vdo);

  virtual void doGetNextFrame();

 private:
  VShmVideo       *p_shm_vdo_;
  struct timeval  c_tm_capture_;  // 采集时间
};

#include "H264VideoRTPSink.hh"
#include "ByteStreamFileSource.hh"
#include "H264VideoStreamFramer.hh"
#include "FileServerMediaSubsession.hh"

class H264LiveVideoServerMediaSubsession: public OnDemandServerMediaSubsession {
 public:
  static H264LiveVideoServerMediaSubsession *createNew(
    UsageEnvironment  &env,
    Boolean           reuseFirstSource,
    VShmVideo         *p_shm_vdo);

 private:
  H264LiveVideoServerMediaSubsession(UsageEnvironment &env,
                                     Boolean          reuseFirstSource,
                                     VShmVideo        *p_shm_vdo);
  // called only by createNew();
  virtual ~H264LiveVideoServerMediaSubsession();

 private:
  // redefined virtual functions
  virtual FramedSource* createNewStreamSource(unsigned clientSessionId,
      unsigned& estBitrate);

  virtual RTPSink* createNewRTPSink(Groupsock     *rtpGroupsock,
                                    unsigned char rtpPayloadTypeIfDynamic,
                                    FramedSource  *inputSource);
 protected:
  virtual char const  *sdpLines();
  VShmVideo           *p_shm_vdo_;
};

// Functions to set the optimal buffer size for RTP sink objects.
// These should be called before each RTPSink is created.
#define RTP_MAX_FRAME_SIZE    400000
inline void setRTPSinkBufferSize() { OutPacketBuffer::maxSize = RTP_MAX_FRAME_SIZE; }

#endif  // LIVECAMERA_CH264LIVEVIDEO_H
