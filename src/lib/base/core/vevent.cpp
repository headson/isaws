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

#include "base/core/vthread.h"

EVT_LOOP::EVT_LOOP()
  : m_pEvent(NULL) {
}

EVT_LOOP::~EVT_LOOP() {
  Stop();
}

int32_t EVT_LOOP::Start() {
  m_pEvent = event_base_new();
  if (!m_pEvent) {
    printf("can't new a loop.\n");
    return -1;
  }
  return 0;
}

void EVT_LOOP::Stop() {
  if (m_pEvent) {
    event_base_loopbreak(m_pEvent);

    event_base_free(m_pEvent);
    m_pEvent = NULL;
  }
}

void EVT_LOOP::Runing(bool* b_runing) {
#ifdef WIN32
  evthread_use_windows_threads();
#else
  evthread_use_pthreads();
#endif

  int32_t nRet = 0;
  while (b_runing && *b_runing) {
    VThread::msleep(200);
    nRet = event_base_loop(m_pEvent, 0);
    if (nRet < 0) {                 // 表示发生了错误
      break;
    }
    //printf("%s no event %d.\n", get_name().c_str(), VThread::get_pid());
    //(nRet == 1 || nRet == 0)      // 表示没有事件被注册
  }
}

///TIMER///////////////////////////////////////////////////////////////////////
EVT_TIMER::EVT_TIMER() {
  m_pLoop      = NULL;
  m_pCallback  = NULL;
  m_pUserArgs  = NULL;

  m_sName      = "";
  m_bStart     = false;

  m_nAfter     = 0;
  m_nRepeat    = 0;

  m_cEvt.ev_flags = -1;
}

void EVT_TIMER::set_name(const std::string& sName) {
  m_sName = sName;
}

const std::string& EVT_TIMER::get_name() const {
  return m_sName;
}

void EVT_TIMER::Init(const EVT_LOOP* loop, EVT_FUNC func, void* pArg) {
  m_pLoop     = const_cast<EVT_LOOP*>(loop);

  m_pCallback = func;
  m_pUserArgs = pArg;
}

int32_t EVT_TIMER::Start(uint32_t after, uint32_t repeat) {
  if (!m_pLoop) {
    return -1;
  }

  m_nAfter  = after;
  m_nRepeat = repeat;

  int32_t nRet = 0;
  if (m_cEvt.ev_flags <= 0) {
    if (repeat != 0) {
      event_set(&m_cEvt, -1, EV_TIMEOUT | EV_PERSIST, evt_callback, (void*)this);
    } else {
      event_set(&m_cEvt, -1, EV_TIMEOUT, evt_callback, (void*)this);
    }
    event_base_set(m_pLoop->get_event(), &m_cEvt);
  }

  if (m_cEvt.ev_flags > 0) {
    struct timeval tv;
    tv.tv_sec  = m_nRepeat / 1000;
    tv.tv_usec = (m_nRepeat % 1000)*1000;

    nRet = event_add(&m_cEvt, &tv);
    if (nRet == 0) {
      m_bStart = true;
    }
  }
  return nRet;
}

void EVT_TIMER::Stop() {
  if (m_bStart)  {
    event_del(&m_cEvt);
    m_bStart = false;
  }
}

void EVT_TIMER::evt_callback(int fd, short event, void *ctx) {
  if (ctx) {
    EVT_TIMER* pEvt = (EVT_TIMER*)ctx;
    if (pEvt->m_pCallback) {
      /*if (pEvt->_nRepeat) {
      struct timeval tv;
      tv.tv_sec  = pEvt->_nRepeat / 1000;
      tv.tv_usec = (pEvt->_nRepeat % 1000)*1000;

      if (pEvt->_cEvt.ev_flags > 0) {
      event_add(&pEvt->_cEvt, &tv);
      }
      }*/

      pEvt->m_pCallback(event, pEvt->m_pUserArgs);
    }
  }
}

///IO//////////////////////////////////////////////////////////////////////////
EVT_IO::EVT_IO() {
  m_pLoop       = NULL;
  m_pCallback   = NULL;
  m_pUserArgs   = NULL;

  m_sName       = "";
  m_bStart      = false;
  m_nIdleCall   = 0;

  m_cEvt.ev_flags = -1;
}

bool EVT_IO::is_start() {
  return m_bStart;
}

void EVT_IO::set_name(const std::string& sName) {
  m_sName = sName;
}

const std::string& EVT_IO::get_name() const {
  return m_sName;
}

void EVT_IO::Init(const EVT_LOOP* loop, EVT_FUNC func, void* pArg) {
  m_pLoop     = const_cast<EVT_LOOP*>(loop);

  m_pCallback = func;
  m_pUserArgs = pArg;
}

int32_t EVT_IO::Start(SOCKET vHdl, int32_t nEvt) {
  if (!m_pLoop || !m_pLoop->get_event()) {
    return -1;
  }

  int32_t nRet = 0;
  if (m_cEvt.ev_flags <= 0) {
    event_set(&m_cEvt, vHdl, nEvt, evt_callback, this);
    nRet = event_base_set(m_pLoop->get_event(), &m_cEvt);
    if (nRet != 0) {
      return -2;
    }
  }

  if (m_cEvt.ev_flags > 0) {
    nRet = event_add(&m_cEvt, NULL);
    if (nRet == 0) {
      m_bStart = true;
    }
  }
  return nRet;
}

void EVT_IO::Stop() {
  if (m_bStart)  {
    event_del(&m_cEvt);
    m_bStart = false;
  }
}

void EVT_IO::evt_callback(evutil_socket_t fd, short events, void *ctx) {
  if (ctx) {
    EVT_IO* pEvt = (EVT_IO*)ctx;
    if (pEvt->m_pCallback) {
      pEvt->m_pCallback(events, pEvt->m_pUserArgs);
    }
  }
}
