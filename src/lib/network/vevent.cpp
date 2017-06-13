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
#include "vevent.h"
#include "base/stdafx.h"

EVT_LOOP::EVT_LOOP()
  : p_event_(NULL) {
}

EVT_LOOP::~EVT_LOOP() {
  Stop();
}

int32_t EVT_LOOP::Start() {
#ifdef WIN32
  evthread_use_windows_threads();
#else
  evthread_use_pthreads();
#endif

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

int32_t EVT_LOOP::Runing() {
  int32_t n_ret = 0;
  n_ret = event_base_loop(p_event_, 0);
  return n_ret;
}

///TIMER///////////////////////////////////////////////////////////////////////
EVT_TIMER::EVT_TIMER() {
  p_base_      = NULL;
  p_callback_  = NULL;
  p_usr_args_  = NULL;

  s_name_      = "";
  b_start_     = false;

  c_evt_.ev_evcallback.evcb_flags = -1;
}

void EVT_TIMER::set_name(const std::string& sName) {
  s_name_ = sName;
}

const std::string& EVT_TIMER::get_name() const {
  return s_name_;
}

void EVT_TIMER::Init(const EVT_LOOP* loop, EVT_FUNC fn_cb, void* p_arg) {
  p_base_     = const_cast<EVT_LOOP*>(loop);

  p_callback_ = fn_cb;
  p_usr_args_ = p_arg;
}

int32_t EVT_TIMER::Start(uint32_t after_ms, uint32_t repeat_ms) {
  int32_t n_ret = RET_INVLIAD_HANDLE;
  if (!p_base_ || !p_base_->get_event()) {
    LOG(L_ERROR)<<"param error.";
    return n_ret;
  }

  if (b_start_ == false) {
    if (repeat_ms != 0) {
      event_set(&c_evt_, -1, EV_TIMEOUT | EV_PERSIST, evt_callback, (void*)this);
    } else {
      event_set(&c_evt_, -1, EV_TIMEOUT, evt_callback, (void*)this);
    }
    n_ret = event_base_set(p_base_->get_event(), &c_evt_);
    if (n_ret != 0) {
      LOG(L_ERROR)<<"event base set failed.";
      return n_ret;
    }
  }

  struct timeval tv;
  tv.tv_sec  = repeat_ms / 1000;
  tv.tv_usec = (repeat_ms % 1000)*1000;
  n_ret = event_add(&c_evt_, &tv);
  if (n_ret == 0) {
    b_start_ = true;
  }

  return n_ret;
}

void EVT_TIMER::Stop() {
  if (b_start_)  {
    event_del(&c_evt_);
    b_start_ = false;
  }
}

void EVT_TIMER::evt_callback(int fd, short event, void *ctx) {
  if (ctx) {
    EVT_TIMER* pEvt = (EVT_TIMER*)ctx;
    if (pEvt->p_callback_) {
      pEvt->p_callback_(event, pEvt->p_usr_args_);
    }
  }
}

///IO//////////////////////////////////////////////////////////////////////////
EVT_IO::EVT_IO() {
  p_base_       = NULL;
  p_callback_   = NULL;
  p_usr_args_   = NULL;

  s_name_       = "";
  b_start_      = false;
}

void EVT_IO::set_name(const std::string& s_name) {
  s_name_ = s_name;
}

const std::string& EVT_IO::get_name() const {
  return s_name_;
}

void EVT_IO::Init(const EVT_LOOP* loop, EVT_FUNC func, void* pArg) {
  p_base_     = const_cast<EVT_LOOP*>(loop);

  p_callback_ = func;
  p_usr_args_ = pArg;
}

int32_t EVT_IO::Start(SOCKET vHdl, int32_t nEvt) {
  int32_t n_ret = RET_INVLIAD_HANDLE;
  if (!p_base_ || !p_base_->get_event()) {
    LOG(L_ERROR)<<"param error.";
    return n_ret;
  }

  if (b_start_ == false) {
    event_set(&c_evt_, vHdl, nEvt, evt_callback, this);
    n_ret = event_base_set(p_base_->get_event(), &c_evt_);
    if (n_ret != 0) {
      LOG(L_ERROR)<<"event base set failed.";
      return n_ret;
    }
  }

  if (c_evt_.ev_evcallback.evcb_flags > 0) {
    n_ret = event_add(&c_evt_, NULL);
    if (n_ret == 0) {
      b_start_ = true;
    }
  }
  return n_ret;
}

void EVT_IO::Stop() {
  if (b_start_)  {
    event_del(&c_evt_);
    b_start_ = false;
  }
}

void EVT_IO::evt_callback(evutil_socket_t fd, short events, void *ctx) {
  if (ctx) {
    EVT_IO* p_evt = (EVT_IO*)ctx;
    if (p_evt->p_callback_) {
      p_evt->p_callback_(events, p_evt->p_usr_args_);
    }
  }
}
