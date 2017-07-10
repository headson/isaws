#include "vzbase/helper/stdafx.h"

#include "vzconn/base/clibevent.h"
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

void DpMsgCallback(DPPollHandle p_hdl, const DpMessage *dmp, void* p_usr_arg) {
  printf("dp message %s, replay id %d %d.\n",
         dmp->method, dmp->id, ((dmp->id >> 24) & 0xff));

  if (dmp->data_size > 0) {
    //dmp->data[dmp->data_size] = '\0';
    LOG(L_INFO) << "recv packet length " << dmp->data
                << " data " << dmp->data;
  }
  //if (dmp->type == TYPE_REQUEST) {
  printf("dp message %s, replay id %d %d.\n",
         dmp->method, dmp->id, ((dmp->id >> 24) & 0xff));

  if (dmp->data_size > 0) {
    //dmp->data[dmp->data_size] = '\0';
    LOG(L_INFO) << "recv packet length " << dmp->data
                << " data " << dmp->data;
  }

  if (dmp->type == TYPE_REQUEST) {
    std::string broadcast("broadcastMsg");
    DpClient_SendDpMessage("broadcast", 0,
                           broadcast.c_str(), broadcast.length());

    DpClient_SendDpReply(dmp->method,
                         dmp->channel_id,
                         dmp->id,
                         "reply",
                         5);
  }
}

// 状态只处理了断网,由于重连还需要去get_session_id
// 所以注册消息不能放在连接成功之后立刻注册,需要放在get_session_id之后,
// DpClient_HdlReConnect完成了连接+get_session_id
void DpStateCallback(DPPollHandle p_hdl, uint32 n_state, void* p_usr_arg) {
  int32 n_ret = 0;
  if (n_state == DP_CLIENT_DISCONNECT) {
    n_ret = DpClient_HdlReConnect(p_hdl);
    if (n_ret == VZNETDP_SUCCEED) {
      // 重新注册消息
      DpClient_HdlAddListenMessage(p_hdl, MSG_TYPES, MAX_TYPES_SIZE);
    }
  }
}

static int32 evt_timer_cb(SOCKET          fd,
                          short           events,
                          const void      *p_usr_arg) {
  return 0;
}

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);
#ifdef WIN32
  ShowVzLoggingAlways();
#endif

  DpClient_Init("127.0.0.1", 5291);

  DpClient_Start(0);

  vzconn::EVT_TIMER evt_timer;

  // 创建句柄
  void *p_hdl = DpClient_CreatePollHandle(DpMsgCallback, NULL,
                                          DpStateCallback, NULL,
                                          NULL);
  if (NULL == p_hdl) {
    LOG(L_ERROR) << "memory empty.";
    return -1;
  }
  // 注册消息
  DpClient_HdlAddListenMessage(p_hdl, MSG_TYPES, MAX_TYPES_SIZE);

  // 一个定时器例子
  vzconn::EVT_LOOP *p_evt_loop =
    (vzconn::EVT_LOOP *)DpClient_GetEvtLoopFromPoll(p_hdl);
  evt_timer.Init(p_evt_loop, evt_timer_cb, NULL);
  evt_timer.Start(1000, 1000);

  while (true) {
    DpClient_PollDpMessage(p_hdl, 1000);
  }

  getchar();

  DpClient_Stop();
  return 0;
}