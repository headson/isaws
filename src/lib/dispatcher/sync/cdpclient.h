/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#ifndef LIBDISPATCH_CDPCLIENT_H_
#define LIBDISPATCH_CDPCLIENT_H_

#include "vzbase/base/basictypes.h"

#include "dpclient_c.h"
#include "vzconn/sync/ctcpclient.h"

class CDpClient : public vzconn::CTcpClient,
  public vzconn::CClientInterface {
 protected:
  CDpClient(const char *server, unsigned short port,
            DpClient_MessageCallback   p_msg_cb,
            void                      *p_msg_usr_arg,
            DpClient_PollStateCallback p_state_cb,
            void                      *p_state_usr_arg,
            vzconn::EVT_LOOP          *p_evt_loop);
  virtual void  Remove() { }

 public:
  static CDpClient* Create(const char *server, unsigned short port,
                           DpClient_MessageCallback   p_msg_cb=NULL,
                           void                      *p_msg_usr_arg=NULL,
                           DpClient_PollStateCallback p_state_cb=NULL,
                           void                      *p_state_usr_arg=NULL,
                           vzconn::EVT_LOOP          *p_evt_loop = NULL);
  virtual ~CDpClient();

 public:
  // return 0=timeout,1=success
  int32 RunLoop(uint32 n_timeout);

  // 轮询使用
  // return 0=timeout,1=success
  int32 PollRunLoop(uint32 n_timeout);

  vzconn::EVT_LOOP *GetEvtLoop() {
    return p_evt_loop_;
  }

 public:
  void  Reset(DpClient_MessageCallback callback, void *p_usr_arg);

  bool  CheckAndConnected();

 public:
  /* method;add\remove */
  int32 ListenMessage(uint8        e_type,
                      const char  *p_method[],    /* 方法名组成32Byte长度发送 */
                      unsigned int n_method_no,   /* 方法个数 */
                      uint16       n_flag);

  int32 SendMessage(unsigned char             n_type,
                    const char               *p_method,
                    unsigned int              n_method,
                    const char               *p_data,
                    int                       n_data,
                    DpClient_MessageCallback  p_callback,
                    void                     *p_user_arg);

 protected:
  virtual int32 HandleRecvPacket(vzconn::VSocket  *p_cli,
                                 const uint8            *p_data,
                                 uint32                  n_data,
                                 uint16                  n_flag);
  virtual int32 HandleSendPacket(vzconn::VSocket *p_cli) {
    return 0;
  }
  virtual void  HandleClose(vzconn::VSocket *p_cli);

  static int32 evt_timer_cb(SOCKET          fd,
                            short           events,
                            const void      *p_usr_arg) {
    if (p_usr_arg) {
      ((CDpClient*)p_usr_arg)->OnEvtTimer();
    }
    return 0;
  }

  int32 OnEvtTimer();

 public:
  uint32 get_session_id() {
    return n_session_id_;
  }
  uint32 new_msg_id() {
    if (n_message_id_ >= MAX_MESSAGE_ID) {
      n_message_id_ = 1;
    }

    n_message_id_++;
    return n_session_id_ + n_message_id_;
  }
  uint32 get_msg_id() {
    return n_cur_msg_id_;
  }

  int32 get_ret_type() {
    return n_ret_type_;
  }

 protected:
  vzconn::EVT_LOOP         *p_evt_loop_;    //
  vzconn::EVT_TIMER         c_evt_timer_;   // 检查断网

 protected:
  DpClient_MessageCallback     p_msg_cb_;    // 消息回调
  void                        *p_msg_usr_arg_;   // 回调用户参数

  DpClient_MessageCallback     p_poll_msg_cb_;    // 消息回调
  void                        *p_poll_msg_usr_arg_;   // 回调用户参数

  DpClient_PollStateCallback   p_state_cb_;  // 状态回调
  void                        *p_state_usr_arg_;   // 回调用户参数

 protected:
  int32                     n_session_id_;    // SESSION ID
  uint32                    n_message_id_;    // ID[8bit] + 包序号[24bit]

  uint32                    n_cur_msg_id_;    // 当前发送msg id

 protected:
  uint32                    n_ret_type_;      // 回执结果,也做evt loop退出标签

 protected:
  uint32                    b_poll_enabel_;   // poll 使能
  uint32                    n_recv_packet_;   // poll loop退出标签

  static const uint32       MAX_MESSAGE_ID = 0X00FFFFFF;

 protected:
  char                      dp_addr_[64];
  unsigned short            dp_port_;

 public:
  static int EncDpMsg(DpMessage      *p_msg,
                      unsigned char   n_type,
                      unsigned char   n_session_id,
                      const char     *method,
                      unsigned int    n_message_id,
                      int             data_size);

  static DpMessage *DecDpMsg(const void *p_data,
                             uint32      n_data);
};

#endif  // LIBDISPATCH_CDPCLIENT_H_
