/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#ifndef LIBDPCLIENT_CKVDBCLIENT_H_
#define LIBDPCLIENT_CKVDBCLIENT_H_

#include "vzconn/sync/ctcpclient.h"
#include "dpclient_c.h"
#include "vzbase/base/basictypes.h"

class CKvdbClient : public vzconn::CTcpClient,
  public vzconn::CClientInterface {
 protected:
  CKvdbClient(const char *server, unsigned short port);
  virtual void  Remove() { }

 public:
  static CKvdbClient* Create(const char *server, unsigned short port);
  virtual ~CKvdbClient();

 public:
  // return 0=timeout,1=success
  int32 RunLoop(uint32 n_timeout);

 protected:
  bool CheckAndConnected();

 public:
  bool SetKey(const char *p_key,
              uint8       n_key,
              const char *p_value,
              uint32      n_value);

  bool SetKey(const std::string  s_key,
              const char *p_value,
              uint32      n_value);

  bool GetKey(const char *p_key,
              uint8       n_key,
              void       *p_value,
              uint32      n_value,
              bool        absolute = false);

  bool GetKey(const std::string  s_key,
              void       *p_value,
              uint32      n_value,
              bool           absolute = false);

  bool GetKey(const char    *p_key,
              uint8          n_key,
              std::string   *p_value,
              bool           absolute = false);

  bool GetKey(const std::string  s_key,
              std::string       *p_value,
              bool               absolute = false);

  bool GetKey(const char          *p_key,
              uint8                n_key,
              Kvdb_GetKeyCallback  p_callback,
              void                *p_usr_data,
              bool                 absolute = false);

  bool Delete(const char *p_key, uint8 n_key);
  bool BackupDatabase();
  bool RestoreDatabase();

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

 public:
  uint32 new_msg_id() {
    n_message_id_++;
    return n_message_id_;
  }
  uint32 get_msg_id() {
    return n_cur_msg_id_;
  }

 protected:
  vzconn::EVT_LOOP          evt_loop_;    //

 protected:
  uint32                    n_ret_type_;  // 回执结果,也做evt loop退出标签

  uint32                    n_cur_msg_;
  KvdbMessage              *p_cur_msg_;

protected:
  uint32                    n_message_id_;    // 包序号[32bit]
  uint32                    n_cur_msg_id_;    // 当前发送msg id

 protected:
  char                      s_addr_[64];
  unsigned short            n_port_;

 public:
  int EncKvdbMsg(KvdbMessage    *p_msg,
                 uint8           type,
                 const char     *p_key,
                 uint8           n_key,
                 uint32          n_value);
  KvdbMessage *DecKvdbMsg(const uint8 *p_data, uint32 n_data);
};

#endif  // LIBDPCLIENT_CKVDBCLIENT_H_
