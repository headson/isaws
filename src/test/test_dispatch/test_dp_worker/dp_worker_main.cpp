#include "vzbase/helper/stdafx.h"

#include "dispatcher/base/pkghead.h"
#include "dispatcher/sync/dpclient_c.h"

#ifdef WIN32
#include <Windows.h>
#else
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const int MAX_TYPES_SIZE = 36;
const char* MSG_TYPES[] = {
  "TEST_MSG_TYPE_01",
  "TEST_MSG_TYPE_02",
  "TEST_MSG_TYPE_03",
  "TEST_MSG_TYPE_04",
  "TEST_MSG_TYPE_05",
  "TEST_MSG_TYPE_06",
  "TEST_MSG_TYPE_07",
  "TEST_MSG_TYPE_08",
  "TEST_MSG_TYPE_09",
  "TEST_MSG_TYPE_10",
  "TEST_MSG_TYPE_11",
  "TEST_MSG_TYPE_12",
  "TEST_MSG_TYPE_13",
  "TEST_MSG_TYPE_14",
  "TEST_MSG_TYPE_15",
  "TEST_MSG_TYPE_16",
  "TEST_MSG_TYPE_17",
  "TEST_MSG_TYPE_18",
  "TEST_MSG_TYPE_19",
  "TEST_MSG_TYPE_20",
  "TEST_MSG_TYPE_21",
  "TEST_MSG_TYPE_22",
  "TEST_MSG_TYPE_23",
  "TEST_MSG_TYPE_24",
  "TEST_MSG_TYPE_25",
  "TEST_MSG_TYPE_26",
  "TEST_MSG_TYPE_27",
  "TEST_MSG_TYPE_28",
  "TEST_MSG_TYPE_29",
  "TEST_MSG_TYPE_30",
  "TEST_MSG_TYPE_31",
  "TEST_MSG_TYPE_32",
  "TEST_MSG_TYPE_33",
  "TEST_MSG_TYPE_34",
  "TEST_MSG_TYPE_35",
  "TEST_MSG_TYPE_36",
};

void DpMsgallback(const DpMessage *dmp, void* p_usr_arg) {
  printf("dp message %s, replay id %d %d.\n",
         dmp->method, dmp->id, ((dmp->id >> 24) & 0xff));

  if (dmp->data_size > 0) {
    //dmp->data[dmp->data_size] = '\0';
    LOG(L_INFO) << "recv packet length " << dmp->data
                << " data " << dmp->data;
  }
  //if (dmp->type == TYPE_REQUEST) {
  DpClient_SendDpReply(dmp->method,
                       dmp->channel_id,
                       dmp->id,
                       "reply",
                       5);
  //}
}

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);
#ifdef WIN32
  ShowVzLoggingAlways();
#endif

  DpClient_Init("127.0.0.1", 5291);

  DpClient_Start(0);

  void *p_hdl = DpClient_CreatePollHandle();
  DpClient_HdlAddListenMessage(p_hdl, MSG_TYPES, MAX_TYPES_SIZE);

  while (true) {
    int32 n_ret = DpClient_PollDpMessage(p_hdl, DpMsgallback, NULL, 1000);
    if (n_ret == VZNETDP_FAILURE) {
      DpClient_ReleasePollHandle(p_hdl);
      p_hdl = NULL;

      p_hdl = DpClient_CreatePollHandle();
      DpClient_HdlAddListenMessage(p_hdl, MSG_TYPES, MAX_TYPES_SIZE);
    }
  }

  getchar();

  DpClient_Stop();
  return 0;
}