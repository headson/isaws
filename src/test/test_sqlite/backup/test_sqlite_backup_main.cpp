#include "vzbase/helper/stdafx.h"

#include "vzconn/base/clibevent.h"
#include "dispatcher/sync/dpclient_c.h"

#include "vzbase/thread/thread.h"

#ifdef WIN32
#include <Windows.h>
#else
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

const int MAX_TYPES_SIZE = 3;
const char* MSG_TYPES[] = {
  "TEST_MSG_TYPE_01",
  "TEST_MSG_TYPE_02",
  "TEST_MSG_TYPE_03"
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

#define MSG_HDL_USER_ID   0x123456
class CMsgTimerProc;

static CMsgTimerProc *p_msg_hdl = NULL;
class CMsgTimerProc : public vzbase::MessageHandler {
 public:
 protected:
  void OnMessage(vzbase::Message* msg) {
    if (msg->message_id == MSG_HDL_USER_ID) {
      LOG_INFO("hello worlds.\n");

      vzbase::Thread::Current()->PostDelayed(3*1000, p_msg_hdl, MSG_HDL_USER_ID);
    }
  }
 private:
};

class CMsgProc : public vzbase::MessageHandler {
public:
protected:
  void OnMessage(vzbase::Message* msg) {
    if (msg->message_id == MSG_HDL_USER_ID) {
      LOG_INFO("hello worlds.\n");

      vzbase::Thread::Current()->PostDelayed(3 * 1000, p_msg_hdl, MSG_HDL_USER_ID);
    }
  }
private:
};


int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);
#ifdef WIN32
  ShowVzLoggingAlways();
#endif

#if 1
  DpClient_Init("127.0.0.1", 5291);
  DpClient_Start(0);

  // 创建句柄
  void *p_hdl = DpClient_CreatePollHandle(DpMsgCallback, NULL,
                                          DpStateCallback, NULL,
                                          vzbase::Thread::Current()->socketserver()->GetEvtService());
  if (NULL == p_hdl) {
    LOG(L_ERROR) << "memory empty.";
    return -1;
  }
  // 注册消息
  DpClient_HdlAddListenMessage(p_hdl,
                               MSG_TYPES,
                               MAX_TYPES_SIZE);
#endif

  // 投递消息:定时器
  p_msg_hdl = new CMsgTimerProc();
  vzbase::Thread::Current()->PostDelayed(3*1000, p_msg_hdl, MSG_HDL_USER_ID);

  // 投递消息:传递消息

  //vzbase::Thread::Current()->Post();

  while (true) {
    // 如果调用此处之后无其他处理,请调用vzbase::Thread::Current()->Run();
    // 线程的Run可通过投递消息(Post\PostDelayed)的方式实现定时器

#if 1
    vzbase::Thread::Current()->Run();
#else
    DpClient_PollDpMessage(p_hdl, 100); 
#endif
  }

  getchar();

  DpClient_Stop();
  if (p_msg_hdl) {
    delete p_msg_hdl;
    p_msg_hdl = NULL;
  }
  return 0;
}