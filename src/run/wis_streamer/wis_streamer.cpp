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
// An application that streams audio/video captured by a WIS GO7007,
// using a built-in RTSP server.
// main program

#include <signal.h>
#include <BasicUsageEnvironment.hh>
#include <liveMedia.hh>
#include "Err.hh"
#include "WISH264VideoServerMediaSubsession.hh"
#include "WISMPEG4VideoServerMediaSubsession.hh"
#include "WISPCMAudioServerMediaSubsession.hh"
#include <GroupsockHelper.hh>

//#define ACCESS_CONTROL

char watchVariable = 0;

portNumBits g_rtsp_srv_port   = 554;
char const* g_chn_name_main   = "h264";
char const* g_stream_description = "RTSP/RTP stream from VZ Smart-IPC";

int VideoBitrate        = 1500000;
int audioOutputBitrate  = 128000;

int      audio_enable           = 0;
unsigned audioNumChannels       = 1;
unsigned audioSamplingFrequency = 8000;

//建立标准的
static int iCreateSingle(UsageEnvironment *pEnv, RTSPServer *pRTSPServer) {
  APPROInput *pVideoInputDevice = APPROInput::createNew(*pEnv, 0);
  if (pVideoInputDevice == NULL) {
    err(*pEnv) << "Failed to create H264 input device\n";
    return(-1);
  }

  //建立一个服务器媒体会话
  ServerMediaSession *pSMS = ServerMediaSession::createNew(*pEnv,
                             g_chn_name_main, g_chn_name_main, g_stream_description, False);

  // 
  pSMS->addSubsession(WISH264VideoServerMediaSubsession::createNew(*pEnv, *pVideoInputDevice, VideoBitrate));

#ifdef AUDIO_STREAM
  pSMS->addSubsession(WISPCMAudioServerMediaSubsession::createNew(sms->envir(), *pVideoInputDevice));
#endif

  pRTSPServer->addServerMediaSession(pSMS);

  char *pURL = pRTSPServer->rtspURL(pSMS);
  *pEnv << "Play this stream using the URL:\n\t" << pURL << "\n";
  delete[] pURL;

#ifdef SEPARATE_INFO_STREAM
  pSMS = ServerMediaSession::createNew(*pEnv,
                                       VzInfoStreamName, VzInfoStreamName, g_stream_description, False);
  pSMS->addSubsession(VzInfoServerMediaSubsession::createNew(pSMS->envir(), 0));
  pRTSPServer->addServerMediaSession(pSMS);

  pURL = pRTSPServer->rtspURL(pSMS);
  *pEnv << "Play this stream using the URL:\n\t" << pURL << "\n";
  delete[] pURL;
#endif

  return(0);
}

int main(int argc, char** argv) {
  g_rtsp_srv_port = 8557;

  // Begin by setting up our usage environment:
  TaskScheduler* scheduler = BasicTaskScheduler::createNew();
  UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);

  *env << "Initializing...\n";

  // Create the RTSP server:
  RTSPServer* rtspServer = NULL;
  UserAuthenticationDatabase *authDB = NULL;
#ifdef ACCESS_CONTROL
  *env << "Auth Data\n";
  authDB = new UserAuthenticationDatabase;
  authDB->addUserRecord("", "");
  authDB->addUserRecord("VisionZenith", "147258369");
#else
  printf("No access control\n");
#endif
  // Normal case: Streaming from a built-in RTSP server:
  rtspServer = RTSPServer::createNew(*env, g_rtsp_srv_port, authDB, 20);
  if (rtspServer == NULL) {
    *env << "Failed to create RTSP server: " << env->getResultMsg() << "\n";
    exit(1);
  }

  if (iCreateSingle(env, rtspServer) < 0)
    return(-1);

  // Begin the LIVE555 event loop:
  env->taskScheduler().doEventLoop(&watchVariable);

  Medium::close(rtspServer); // will also reclaim "sms" and its "ServerMediaSubsession"s

  env->reclaim();

#ifdef ACCESS_CONTROL
  delete authDB;
#endif
  delete scheduler;
  return 0;
}
