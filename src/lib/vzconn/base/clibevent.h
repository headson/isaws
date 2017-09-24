/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#ifndef LIBVZCONN_CLIBEVENT_H_
#define LIBVZCONN_CLIBEVENT_H_

#include "vzbase/base/basictypes.h"

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

namespace vzconn {

typedef int (*EVT_FUNC)(SOCKET fd, short events, const void *usr_arg);

#define EVT_READ          EV_READ     // 读事件
#define EVT_WRITE         EV_WRITE    // 写事件

// 永久事件，激活执行后会重新加到队列中等待下一次激活，否则激活执行后会自动移除
#define EVT_PERSIST       EV_PERSIST

class EVT_LOOP;
///TIMER///////////////////////////////////////////////////////////////////////
class EVT_TIMER {
 private:
  struct event    event_;           //
  EVT_LOOP*       base_event_;      //

  EVT_FUNC        callback_;        // 消息回调
  void*           usr_args_;        // 回调参数

  unsigned int    init_, start_;

 public:
  EVT_TIMER();

  /************************************************************************
  *Description :
  *Parameters  :
  *Return      :
  ************************************************************************/
  void            Init(const EVT_LOOP* loop, EVT_FUNC func, void* pArg);

  /************************************************************************
  *Description : 启动定时器
  *Parameters  : after_ms[IN] 延迟多少ms执行,当repeat_ms不为0时不起作用
  *              repeat_ms[IN] 此不为0,注册为永久超时事件,延迟repeat_ms执行一次
  *Return      : 0 成功
  ************************************************************************/
  int             Start(unsigned int after_ms, unsigned int repeat_ms);
  void            Stop();

  static void     evt_callback(evutil_socket_t fd, short event, void *ctx);
};

///IO//////////////////////////////////////////////////////////////////////////
class EVT_IO {
 private:
  struct event    event_;           //
  EVT_LOOP*       base_event_;      //

  EVT_FUNC        callback_;        // 消息回调
  void*           usr_args_;        // 回调参数
  unsigned int          init_, start_;

 public:
  EVT_IO();

  void           Init(const EVT_LOOP* loop, EVT_FUNC func, void* usr_arg);

  /************************************************************************
  *Description : 启动定时器
  *Parameters  : hdl[IN] 句柄
  *              evt[IN] 事件类型;EV_READ\EV_WRITE\EV_PERSIST
  *              ms_timeout[IN] 超时时间
  *Return      : 0 成功
  ************************************************************************/
  int           Start(SOCKET hdl, int evt, unsigned int ms_timeout=0);
  void            Stop();

  // 用户主动唤醒事件,调用事件回调时使用
  void            ActiceEvent();

 private:
  static void     evt_callback(evutil_socket_t fd, short events, void *ctx);
};

///LOOP////////////////////////////////////////////////////////////////////////
class EVT_LOOP {
 private:
  struct event_base* base_event_;     // 只能在第一位

  unsigned int             running_;        // 运行状态
  EVT_TIMER          evt_exit_timer_; // 退出定时器

 public:
  EVT_LOOP();
  virtual ~EVT_LOOP();

  int   Start();
  void    Stop();

  // ms_timeout > 0,超时退出
  // ms_timeout = 0,运行一次;
  int   RunLoop(unsigned int ms_timeout = 0);

  // 定时退出,0=立刻退出
  void    LoopExit(unsigned int ms_timeout);

  bool    isRuning();

  struct event_base* get_event() const {
    return base_event_;
  }

 private:
  static int exit_callback(SOCKET      fd,
                           short       events,
                           const void *usr_arg);
};
typedef EVT_LOOP EventService;

}  // namespace vzconn

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
*Description : 退出信号监听
*Parameters  : evt_srv[IN] 事件分发器指针
*              is_exit[IN]
*Return      :
************************************************************************/
void ExitSignalHandle(vzconn::EventService *evt_srv,
                      unsigned int *is_exit);

#ifdef __cplusplus
};
#endif
#endif  // LIBVZCONN_CLIBEVENT_H_
