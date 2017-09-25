#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "chttpasync.h"
#include "cmultiserver.h"
#include "vzbase/helper/stdafx.h"

class CHttpRespCall : public CHttpRespose
{
public:
  void RespCallvack(CHttpAsync *cfg, int errcode) {
    LOG(L_ERROR) << CMultiServer::CurlEasyError(errcode);
  }
};

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
#ifdef _WIN32
  ShowVzLoggingAlways();
#endif

  CHttpRespCall http_resp;
  vzconn::EVT_LOOP evt_loop;
  evt_loop.Start();

  CMultiServer *psrv = CMultiServer::Create(&evt_loop);
  if (psrv == NULL) {
    return -1;
  }

  CHttpAsync chttp(&http_resp);

  const char* post_data = "hello worlds.";
  chttp.Post("http://192.168.6.8", 8080, 
             post_data, strlen(post_data), 
             psrv);

  while (true) {
    evt_loop.RunLoop(1000);
  }

  return 0;
}
