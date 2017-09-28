/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "clibevent.h"

#include "vzbase/helper/stdafx.h"

namespace vzconn {

EVT_LOOP::EVT_LOOP()
  : base_event_(NULL)
  , running_(0) {
}

EVT_LOOP::~EVT_LOOP() {
  Stop();
}

int EVT_LOOP::Start() {
#ifdef WIN32
  evthread_use_windows_threads();
#else
  evthread_use_pthreads();
#endif
  if (base_event_ == NULL) {
    base_event_ = event_base_new();
    if (!base_event_) {
      printf("can't new a loop.\n");
      return -1;
    }
  }

  exit_timer_.Init(this, exit_callback, this);
  return 0;
}

void EVT_LOOP::Stop() {
  if (base_event_) {
    event_base_loopbreak(base_event_);

    event_base_free(base_event_);
    base_event_ = NULL;
  }
}

int EVT_LOOP::RunLoop(unsigned int ms_timeout) {
  int n_ret = -1;
  if (base_event_) {
    exit_timer_.Stop();

    if (ms_timeout > 0) {
      LoopExit(ms_timeout);
    }
    if (0 == running_) {
      running_ = 1;
      if (ms_timeout == 0) {
        n_ret = event_base_loop(base_event_, EVLOOP_NONBLOCK);
      } else {
        n_ret = event_base_loop(base_event_, EVLOOP_NO_EXIT_ON_EMPTY); // 无事件不退出
      }
      running_ = 0;
      return n_ret;
    }
  }
  return n_ret;
}

void EVT_LOOP::LoopExit(unsigned int ms_timeout) {
  if (base_event_) {
    int n_ret = 0;
    if (ms_timeout > 0) {
      struct timeval tv;
      tv.tv_sec = ms_timeout / 1000;
      tv.tv_usec = ms_timeout % 1000 * 1000;
      exit_timer_.Start(ms_timeout, 0);
    } else {
      n_ret = event_base_loopbreak(base_event_);
    }
    if (n_ret == -1) {
      LOG(L_ERROR) << "base loop exit failed.";
    }
  }
}

bool EVT_LOOP::isRuning() {
  return ((running_==1) ? true : false);
}

int EVT_LOOP::exit_callback(SOCKET fd, short events, const void *p_usr_arg) {
  EVT_LOOP *p_loop = (EVT_LOOP*)p_usr_arg;
  if (p_loop && p_loop->base_event_) {
    event_base_loopbreak(p_loop->base_event_);
  }
  return 0;
}

///TIMER///////////////////////////////////////////////////////////////////////
EVT_TIMER::EVT_TIMER() {
  base_event_      = NULL;
  callback_  = NULL;
  usr_args_  = NULL;

  init_      = 0;
  start_     = 0;
}

void EVT_TIMER::Init(const EVT_LOOP* loop, EVT_FUNC fn_cb, void* p_arg) {
  base_event_     = const_cast<EVT_LOOP*>(loop);

  callback_ = fn_cb;
  usr_args_ = p_arg;
}

int EVT_TIMER::Start(unsigned int after_ms, unsigned int repeat_ms) {
  int n_ret = -1;
  if (!base_event_ || !base_event_->get_event()) {
    LOG(L_ERROR)<<"param error.";
    return n_ret;
  }

  if (0 == init_) {
    if (repeat_ms != 0) {
      event_set(&event_, -1, EV_TIMEOUT | EV_PERSIST, evt_callback, (void*)this);
    } else {
      event_set(&event_, -1, EV_TIMEOUT, evt_callback, (void*)this);
    }
    n_ret = event_base_set(base_event_->get_event(), &event_);
    if (n_ret != 0) {
      LOG(L_ERROR)<<"event base set failed.";
      return n_ret;
    }
    init_ = 1;
    LOG(L_INFO) << "Set event "<<after_ms<<"-"<<repeat_ms;
  }

  if (repeat_ms == 0) {
    repeat_ms = after_ms;
  }
  struct timeval tv;
  tv.tv_sec  = repeat_ms / 1000;
  tv.tv_usec = (repeat_ms % 1000)*1000;
  n_ret = event_add(&event_, &tv);
  if (n_ret == 0) {
    start_ = 1;
  }

  return n_ret;
}

void EVT_TIMER::Stop() {
  if (1 == start_)  {
    event_del(&event_);
    start_ = 0;
  }
}

void EVT_TIMER::evt_callback(evutil_socket_t fd, short events, void *ctx) {
  if (ctx) {
    EVT_TIMER* pEvt = (EVT_TIMER*)ctx;
    if (pEvt->callback_) {
      pEvt->callback_(fd, events, pEvt->usr_args_);
    }
  }
}

///IO//////////////////////////////////////////////////////////////////////////
EVT_IO::EVT_IO() {
  base_event_       = NULL;
  callback_   = NULL;
  usr_args_   = NULL;

  init_       = 0;
  start_      = 0;
}

void EVT_IO::Init(const EVT_LOOP* loop, EVT_FUNC func, void* pArg) {
  base_event_     = const_cast<EVT_LOOP*>(loop);

  callback_ = func;
  usr_args_ = pArg;
}

int EVT_IO::Start(SOCKET vHdl, int nEvt, unsigned int ms_timeout) {
  int n_ret = -1;
  if (!base_event_ || !base_event_->get_event()) {
    LOG(L_ERROR)<<"param error.";
    return n_ret;
  }

  if (0 == init_ ||
      event_.ev_fd != vHdl ||
      event_.ev_events != nEvt) {
    Stop();

    event_set(&event_, vHdl, nEvt, evt_callback, this);
    n_ret = event_base_set(base_event_->get_event(), &event_);
    if (n_ret != 0) {
      LOG(L_ERROR) << "event base set failed.";
      return n_ret;
    }
    init_  = 1;
    //LOG(L_INFO) << "Set "<<vHdl<<" event "<<nEvt<<"-"<<c_evt_.ev_events;
  }

  if (start_ == 0) {
    if (ms_timeout == 0) {
      n_ret = event_add(&event_, NULL);
    } else {
      struct timeval tv = { 0, 0 };
      tv.tv_sec = ms_timeout / 1000;
      tv.tv_usec = (ms_timeout % 1000) * 1000;
      n_ret = event_add(&event_, &tv);
    }
  }
  if (n_ret == 0) {
    start_ = 1;
  }
  return n_ret;
}

void EVT_IO::Stop() {
  if (1 == start_)  {
    event_del(&event_);
    start_ = 0;
  }
}

void EVT_IO::ActiceEvent() {
  event_active(&event_, 0, 0);
}

void EVT_IO::evt_callback(evutil_socket_t fd, short events, void *ctx) {
  if (ctx) {
    EVT_IO* p_evt = (EVT_IO*)ctx;
    if (p_evt->callback_) {
      p_evt->callback_(fd, events, p_evt->usr_args_);
    }
  }
}

}  // namespace vzconn

