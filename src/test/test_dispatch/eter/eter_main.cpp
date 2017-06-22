#include "vznetdp/netdp/dpclient_c.h"
#include "vzbase/base/loggingsettings.h"

#ifdef WIN32
#include <Windows.h>
#else
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const int MAX_TYPES_SIZE = 2;
const int MAX_TIMES_PERIOD = 8;
const char* MSG_TYPES[] = {
  "ATEST_MSG_TYPE_01",
  "ATEST_MSG_TYPE_02",
};

void OnReplyDpMessage(const DpMessage* dp_msg, void* user_data) {
  //printf("OnDpMessage %s... ...\n", dp_msg->method_);
  unsigned int tty_id = 0;
  //DpClient_SendDpRequest("BUS_GET_LINKAGE_CFG", 0,
  //                       (const char *)(&tty_id),
  //                       sizeof(tty_id),
  //                       OnReplyDpMessage,
  //                       NULL,
  //                       5);
  if (dp_msg->type == TYPE_REQUEST) {
    LOG_INFO("Recv Request %s %d\n", dp_msg->method, dp_msg->id);
  } else if (dp_msg->type == TYPE_REPLY) {
    LOG_INFO("Recv Replay %s %d\n", dp_msg->method, dp_msg->id);
  } else {
    LOG_INFO("OnReplyDpMessage Recv Push Message %s %d\n", dp_msg->method, dp_msg->id);
  }
}

void OnConnectSucceed(void * user_data) {
  *((int *)(user_data)) = 1;
  const char* temp[1] = { "Hello" };
  int r = rand() % 16;
  DpClient_AddListenMessage(MSG_TYPES, MAX_TYPES_SIZE, NULL, NULL);
  // DpClient_SendDpMessage("HEELO", 0, "H", 1);
  unsigned int tty_id = 0;
  //DpClient_SendDpRequest("BUS_GET_LINKAGE_CFG", 0,
  //                       NULL,
  //                       0,
  //                       OnReplyDpMessage,
  //                       NULL,
  //                       5);
}

void OnDpErrorEvent(const char* msg, int size, void* user_data) {
  *((int *)(user_data)) = 0;
  LOG_INFO("OnDpErrorEvent %s... ...\n", msg);
}

void OnDpMessage(const DpMessage* dp_msg, void* user_data) {
  //printf("OnDpMessage %s... ...\n", dp_msg->method_);
  if (dp_msg->type == TYPE_REQUEST) {
    LOG_INFO("Recv Request %s %d\n", dp_msg->method, dp_msg->id);
    DpClient_SendDpReply(dp_msg->method, dp_msg->channel_id,
                         dp_msg->id, dp_msg->data, dp_msg->data_size);
  } else if (dp_msg->type == TYPE_REPLY) {
    LOG_INFO("Recv Replay %s %d\n", dp_msg->method, dp_msg->id);
    //DpClient_SendDpRequest("BUS_GET_LINKAGE_CFG", 0,
    //                       NULL,
    //                       0,
    //                       OnReplyDpMessage,
    //                       NULL,
    //                       5);
  } else {
    LOG_INFO("OnDpErrorEvent Recv Push Message %s %d\n", dp_msg->method, dp_msg->id);
  }
}

int main(int argc, char *argv[]) {
  vzbase::InitLoggingSystem(argc, argv);
  DpClient_Init("127.0.0.1", 5291);
  int is_connect_succeed = 0;
  DpClient_SetConnectSucceedCallback(OnConnectSucceed, &is_connect_succeed);
  DpClient_SetErrorEventCallback(OnDpErrorEvent, &is_connect_succeed);
  DpClient_SetDefaultMessageCallback(OnDpMessage, NULL);

  DpClient_Start(false);
#ifdef WIN32
  Sleep(100000);
#endif
  DpClient_Stop();
  return 0;
}