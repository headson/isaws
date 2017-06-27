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
  CKvdbClient();
  virtual void  Remove() { }

 public:
  static CKvdbClient* Create();
  virtual ~CKvdbClient();

 public:
  // return 0=timeout,1=success
  int32 RunLoop(uint32 n_timeout);

 public:
  /***********************************************************************
  *Description : 
  *Parameters  : p_callback[IN] 回调函数指针
  *              p_user_arg[IN] 回调函数参数
  *              p_get_data[IN] 返回数据buffer
  *              n_get_data[IN] 返回数据buffer长度
  *Return      :
  ***********************************************************************/
  void Reset(Kvdb_GetKeyCallback  p_callback,
             void                *p_user_arg,
             uint8               *p_get_data,
             uint32               n_get_data);

  /* method;add\remove */
  bool SetKey(const char *p_key,
              uint8       n_key,
              const char *p_value,
              uint32      n_value);

  bool GetKey(const char *p_key,
              uint8       n_key,
              void       *p_value,
              uint32      n_value,
              bool        absolute = false);

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

 protected:
  vzconn::EVT_LOOP          evt_loop_;    //

 protected:
  // Get Key
  uint8                     s_key_[MAX_KVDB_KEY_SIZE];
  uint32                    n_key_;

  uint32                    n_resp_ret_;  // 回执

  Kvdb_GetKeyCallback       p_callback_;  // 回调
  void                     *p_usr_arg_;   // 回调用户参数

  uint8                    *p_get_data_;  // 回调
  int32                     n_get_data_;

 protected:
  uint32                   n_msg_id_;

 public:
  int EncKvdbMsg(KvdbMessage    *p_msg,
                 uint8           type,
                 const char     *p_key,
                 uint8           n_key,
                 uint32          n_value);
  KvdbMessage *DecKvdbMsg(const uint8 *p_data, uint32 n_data);
};

#endif  // LIBDPCLIENT_CKVDBCLIENT_H_
