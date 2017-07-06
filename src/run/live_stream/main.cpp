
#include <stdlib.h>
#include "vzbase/helper/stdafx.h"

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

#include "systemv/shm/vzshm_c.h"

#include "ch264streamframer.h"
#include "ch264servermediasubsession.h"
#include "cpcmstreamframer.h"
#include "cpcmservermediasubsession.h"

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

  void *p_shm_vdo = Shm_Create(SHM_VIDEO_0, SHM_VIDEO_0_SIZE);
  void *p_shm_ado = Shm_Create(SHM_AUDIO_0, SHM_AUDIO_0_SIZE);
  {
    char const* s_stream_name = "live";
    ServerMediaSession* sms = 
      ServerMediaSession::createNew(
      *env, s_stream_name, s_stream_name, descriptionString);
    sms->addSubsession(CH264LiveVideoServerMediaSubsession::createNew(
      *env, reuseFirstSource, p_shm_vdo));
    //sms->addSubsession(CPCMAudioServerMediaSubsession::createNew(
    //  *env, reuseFirstSource, p_shm_ado));
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
