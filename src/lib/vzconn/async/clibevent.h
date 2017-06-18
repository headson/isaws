/************************************************************************/
/* Author      : Sober.Peng 17-06-15
/* Description :
/************************************************************************/
#ifndef LIBVZCONN_CLIBEVENT_H_
#define LIBVZCONN_CLIBEVENT_H_

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

#include "basictypes.h"

typedef int32 (*EVT_FUNC)(SOCKET          fd,
                          short           events, 
                          const void      *p_usr_arg);

#define EVT_READ        EV_READ     // 读事件
#define EVT_WRITE       EV_WRITE    // 写事件

// 永久事件，激活执行后会重新加到队列中等待下一次激活，否则激活执行后会自动移除  
#define EVT_PERSIST     EV_PERSIST  
///LOOP////////////////////////////////////////////////////////////////////////
class EVT_LOOP {
 private:
  struct event_base* p_event_;

 public:
  EVT_LOOP();
  virtual ~EVT_LOOP();

 public:
  int32   Start();
  void    Stop();

  int32   RunLoop();

  struct event_base* get_event() const {
    return p_event_;
  }
};

///TIMER///////////////////////////////////////////////////////////////////////
class EVT_TIMER {
 private:
  struct event    c_evt_;       //
  EVT_LOOP*       p_base_;      //

  EVT_FUNC        p_callback_;  // 消息回调
  void*           p_usr_args_;  // 回调参数

<<<<<<< HEAD:src/lib/vzconn/async/clibevent.h
  uint32          b_init_, b_start_;

 public:
  EVT_TIMER();
=======
  std::string     s_name_;      // 名称
  bool            b_init_, b_start_;

 public:
  EVT_TIMER();
  void                SetName(const std::string& sName);
  const std::string&  GetName() const;
>>>>>>> 8c471fd87910ea7f532ac1fc43711c32142c523a:src/lib/network/vevent.h

  void            Init(const EVT_LOOP* loop, EVT_FUNC func, void* pArg);
  int32         Start(uint32 after_ms, uint32 repeat_ms);
  void            Stop();

  static void     evt_callback(int fd, short event, void *ctx);
};

///IO//////////////////////////////////////////////////////////////////////////
class EVT_IO {
 private:
  struct event    c_evt_;       //
  EVT_LOOP*       p_base_;      //

  EVT_FUNC        p_callback_;  // 消息回调
  void*           p_usr_args_;  // 回调参数

<<<<<<< HEAD:src/lib/vzconn/async/clibevent.h
  uint32          b_init_, b_start_;
=======
  std::string     s_name_;      // 名称
  bool            b_init_, b_start_;
>>>>>>> 8c471fd87910ea7f532ac1fc43711c32142c523a:src/lib/network/vevent.h

 public:
  EVT_IO();

<<<<<<< HEAD:src/lib/vzconn/async/clibevent.h
=======
  void                SetName(const std::string& s_name);
  const std::string&  GetName() const;

>>>>>>> 8c471fd87910ea7f532ac1fc43711c32142c523a:src/lib/network/vevent.h
  void            Init(const EVT_LOOP* loop, EVT_FUNC func, void* p_arg);
  int32         Start(SOCKET v_hdl, int32 nEvt, uint32 n_timeout=0);
  void            Stop();

  // 用户主动关闭链接时调用此函数,用于唤醒事件处理已关闭链接
  void            ActiceEvent();

  static void     evt_callback(evutil_socket_t fd, short events, void *ctx);
};

#endif  // LIBVZCONN_CLIBEVENT_H_
