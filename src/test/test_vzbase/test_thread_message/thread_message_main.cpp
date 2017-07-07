/*
 * libjingle
 * Copyright 2004--2011, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "vzbase/thread/thread.h"
#include "vzlogging/logging/vzloggingcpp.h"

#include "dispatcher/sync/dpclient_c.h"

#ifdef WIN32
#include <comdef.h>  // NOLINT
#endif

using namespace vzbase;

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

Thread* thread1 = NULL;
class ComThreadTest : public MessageHandler {
 public:
  ComThreadTest() : done_(false) {}
 protected:
  virtual void OnMessage(Message* message) {
#if 0
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    // S_FALSE means the thread was already inited for a multithread apartment.
    ASSERT(S_FALSE != hr);
    if (SUCCEEDED(hr)) {
      CoUninitialize();
    }
    done_ = true;
#endif
    LOG(L_INFO)<<"read one message."<<Thread::GetCurrentThreadID();
    TypedMessageData<std::string>::Ptr msg =
      boost::static_pointer_cast< TypedMessageData< std::string > > (message->pdata);

    LOG(L_INFO) <<"message id "<< message->message_id
                <<" data "<<  msg->data().c_str();
  }
 public:
  bool done_;
};

void ComThreadTest_ComInited() {
  thread1 = new Thread();

  ComThreadTest ctt;
  uint32 n_msg_id = 0;

  ASSERT(thread1->Start());
  thread1->Start();

  // 创建句柄
  void *p_hdl = DpClient_CreatePollHandle(DpMsgCallback, NULL,
                                          DpStateCallback, NULL,
                                          (DpEvtService)Thread::Current()->socketserver()->GetEvtService());
  if (NULL == p_hdl) {
    LOG(L_ERROR) << "memory empty.";
    return;
  }
  // 注册消息
  DpClient_HdlAddListenMessage(p_hdl, MSG_TYPES, MAX_TYPES_SIZE);

  Thread::Current()->Run();

  //EXPECT_TRUE_WAIT(done_, 1000);
  while(true) {
    Thread::SleepMs(2*1000);

    std::string ss = "test hello worlds.\n";
    TypedMessageData<std::string>::Ptr talk_ptr =
      TypedMessageData<std::string>::Ptr(
        new TypedMessageData<std::string>(ss));

    LOG(L_INFO) << "post one message to thread1."<<Thread::GetCurrentThreadID();

    thread1->Post(&ctt, n_msg_id++, talk_ptr);
    //thread1->PostDelayed(1*1000, &ctt, n_msg_id++, talk_ptr);
  }
  delete thread1;
}

void evt_log_cb(int severity, const char *msg) {
  puts(msg);
}

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
  ShowVzLoggingAlways();

  DpClient_Init("127.0.0.1", 5291);
  DpClient_Start(0);

  LOG(L_INFO)<<"current id "<<Thread::GetCurrentThreadID();
  ComThreadTest_ComInited();
  return 0;
}
