#include "vzbase/helper/stdafx.h"
#include "httpsender/server/senderserver.h"
#include "vzbase/thread/thread.h"

#include <signal.h>


hs::SenderServer sender_server;

//const unsigned MAX_TEST_CASE_SIZE = 10;
//unsigned int test_case_size = 0;
//
//const char POST_DATA[] =
//  "{"
//  "   \"AlarmInfoPlate\" : {"
//  "      \"channel\" : 0,"
//  "      \"deviceName\" : \"IVS\","
//  "      \"ipaddr\" : \"192.168.4.71\","
//  "      \"result\" : {"
//  "         \"PlateResult\" : {"
//  "            \"bright\" : 0,"
//  "            \"carBright\" : 0,"
//  "            \"carColor\" : 0,"
//  "            \"colorType\" : 0,"
//  "            \"colorValue\" : 0,"
//  "            \"confidence\" : 0,"
//  "            \"direction\" : 0,"
//  "            \"imagePath\" : \"%2Fmmc%2FVzIPCCap%2F2016_09_03%2F1750152404__%CE%DE_.jpg\","
//  "            \"license\" : \"_нч_\","
//  "            \"location\" : {"
//  "               \"RECT\" : {"
//  "                  \"bottom\" : 0,"
//  "                  \"left\" : 0,"
//  "                  \"right\" : 0,"
//  "                  \"top\" : 0"
//  "               }"
//  "            },"
//  "            \"timeStamp\" : {"
//  "               \"Timeval\" : {"
//  "                  \"sec\" : 1472896215,"
//  "                  \"usec\" : 245208"
//  "               }"
//  "            },"
//  "            \"timeUsed\" : 0,"
//  "            \"triggerType\" : 4,"
//  "            \"type\" : 0"
//  "         }"
//  "      },"
//  "      \"serialno\" : \"7a827083-62ad93cd\""
//  "   }"
//  "}";
//void HttpConnComplete(hs::HttpConn::Ptr http_conn,
//                      const char *data,
//                      std::size_t data_size,
//                      const boost::system::error_code& err,
//                      hs::CurlServices::Ptr curl_services) {
//  LOG(INFO).write(data, data_size);
//  test_case_size++;
//
//  LOG(WARNING) << "test_case_size = " << test_case_size;
//
//}

int SignalHandle(int n_signal, short events, const void *p_usr_arg) {
  if (n_signal == SIGINT) {
    LOG(L_WARNING) << "revive SIGINT,End of the http_sender";
  } else if (n_signal == SIGTERM) {
    LOG(L_WARNING) << "revive SIGTERM,End of the http_sender";
  } else if (n_signal == SIGSEGV) {
    LOG(L_ERROR) << "revive SIGSEGV,End of the http_sender";
  } else if (n_signal == SIGABRT) {
    LOG(L_ERROR) << "revive SIGSEGV,End of the http_sender";
  }
#ifdef POSIX
  else if (n_signal == SIGPIPE) {
    LOG(L_INFO) << "revive SIGSEGV, http_sender";
  }
#endif
  if (n_signal == SIGINT ||
      n_signal == SIGTERM ||
      n_signal == SIGABRT) {

    sender_server.Stop();
    LOG(L_WARNING) << "End of the http_sender";
    exit(EXIT_SUCCESS);
  }
  return 0;
}

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif  // _WIN32

  DpEvtService p_evt_service =
    (DpEvtService)vzbase::Thread::Current()->socketserver()->GetEvtService();

  Event_CreateSignalHandle(p_evt_service, SIGINT, SignalHandle,  NULL);
  Event_CreateSignalHandle(p_evt_service, SIGTERM, SignalHandle, NULL);
  Event_CreateSignalHandle(p_evt_service, SIGSEGV, SignalHandle, NULL);
  Event_CreateSignalHandle(p_evt_service, SIGABRT, SignalHandle, NULL);
#ifdef POSIX
  Event_CreateSignalHandle(p_evt_service, SIGPIPE, SignalHandle, NULL);
#endif

  bool b_ret = sender_server.Start();
  while(b_ret) {
    try {
      sender_server.RunLoop();
    } catch(std::exception &e) {
      LOG(L_ERROR) << e.what();
    }
  }

  LOG(L_ERROR) << "sender server start failed.";
  return 0;
}