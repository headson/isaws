
#include <stdlib.h>

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

#include "shm/vshmvideo.h"
#include "ch264streamframer.h"
#include "ch264servermediasubsession.h"

UsageEnvironment* env;

// To make the second and subsequent client for each stream reuse the same
// input stream as the first client (rather than playing the file from the
// start for each client), change the following "False" to "True":
Boolean reuseFirstSource = False;

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
                           char const* streamName); // fwd


int main(int argc, char** argv) {
  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  env = BasicUsageEnvironment::createNew(*scheduler);

  UserAuthenticationDatabase* authDB = NULL;
  // Create the RTSP server:
  RTSPServer* rtspServer = RTSPServer::createNew(*env, 554, authDB);
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }

  char const* descriptionString
    = "Session streamed by \"testOnDemandRTSPServer\"";

  VShmVideo v_chm_vdo;
  v_chm_vdo.Open((int8_t*)("video_0"), sizeof(TAG_SHM_VIDEO));

  {
    char const* s_stream_name = "live";
    ServerMediaSession* sms = 
      ServerMediaSession::createNew(
      *env, s_stream_name, s_stream_name, descriptionString);
    sms->addSubsession(H264LiveVideoServerMediaSubsession::createNew(
      *env, reuseFirstSource, &v_chm_vdo));
    rtspServer->addServerMediaSession(sms);

    announceStream(rtspServer, sms, s_stream_name);
  }

  env->taskScheduler().doEventLoop(); // does not return
  return 0; // only to prevent compiler warning
}

static void announceStream(RTSPServer* rtspServer, ServerMediaSession* sms,
                           char const* streamName) {
  char* url = rtspServer->rtspURL(sms);
  UsageEnvironment& env = rtspServer->envir();
  env << "Play using the URL \"" << url << "\"\n";
  delete[] url;
}