#ifdef __cplusplus
extern "C" {
#endif
#include <signal.h>

static char k_app_name[32] = {0};
static int EvtSignalCallback(int sgl_num, short events, const void *usr_arg) {
  if (sgl_num == SIGINT) {
    LOG(L_ERROR) << "revive SIGINT, End of the " << k_app_name;
  } else if (sgl_num == SIGTERM) {
    LOG(L_ERROR) << "revive SIGTERM, End of the " << k_app_name;
  } else if (sgl_num == SIGSEGV) {
    LOG(L_ERROR) << "revive SIGSEGV, End of the " << k_app_name;
  } else if (sgl_num == SIGABRT) {
    LOG(L_ERROR) << "revive SIGSEGV, End of the " << k_app_name;
  }
#ifdef POSIX
  else if (sgl_num == SIGPIPE) {
    LOG(L_INFO) << "revive SIGSEGV, " << k_app_name;
  }
#endif
  if (sgl_num == SIGINT || sgl_num == SIGTERM ||
      sgl_num == SIGSEGV || sgl_num == SIGABRT) {
    *((unsigned int*)usr_arg) = 1;
    LOG(L_ERROR) << "End of the " << k_app_name;
  }
  return 0;
}

static vzconn::EVT_IO *CreateSignalHandle(vzconn::EventService* evt_service,
    int signal_no, void *user_arg) {
  vzconn::EVT_IO *evt_io = new vzconn::EVT_IO();
  if (NULL == evt_io) {
    LOG(L_ERROR) << "new evt_io failed.";
    return NULL;
  }

  evt_io->Init((vzconn::EVT_LOOP*)evt_service,
               (vzconn::EVT_FUNC)EvtSignalCallback, user_arg);
  int32 n_ret = evt_io->Start(signal_no, EV_SIGNAL | EVT_PERSIST);
  if (n_ret != 0) {
    LOG(L_ERROR) << "listening signal failed.";

    delete evt_io;
    evt_io = NULL;
  }
  return evt_io;
}

void ExitSignalHandle(vzconn::EventService *evt_srv,
                      const char *app_name, unsigned int *is_exit) {
  strncpy(k_app_name, app_name, 31);

  CreateSignalHandle(evt_srv, SIGINT,  is_exit);
  CreateSignalHandle(evt_srv, SIGTERM, is_exit);
  CreateSignalHandle(evt_srv, SIGSEGV, is_exit);
  CreateSignalHandle(evt_srv, SIGABRT, is_exit);
#ifdef POSIX
  CreateSignalHandle(evt_srv, SIGPIPE, is_exit);
#endif
}

#ifdef __cplusplus
};
#endif
