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

#ifdef WIN32
#include <comdef.h>  // NOLINT
#endif

using namespace vzbase;

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
  // EXPECT_TRUE_WAIT(done_, 1000);
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
  //event_set_log_callback(evt_log_cb);
  //event_enable_debug_logging(EVENT_DBG_ALL);

  //ShowVzLoggingAlways();
  LOG(L_INFO)<<"current id "<<Thread::GetCurrentThreadID();
  ComThreadTest_ComInited();
  return 0;
}
