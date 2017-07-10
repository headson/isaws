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
  CDpClient(const char *s_dp_ip, unsigned short n_dp_port,
            vzconn::EVT_LOOP          *p_evt_loop);

 public:
  static CDpClient* Create(const char *s_dp_ip, unsigned short n_dp_port);
  ~CDpClient();

 public:
  // return 0=timeout,1=success
  virtual int RunLoop(unsigned int n_timeout);

 public:
  void  Reset();
  bool  CheckAndConnected();

 public:
  int GetSessionIDFromServer();

  int SendDpMessage(const char    *p_method,
                    unsigned char  n_session_id,
                    const char    *p_data,
                    unsigned int   n_data);

  // return VZNETDP_FAILURE, ERROR
  // return > 0, request id
  int SendDpRequest(const char                *p_method,
                    unsigned char              n_chn_id,
                    const char                *p_data,
                    unsigned int               n_data,
                    DpClient_MessageCallback   p_callback,
                    void                      *p_user_arg,
                    unsigned int               n_timeout);

  // return VZNETDP_FAILURE / or VZNETDP_SUCCEED
  int SendDpReply(const char    *p_method,
                  unsigned char  n_chn_id,
                  unsigned int   n_msg_id,
                  const char    *p_data,
                  unsigned int   n_data);

 protected:
  int SendMessage(unsigned char             n_type,
                  const char               *p_method,
                  unsigned int              n_message_id,
                  const char               *p_data,
                  unsigned int              n_data);

 protected:
  virtual int HandleRecvPacket(vzconn::VSocket  *p_cli,
                               const char       *p_data,
                               unsigned int      n_data,
                               unsigned short    n_flag);
  virtual int HandleSendPacket(vzconn::VSocket *p_cli) {
    return 0;
  }
  virtual void  HandleClose(vzconn::VSocket *p_cli);

 public:
  unsigned int get_session_id() {
    return n_session_id_;
  }
  unsigned int new_msg_id() {
    if (n_message_id_ >= MAX_MESSAGE_ID) {
      n_message_id_ = 1;
    }

    n_message_id_++;
    return n_session_id_ + n_message_id_;
  }
  unsigned int get_msg_id() {
    return n_cur_msg_id_;
  }

  int get_ret_type() {
    return n_ret_type_;
  }

 protected:
  vzconn::EVT_LOOP         *p_evt_loop_;    //

 public:
  unsigned int              n_ret_type_;    // 回执结果,也做evt loop退出标签
  DpMessage                *p_cur_dp_msg_;  //

 protected:
  int                     n_session_id_;  // SESSION ID
  unsigned int              n_message_id_;  // ID[8bit] + 包序号[24bit]

  unsigned int              n_cur_msg_id_;  // 当前发送msg id

  static const unsigned int MAX_MESSAGE_ID = 0X00FFFFFF;

 protected:
  char                      dp_addr_[64];
  unsigned short            dp_port_;

 public:
  static int EncDpMsg(DpMessage      *p_msg,
                      unsigned char   n_type,
                      unsigned char   n_session_id,
                      const char     *method,
                      unsigned int    n_message_id,
                      int             n_data_size);

  static DpMessage *DecDpMsg(const void   *p_data,
                             unsigned int  n_data);
};

#endif  // LIBDISPATCH_CDPCLIENT_H_
