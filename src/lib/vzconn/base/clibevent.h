/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : 
************************************************************************/
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

#include "vzbase/base/basictypes.h"

namespace vzconn {

typedef int32 (*EVT_FUNC)(SOCKET          fd,
                          short           events,
                          const void      *p_usr_arg);

#define EVT_READ          EV_READ     // ���¼�
#define EVT_WRITE         EV_WRITE    // д�¼�

// �����¼�������ִ�к�����¼ӵ������еȴ���һ�μ�����򼤻�ִ�к���Զ��Ƴ�
#define EVT_PERSIST       EV_PERSIST
///LOOP////////////////////////////////////////////////////////////////////////
class EVT_LOOP {
 public:
  struct event_base* p_event_;
  bool               b_runging_;   // ����״̬

 public:
  EVT_LOOP();
  virtual ~EVT_LOOP();

 public:
  int32   Start();
  void    Stop();

  // n_timeout>0,��ʱ�˳�
  // n_timeout=0,����ѭ��,���ǵ���LoopExit�˳�
  int32   RunLoop(uint32 n_timeout=0);

  // ��ʱ�˳�,0=�����˳�
  void    LoopExit(uint32 n_timeout);

  bool    isRuning();

  struct event_base* get_event() const {
    return p_event_;
  }
};

///TIMER///////////////////////////////////////////////////////////////////////
class EVT_TIMER {
 private:
  struct event    c_evt_;       //
  EVT_LOOP*       p_base_;      //

  EVT_FUNC        p_callback_;  // ��Ϣ�ص�
  void*           p_usr_args_;  // �ص�����

  uint32          b_init_, b_start_;

 public:
  EVT_TIMER();

  void            Init(const EVT_LOOP* loop, EVT_FUNC func, void* pArg);
  int32           Start(uint32 after_ms, uint32 repeat_ms);
  void            Stop();

  static void     evt_callback(evutil_socket_t fd, short event, void *ctx);
};

///IO//////////////////////////////////////////////////////////////////////////
class EVT_IO {
 private:
  struct event    c_evt_;       //
  EVT_LOOP*       p_base_;      //

  EVT_FUNC        p_callback_;  // ��Ϣ�ص�
  void*           p_usr_args_;  // �ص�����
  uint32          b_init_, b_start_;

 public:
  EVT_IO();

  void            Init(const EVT_LOOP* loop, EVT_FUNC func, void* p_arg);
  int32           Start(SOCKET v_hdl, int32 nEvt, uint32 n_timeout=0);
  void            Stop();

  // �û������ر�����ʱ���ô˺���,���ڻ����¼������ѹر�����
  void            ActiceEvent();

  static void     evt_callback(evutil_socket_t fd, short events, void *ctx);
};

typedef EVT_LOOP EventService;

}  // namespace vzconn
#endif  // LIBVZCONN_CLIBEVENT_H_
