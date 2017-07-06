/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#ifndef LIBDISPATCH_CDPPOLLCLIENT_H_
#define LIBDISPATCH_CDPPOLLCLIENT_H_

#include "vzbase/base/basictypes.h"

#include "dpclient_c.h"

#include "vzconn/sync/ctcpclient.h"
#include "dispatcher/sync/cdpclient.h"

class CDpPollClient : public CDpClient {
 protected:
  CDpPollClient(const char *server, unsigned short port,
            DpClient_MessageCallback   p_msg_cb,
            void                      *p_msg_usr_arg,
            DpClient_PollStateCallback p_state_cb,
            void                      *p_state_usr_arg,
            vzconn::EVT_LOOP          *p_evt_loop);
  virtual void  Remove() { }

 public:
  static CDpPollClient* Create(const char *server, unsigned short port,
                           DpClient_MessageCallback   p_msg_cb=NULL,
                           void                      *p_msg_usr_arg=NULL,
                           DpClient_PollStateCallback p_state_cb=NULL,
                           void                      *p_state_usr_arg=NULL,
                           vzconn::EVT_LOOP          *p_evt_loop = NULL);
  virtual ~CDpPollClient();

 public:
  // 轮询使用
  // return 0=timeout,1=success
  int32 PollRunLoop(uint32 n_timeout);

  vzconn::EVT_LOOP *GetEvtLoop() {
    return p_evt_loop_;
  }

 protected:
  virtual int32 HandleRecvPacket(vzconn::VSocket  *p_cli,
                                 const uint8            *p_data,
                                 uint32                  n_data,
                                 uint16                  n_flag);
  virtual int32 HandleSendPacket(vzconn::VSocket *p_cli) {
    return 0;
  }
  virtual void  HandleClose(vzconn::VSocket *p_cli) {
  }

  static int32 evt_timer_cb(SOCKET          fd,
                            short           events,
                            const void      *p_usr_arg) {
    if (p_usr_arg) {
      ((CDpPollClient*)p_usr_arg)->OnEvtTimer();
    }
    return 0;
  }

  int32 OnEvtTimer();

 protected:
  vzconn::EVT_TIMER            c_evt_timer_;   // 检查断网

 protected:
  DpClient_MessageCallback     p_poll_msg_cb_;    // 消息回调
  void                        *p_poll_msg_usr_arg_;   // 回调用户参数

  DpClient_PollStateCallback   p_state_cb_;  // 状态回调
  void                        *p_state_usr_arg_;   // 回调用户参数
};

#endif  // LIBDISPATCH_CDPCLIENT_H_
