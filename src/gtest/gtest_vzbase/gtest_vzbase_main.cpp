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

const int MAX = 65536;

// Generates a sequence of numbers (collaboratively).
class TestGenerator {
 public:
  TestGenerator() : last(0), count(0) {}

  int Next(int prev) {
    int result = prev + last;
    last = result;
    count += 1;
    return result;
  }

  int last;
  int count;
};

struct TestMessage : public MessageData {
  explicit TestMessage(int v) : value(v) {}
  virtual ~TestMessage() {}

  int value;
};

class CustomThread : public vzbase::Thread {
 public:
  CustomThread() {}
  virtual ~CustomThread() {
    Stop();
  }
  bool Start() {
    return false;
  }
};

//
//// A thread that does nothing when it runs and signals an event
//// when it is destroyed.
//class SignalWhenDestroyedThread : public Thread {
// public:
//  SignalWhenDestroyedThread(Event* event)
//    : event_(event) {
//  }
//
//  virtual ~SignalWhenDestroyedThread() {
//    Stop();
//    event_->Set();
//  }
//
//  virtual void Run() {
//    // Do nothing.
//  }
//
// private:
//  Event* event_;
//};

// Function objects to test Thread::Invoke.
struct Functor1 {
  int operator()() {
    return 42;
  }
};
class Functor2 {
 public:
  explicit Functor2(bool* flag) : flag_(flag) {}
  void operator()() {
    if (flag_) *flag_ = true;
  }
 private:
  bool* flag_;
};

// Test that setting thread names doesn't cause a malfunction.
// There's no easy way to verify the name was set properly at this time.
void ThreadTest_Names() {
  // Default name
  Thread *thread;
  thread = new Thread();
  ASSERT(thread->Start());
  thread->Stop();
  delete thread;
  thread = new Thread();
  // Name with no object parameter
  ASSERT(thread->SetName("No object", NULL));
  ASSERT(thread->Start());
  thread->Stop();
  delete thread;
  // Really long name
  thread = new Thread();
  ASSERT(thread->SetName("Abcdefghijklmnopqrstuvwxyz1234567890", NULL));
  ASSERT(thread->Start());
  thread->Stop();
  delete thread;
}

// Test that setting thread priorities doesn't cause a malfunction.
// There's no easy way to verify the priority was set properly at this time.
void ThreadTest_Priorities() {
  Thread *thread;
  thread = new Thread();
  ASSERT(thread->SetPriority(PRIORITY_HIGH));
  ASSERT(thread->Start());
  thread->Stop();
  delete thread;
  thread = new Thread();
  ASSERT(thread->SetPriority(PRIORITY_ABOVE_NORMAL));
  ASSERT(thread->Start());
  thread->Stop();
  delete thread;

  thread = new Thread();
  ASSERT(thread->Start());
#ifdef WIN32
  ASSERT(thread->SetPriority(PRIORITY_ABOVE_NORMAL));
#else
  ASSERT(thread->SetPriority(PRIORITY_ABOVE_NORMAL));
#endif
  thread->Stop();
  delete thread;

}

void ThreadTest_Wrap() {
  Thread* current_thread = Thread::Current();
  current_thread->UnwrapCurrent();
  CustomThread* cthread = new CustomThread();
  ASSERT(cthread->WrapCurrent());
  ASSERT(cthread->started());
  ASSERT(!cthread->IsOwned());
  cthread->UnwrapCurrent();
  ASSERT(!cthread->started());
  delete cthread;
  current_thread->WrapCurrent();
}

//// Test that calling Release on a thread causes it to self-destruct when
//// it's finished running
//void ThreadTest_Release() {
//  scoped_ptr<Event> event(new Event(true, false));
//  // Ensure the event is initialized.
//  event->Reset();
//
//  Thread* thread = new SignalWhenDestroyedThread(event.get());
//  thread->Start();
//  thread->Release();
//
//  // The event should get signaled when the thread completes, which should
//  // be nearly instantaneous, since it doesn't do anything.  For safety,
//  // give it 3 seconds in case the machine is under load.
//  bool signaled = event->Wait(3000);
//  EXPECT_TRUE(signaled);
//}

//void ThreadTest_Invoke() {
//  // Create and start the thread.
//  Thread thread;
//  thread.Start();
//  // Try calling functors.
//  EXPECT_EQ(42, thread.Invoke<int>(Functor1()));
//  bool called = false;
//  Functor2 f2(&called);
//  thread.Invoke<void>(f2);
//  EXPECT_TRUE(called);
//  // Try calling bare functions.
//  struct LocalFuncs {
//    static int Func1() {
//      return 999;
//    }
//    static void Func2() {}
//  };
//  EXPECT_EQ(999, thread.Invoke<int>(&LocalFuncs::Func1));
//  thread.Invoke<void>(&LocalFuncs::Func2);
//}

#ifdef WIN32
class ComThreadTest : public MessageHandler {
 public:
  ComThreadTest() : done_(false) {}
 protected:
  virtual void OnMessage(Message* message) {
    HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
    // S_FALSE means the thread was already inited for a multithread apartment.
    ASSERT(S_FALSE != hr);
    if (SUCCEEDED(hr)) {
      CoUninitialize();
    }
    done_ = true;
  }
 public:
  bool done_;
};

void ComThreadTest_ComInited() {
  Thread* thread = new ComThread();
  ComThreadTest ctt;
  ASSERT(thread->Start());
  thread->Post(&ctt, 0);
  // EXPECT_TRUE_WAIT(done_, 1000);
  while(!ctt.done_) {
    Sleep(1);
  }
  delete thread;
}
#endif

int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
  ThreadTest_Wrap();
  ThreadTest_Priorities();
  ThreadTest_Names();
#ifdef WIN32
  ComThreadTest_ComInited();
#endif
  return 0;
}