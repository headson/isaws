/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#ifndef LIBDPCLIENT_CKVDBCLIENT_H_
#define LIBDPCLIENT_CKVDBCLIENT_H_

#include "vzbase/base/basictypes.h"
#include "dpclient_c.h"

#include "dispatcher/base/pkghead.h"
#include "vzconn/sync/ctcpclient.h"

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
  int RunLoop(unsigned int n_timeout);

 protected:
  bool CheckAndConnected();

 public:
  bool SetKey(const char *p_key,
              unsigned char       n_key,
              const char *p_value,
              unsigned int      n_value);

  bool GetKey(const char *p_key,
              unsigned char       n_key,
              void       *p_value,
              unsigned int      n_value,
              bool        absolute = false);

  bool GetKey(const char    *p_key,
              unsigned char          n_key,
              std::string   *p_value,
              bool           absolute = false);

  bool GetKey(const std::string  s_key,
              std::string       *p_value,
              bool               absolute = false);

  bool GetKey(const char          *p_key,
              unsigned char                n_key,
              Kvdb_GetKeyCallback  p_callback,
              void                *p_usr_data,
              bool                 absolute = false);

  bool Delete(const char *p_key, unsigned char n_key);
  bool BackupDatabase();
  bool RestoreDatabase();

 protected:
  virtual int HandleRecvPacket(vzconn::VSocket  *p_cli,
                                 const char       *p_data,
                                 unsigned int      n_data,
                                 unsigned short    n_flag);
  virtual int HandleSendPacket(vzconn::VSocket *p_cli) {
    return 0;
  }
  virtual void  HandleClose(vzconn::VSocket *p_cli) {
  }

 public:
  unsigned int new_msg_id() {
    n_message_id_++;
    return n_message_id_;
  }
  unsigned int get_msg_id() {
    return n_cur_msg_id_;
  }

 protected:
  vzconn::EVT_LOOP          evt_loop_;    //

 protected:
  unsigned int              n_ret_type_;  // 回执结果,也做evt loop退出标签

  unsigned int              n_cur_kvdb_msg_;
  KvdbMessage              *p_cur_kvdb_msg_;

protected:
  unsigned int              n_message_id_;    // 包序号[32bit]
  unsigned int              n_cur_msg_id_;    // 当前发送msg id

 protected:
  char                      kvdb_addr_[64];
  unsigned short            kvdb_port_;

 public:
  int EncKvdbMsg(KvdbMessage    *p_msg,
                 unsigned char   n_type,
                 const char     *p_key,
                 unsigned char   n_key,
                 unsigned int    n_value);
  KvdbMessage *DecKvdbMsg(const char *p_data, unsigned int n_data);
};

#endif  // LIBDPCLIENT_CKVDBCLIENT_H_
