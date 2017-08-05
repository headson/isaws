#include "vzbase/helper/stdafx.h"

#include "vzbase/base/vmessage.h"
#include "vzbase/base/timeutils.h"

#include "json/json.h"
#include "alg/basedefine.h"

#include "dispatcher/sync/dpclient_c.h"

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);
#ifdef WIN32
  ShowVzLoggingAlways();
#endif

  DpClient_Init("127.0.0.1", 5291);

  static int positive_number = 0, last_positive_number = 0;
  static int negative_number = 0, last_negative_number = 0;
  static unsigned int nnow = vzbase::CurrentTimet() - 10 * 60 * 60;

  while(1) {
    LOG(L_ERROR) << "begin."; 
    nnow += 60;

    Json::Value jroot;
    jroot[MSG_CMD] = MSG_CATCH_EVENT;
    jroot[MSG_BODY][ALG_EVT_OUT_TIMET] = nnow;

    positive_number += (rand() % 3);
    jroot[MSG_BODY][ALG_POSITIVE_NUMBER] = positive_number;
    jroot[MSG_BODY][ALG_POSITIVE_ADD_NUM] = positive_number - last_positive_number;

    negative_number += (rand() % 3);
    jroot[MSG_BODY][ALG_NEGATIVE_NUMBER] = negative_number;
    jroot[MSG_BODY][ALG_NEGATIVE_ADD_NUM] = negative_number - last_negative_number;

    last_positive_number = positive_number;
    last_negative_number = negative_number;

    Json::FastWriter jfw;
    std::string sjson = jfw.write(jroot);
    int res = DpClient_SendDpMessage(MSG_CATCH_EVENT, 0,
                                     sjson.c_str(), sjson.size());

    LOG(L_ERROR) << "end.\n\n";
    usleep(1000 * 1000);
  }
  getchar();

  DpClient_Stop();
  return 0;
}
