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

#define EVT_READ          EV_READ     // ���¼�
#define EVT_WRITE         EV_WRITE    // д�¼�

// �����¼�������ִ�к�����¼ӵ������еȴ���һ�μ�����򼤻�ִ�к���Զ��Ƴ�
#define EVT_PERSIST       EV_PERSIST

class EVT_LOOP;
///TIMER///////////////////////////////////////////////////////////////////////
class EVT_TIMER {
 private:
  struct event    event_;           //
  EVT_LOOP*       base_event_;      //

  EVT_FUNC        callback_;        // ��Ϣ�ص�
  void*           usr_args_;        // �ص�����

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
  *Description : ������ʱ��
  *Parameters  : after_ms[IN] �ӳٶ���msִ��,��repeat_ms��Ϊ0ʱ��������
  *              repeat_ms[IN] �˲�Ϊ0,ע��Ϊ���ó�ʱ�¼�,�ӳ�repeat_msִ��һ��
  *Return      : 0 �ɹ�
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

  EVT_FUNC        callback_;        // ��Ϣ�ص�
  void*           usr_args_;        // �ص�����
  unsigned int          init_, start_;

 public:
  EVT_IO();

  void           Init(const EVT_LOOP* loop, EVT_FUNC func, void* usr_arg);

  /************************************************************************
  *Description : ������ʱ��
  *Parameters  : hdl[IN] ���
  *              evt[IN] �¼�����;EV_READ\EV_WRITE\EV_PERSIST
  *              ms_timeout[IN] ��ʱʱ��
  *Return      : 0 �ɹ�
  ************************************************************************/
  int           Start(SOCKET hdl, int evt, unsigned int ms_timeout=0);
  void            Stop();

  // �û����������¼�,�����¼��ص�ʱʹ��
  void            ActiceEvent();

 private:
  static void     evt_callback(evutil_socket_t fd, short events, void *ctx);
};

///LOOP////////////////////////////////////////////////////////////////////////
class EVT_LOOP {
 private:
  struct event_base* base_event_;     // ֻ���ڵ�һλ

  unsigned int             running_;        // ����״̬
  EVT_TIMER          evt_exit_timer_; // �˳���ʱ��

 public:
  EVT_LOOP();
  virtual ~EVT_LOOP();

  int   Start();
  void    Stop();

  // ms_timeout > 0,��ʱ�˳�
  // ms_timeout = 0,����һ��;
  int   RunLoop(unsigned int ms_timeout = 0);

  // ��ʱ�˳�,0=�����˳�
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
*Description : �˳��źż���
*Parameters  : evt_srv[IN] �¼��ַ���ָ��
*              is_exit[IN]
*Return      :
************************************************************************/
void ExitSignalHandle(vzconn::EventService *evt_srv,
                      unsigned int *is_exit);

#ifdef __cplusplus
};
#endif
#endif  // LIBVZCONN_CLIBEVENT_H_
