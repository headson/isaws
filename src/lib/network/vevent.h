/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : vbaseevent.h
* Author        : Sober.Peng
* Date          : 18:1:2017
* Description   :
*-----------------------------------------------------------------------------
* Modify        :
*-----------------------------------------------------------------------------
******************************************************************************/
#pragma once
#include "base/vtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "event2/event-config.h"

#include "event2/util.h"
#include "event2/event.h"
#include "event2/buffer.h"
#include "event2/thread.h"
#include "event2/listener.h"
#include "event2/bufferevent.h"
#include "event2/event_compat.h"
#include "event2/event_struct.h"

#ifdef __cplusplus
}
#endif

#include <string>

typedef int32_t (*EVT_FUNC)(int32_t n_evt, const void* p_usr_arg);

#define EVT_READ        EV_READ
#define EVT_WRITE       EV_WRITE
#define EVT_PERSIST     EV_PERSIST
///LOOP////////////////////////////////////////////////////////////////////////
class EVT_LOOP {
 public:
  EVT_LOOP();
  virtual ~EVT_LOOP();

 public:
  int32_t         Start();
  void            Stop();

  int32_t         Runing();

  struct event_base* get_event() const {
    return p_event_;
  }

 protected:
  struct event_base* p_event_;
};

///TIMER///////////////////////////////////////////////////////////////////////
class EVT_TIMER {
 public:
  struct event    c_evt_;       // 
  EVT_LOOP*       p_base_;      //

  EVT_FUNC        p_callback_;  // 消息回调
  void*           p_usr_args_;  // 回调参数

  std::string     s_name_;      // 名称
  bool            b_init_, b_start_;

 public:
  EVT_TIMER();
  void                SetName(const std::string& sName);
  const std::string&  GetName() const;

  void            Init(const EVT_LOOP* loop, EVT_FUNC func, void* pArg);
  int32_t         Start(uint32_t after, uint32_t repeat);
  void            Stop();

  static void     evt_callback(int fd, short event, void *ctx);
};

///IO//////////////////////////////////////////////////////////////////////////
class EVT_IO {
 public:
  struct event    c_evt_;       //
  EVT_LOOP*       p_base_;      //

  EVT_FUNC        p_callback_;  // 消息回调
  void*           p_usr_args_;  // 回调参数

  std::string     s_name_;      // 名称
  bool            b_init_, b_start_;

 public:
  EVT_IO();

  void                SetName(const std::string& s_name);
  const std::string&  GetName() const;

  void            Init(const EVT_LOOP* loop, EVT_FUNC func, void* p_arg);
  int32_t         Start(SOCKET v_hdl, int32_t nEvt);
  void            Stop();

  static void     evt_callback(evutil_socket_t fd, short events, void *ctx);
};
