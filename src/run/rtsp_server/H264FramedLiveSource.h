/*
 * Filename: H264FramedLiveSource.hh
 * Auther: chenbin
 * Create date: 2013/ 1/22
 */

#ifndef _H264FRAMEDLIVESOURCE_HH
#define _H264FRAMEDLIVESOURCE_HH

#include <MediaSink.hh>
#include <FramedSource.hh>

#include "vshmvideo.h"

class H264FramedLiveSource : public FramedSource
{
public:
    static H264FramedLiveSource* createNew(UsageEnvironment& env,
        char const* fileName,
        unsigned preferredFrameSize = 0,
        unsigned playTimePerFrame = 0); 

protected:
    H264FramedLiveSource(UsageEnvironment& env,
        char const* fileName, 
        unsigned preferredFrameSize,
        unsigned playTimePerFrame);
    // called only by createNew()
    ~H264FramedLiveSource();

    //virtual unsigned maxFrameSize() const;

private:
    // redefined virtual functions:
    virtual void doGetNextFrame();
    int TransportData( unsigned char* to, unsigned maxSize );

protected:
    FILE        *fp;
    VShmVideo   v_shm_vdo_;
};

// Functions to set the optimal buffer size for RTP sink objects.
// These should be called before each RTPSink is created.
#define AUDIO_MAX_FRAME_SIZE 20480
#define VIDEO_MAX_FRAME_SIZE 400000
inline void setAudioRTPSinkBufferSize() { OutPacketBuffer::maxSize = AUDIO_MAX_FRAME_SIZE; }
inline void setVideoRTPSinkBufferSize() { OutPacketBuffer::maxSize = VIDEO_MAX_FRAME_SIZE; }


#endif