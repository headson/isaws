/************************************************************************/
/* Author      : Sober.Peng 17-06-21
/* Description :
/************************************************************************/
#ifndef LIBDISPATCH_CDPCLIENT_H_
#define LIBDISPATCH_CDPCLIENT_H_

#include "basictypes.h"
#include "dpclient_c.h"
#include "vzconn/async/cevttcpclient.h"

namespace dp {

class CClientProcess : public vzconn::CClientInterface {
 public:
  virtual int32 HandleRecvPacket(vzconn::VSocket  *p_cli,
                                 const uint8            *p_data,
                                 uint32                  n_data,
                                 uint16                  n_flag);
  virtual int32 HandleSendPacket(vzconn::VSocket *p_cli) {
    return 0;
  }
  virtual void  HandleClose(vzconn::VSocket *p_cli);
};

class CTcpClient : public vzconn::CEvtTcpClient {
 protected:
  CTcpClient();
  virtual void  Remove() { }

 public:
  static CTcpClient* Create();
  virtual ~CTcpClient();

 public:
  // 设置已打开的SCOKET
  virtual bool  Open(SOCKET s, bool b_block = false);

  int32         RunLoop(uint32 n_timeout);

  void          SetCallback(DpClient_MessageCallback callback,
                            void *p_usr_arg);

  void          Reset();

 public:
  /* method;add\remove */
  int32 ListenMessage(uint8        e_type,
                      const char  *method_set[],
                      unsigned int set_size,
                      uint16       n_flag);

  int32 SendMessage(unsigned char              n_type,
                    const char                *p_method,
                    unsigned int               n_msg_id,
                    const char                *p_data,
                    int                       n_data,
                    DpClient_MessageCallback  call_back,
                    void                     *user_data);

 protected:
  virtual int32 OnRecv();
  virtual int32 OnSend();

 public:
  friend class CClientProcess;

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

  uint32 get_resp_type() {
    return n_resp_type_;
  }

 protected:
  vzconn::EVT_LOOP          evt_loop_;    //

 protected:
  DpClient_MessageCallback  callback_;    // 回调
  void                     *p_usr_arg_;   // 回调用户参数
  CClientProcess            c_cli_proc_;  // 消息处理

 protected:
  int32                     n_session_id_;    // SESSION ID
  uint32                    n_message_id_;    // ID[8bit] + 包序号[24bit]

  uint32                    n_cur_msg_id_;   // 当前发送msg id

 protected:
  uint32                    n_resp_type_;

  static const int          MAX_MESSAGE_ID = 0X00FFFFFF;

 public:
  static int EncDpMsg(DpMessage      *p_msg,
                      unsigned char   n_type,
                      unsigned char   n_session_id,
                      const char     *method,
                      unsigned int    n_message_id,
                      int             data_size);

  static int DecDpMsg(DpMessage     *p_msg,
                      const void    *p_data,
                      uint32         n_data);
};

}  // namespace dp

#endif  // LIBDISPATCH_CDPCLIENT_H_
