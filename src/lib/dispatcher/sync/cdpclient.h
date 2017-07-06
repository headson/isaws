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
            vzconn::EVT_LOOP          *p_evt_loop);
  virtual void  Remove() { }

 public:
  static CDpClient* Create(const char *server, unsigned short port,
                           vzconn::EVT_LOOP          *p_evt_loop = NULL);
  virtual ~CDpClient();

 public:
  // return 0=timeout,1=success
  int32 RunLoop(uint32 n_timeout);

 public:
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
                                 const uint8      *p_data,
                                 uint32            n_data,
                                 uint16            n_flag);
  virtual int32 HandleSendPacket(vzconn::VSocket *p_cli) {
    return 0;
  }
  virtual void  HandleClose(vzconn::VSocket *p_cli);

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

 public:
  uint32                    n_ret_type_;      // 回执结果,也做evt loop退出标签
  DpMessage                *p_cur_dp_msg_;  //

 protected:
  int32                     n_session_id_;    // SESSION ID
  uint32                    n_message_id_;    // ID[8bit] + 包序号[24bit]

  uint32                    n_cur_msg_id_;    // 当前发送msg id

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
