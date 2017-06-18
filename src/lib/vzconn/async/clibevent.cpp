/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : VLoopEvent.cpp
* Author        : Sober.Peng
* Date          : 18:1:2017
* Description   :
*-----------------------------------------------------------------------------
* Modify        :
*-----------------------------------------------------------------------------
******************************************************************************/
#include "clibevent.h"

#include "stdafx.h"

EVT_LOOP::EVT_LOOP()
  : p_event_(NULL) {
}

EVT_LOOP::~EVT_LOOP() {
  Stop();
}

int32 EVT_LOOP::Start() {
//#ifdef WIN32
//  evthread_use_windows_threads();
//#else
//  evthread_use_pthreads();
//#endif

  p_event_ = event_base_new();
  if (!p_event_) {
    printf("can't new a loop.\n");
    return -1;
  }
  return 0;
}

void EVT_LOOP::Stop() {
  if (p_event_) {
    event_base_loopbreak(p_event_);

    event_base_free(p_event_);
    p_event_ = NULL;
  }
}

int32 EVT_LOOP::RunLoop() {
  int32 n_ret = 0;
  n_ret = event_base_loop(p_event_, 0);
  return n_ret;
}

///TIMER///////////////////////////////////////////////////////////////////////
EVT_TIMER::EVT_TIMER() {
  p_base_      = NULL;
  p_callback_  = NULL;
  p_usr_args_  = NULL;

<<<<<<< HEAD:src/lib/vzconn/async/clibevent.cpp
  b_init_      = 0;
  b_start_     = 0;
=======
  s_name_      = "";

  b_init_      = false;
  b_start_     = false;
}

void EVT_TIMER::SetName(const std::string& sName) {
  s_name_ = sName;
}

const std::string& EVT_TIMER::GetName() const {
  return s_name_;
>>>>>>> 8c471fd87910ea7f532ac1fc43711c32142c523a:src/lib/network/vevent.cpp
}

void EVT_TIMER::Init(const EVT_LOOP* loop, EVT_FUNC fn_cb, void* p_arg) {
  p_base_     = const_cast<EVT_LOOP*>(loop);

  p_callback_ = fn_cb;
  p_usr_args_ = p_arg;
}

int32 EVT_TIMER::Start(uint32 after_ms, uint32 repeat_ms) {
  int32 n_ret = -1;
  if (!p_base_ || !p_base_->get_event()) {
    LOG(L_ERROR)<<"param error.";
    return n_ret;
  }

<<<<<<< HEAD:src/lib/vzconn/async/clibevent.cpp
  if (0 == b_init_) {
=======
  if (b_init_ == false) {
>>>>>>> 8c471fd87910ea7f532ac1fc43711c32142c523a:src/lib/network/vevent.cpp
    if (repeat_ms != 0) {
      event_set(&c_evt_, -1, EV_TIMEOUT | EV_PERSIST, evt_callback, (void*)this);
    } else {
      repeat_ms = after_ms;
      event_set(&c_evt_, -1, EV_TIMEOUT, evt_callback, (void*)this);
    }
    n_ret = event_base_set(p_base_->get_event(), &c_evt_);
    if (n_ret != 0) {
      LOG(L_ERROR)<<"event base set failed.";
      return n_ret;
    }
<<<<<<< HEAD:src/lib/vzconn/async/clibevent.cpp
    b_init_ = 1;
    LOG(L_INFO) << "Set event "<<after_ms<<"-"<<repeat_ms;
=======
    b_init_ = true;
>>>>>>> 8c471fd87910ea7f532ac1fc43711c32142c523a:src/lib/network/vevent.cpp
  }

  struct timeval tv;
  tv.tv_sec  = repeat_ms / 1000;
  tv.tv_usec = (repeat_ms % 1000)*1000;
  n_ret = event_add(&c_evt_, &tv);
  if (n_ret == 0) {
    b_start_ = 1;
  }

  return n_ret;
}

void EVT_TIMER::Stop() {
  if (1 == b_start_)  {
    event_del(&c_evt_);
    b_start_ = 0;
  }
}

void EVT_TIMER::evt_callback(int fd, short events, void *ctx) {
  if (ctx) {
    EVT_TIMER* pEvt = (EVT_TIMER*)ctx;
    if (pEvt->p_callback_) {
      pEvt->p_callback_(fd, events, pEvt->p_usr_args_);
    }
  }
}

///IO//////////////////////////////////////////////////////////////////////////
EVT_IO::EVT_IO() {
  p_base_       = NULL;
  p_callback_   = NULL;
  p_usr_args_   = NULL;

<<<<<<< HEAD:src/lib/vzconn/async/clibevent.cpp
  b_init_       = 0;
  b_start_      = 0;
=======
  s_name_       = "";

  b_init_       = false;
  b_start_      = false;
}

void EVT_IO::SetName(const std::string& s_name) {
  s_name_ = s_name;
}

const std::string& EVT_IO::GetName() const {
  return s_name_;
>>>>>>> 8c471fd87910ea7f532ac1fc43711c32142c523a:src/lib/network/vevent.cpp
}

void EVT_IO::Init(const EVT_LOOP* loop, EVT_FUNC func, void* pArg) {
  p_base_     = const_cast<EVT_LOOP*>(loop);

  p_callback_ = func;
  p_usr_args_ = pArg;
}

int32 EVT_IO::Start(SOCKET vHdl, int32 nEvt, uint32 n_timeout) {
  int32 n_ret = -1;
  if (!p_base_ || !p_base_->get_event()) {
    LOG(L_ERROR)<<"param error.";
    return n_ret;
  }

<<<<<<< HEAD:src/lib/vzconn/async/clibevent.cpp
  if (0 == b_init_ || c_evt_.ev_events != nEvt) {
=======
  if (b_init_ == false) {
>>>>>>> 8c471fd87910ea7f532ac1fc43711c32142c523a:src/lib/network/vevent.cpp
    event_set(&c_evt_, vHdl, nEvt, evt_callback, this);
    n_ret = event_base_set(p_base_->get_event(), &c_evt_);
    if (n_ret != 0) {
      LOG(L_ERROR) << "event base set failed.";
      return n_ret;
    }
<<<<<<< HEAD:src/lib/vzconn/async/clibevent.cpp
    b_init_ = 1;
    LOG(L_INFO) << "Set "<<vHdl<<" event "<<nEvt<<"-"<<c_evt_.ev_events;
  }
  if (n_timeout == 0) {
    n_ret = event_add(&c_evt_, NULL);
  } else {
    struct timeval tv = {0, 0};
    tv.tv_sec  = n_timeout / 1000;
    tv.tv_usec = (n_timeout % 1000) * 1000;
    n_ret = event_add(&c_evt_, &tv);
  }
  if (n_ret == 0) {
    b_start_ = 1;
=======
    b_init_ = true;
  }

  n_ret = event_add(&c_evt_, NULL);
  if (n_ret == 0) {
    b_start_ = true;
>>>>>>> 8c471fd87910ea7f532ac1fc43711c32142c523a:src/lib/network/vevent.cpp
  }
  return n_ret;
}

void EVT_IO::Stop() {
  if (1 == b_start_)  {
    event_del(&c_evt_);
    b_start_ = 0;
  }
}


void EVT_IO::ActiceEvent() {
  event_active(&c_evt_, 0, 0);
}

void EVT_IO::evt_callback(evutil_socket_t fd, short events, void *ctx) {
  if (ctx) {
    EVT_IO* p_evt = (EVT_IO*)ctx;
    if (p_evt->p_callback_) {
      p_evt->p_callback_(fd, events, p_evt->p_usr_args_);
    }
  }
}
