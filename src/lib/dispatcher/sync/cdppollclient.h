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

 public:
  static CDpPollClient* Create(const char *server, unsigned short port,
                               DpClient_MessageCallback   p_msg_cb,
                               void                      *p_msg_usr_arg,
                               DpClient_PollStateCallback p_state_cb,
                               void                      *p_state_usr_arg,
                               vzconn::EVT_LOOP          *p_evt_loop);
  virtual ~CDpPollClient();

 public:
  /* method;add\remove */
  int ListenMessage(unsigned char  e_type,
                    const char    *p_method[],    /* 方法名组成32Byte长度发送 */
                    unsigned int   n_method_cnt,  /* 方法个数 */
                    unsigned short n_flag);

  // 轮询使用
  // return 0=timeout,1=success
  virtual int RunLoop(unsigned int n_timeout);

  vzconn::EVT_LOOP *GetEvtLoop() {
    return p_evt_loop_;
  }

 protected:
  virtual int HandleRecvPacket(vzconn::VSocket  *p_cli,
                               const unsigned char            *p_data,
                               unsigned int                  n_data,
                               unsigned short                  n_flag);
  virtual int HandleSendPacket(vzconn::VSocket *p_cli) {
    return 0;
  }
  virtual void  HandleClose(vzconn::VSocket *p_cli) {
  }

  static int evt_timer_cb(SOCKET          fd,
                          short           events,
                          const void      *p_usr_arg) {
    if (p_usr_arg) {
      ((CDpPollClient*)p_usr_arg)->OnEvtTimer();
    }
    return 0;
  }

  int OnEvtTimer();

 public:
  void SetResMsgFlag(unsigned int n_flag) {
    had_reg_msg_ = n_flag;
  }

 protected:
  vzconn::EVT_TIMER            c_evt_timer_;   // 检查断网
  unsigned int                       had_reg_msg_;   // 0=需要注册消息,1=已注册消息

 protected:
  DpClient_MessageCallback     p_poll_msg_cb_;        // 消息回调
  void                        *p_poll_msg_usr_arg_;   // 回调用户参数

  DpClient_PollStateCallback   p_state_cb_;           // 状态回调
  void                        *p_state_usr_arg_;      // 回调用户参数
};

#endif  // LIBDISPATCH_CDPCLIENT_H_
