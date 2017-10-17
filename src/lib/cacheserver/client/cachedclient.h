/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#ifndef LIBFCACHECLIENT_CACHEDCLIENT_H_
#define LIBFCACHECLIENT_CACHEDCLIENT_H_

#include "vzbase/base/basictypes.h"

#include "cachedclient_c.h"

#include "cacheserver/base/basedefines.h"
#include "vzconn/sync/ctcpclient.h"

namespace cached {

#define DEF_TIMEOUT_MSEC   1000           // 默认超时 1S

class CachedClient : public vzconn::CTcpClient,
  public vzconn::CClientInterface {
 protected:
  CachedClient(const char *server, unsigned short port);
  virtual void  Remove() {}

 public:
  static CachedClient* Create(const char *server, unsigned short port);
  virtual ~CachedClient();

 public:
  int32 RunLoop(uint32 n_timeout);

 protected:
  bool CheckAndConnected();

 public:
  bool SaveCachedFile(const char *path, int path_size,
                      const char *data, int data_size);

  bool GetCachedFile(const char *path, int path_size,
                     Cached_GetFileCallback call_back,
                     void *user_data);

  bool GetCachedFile(const char *path, int path_size, std::string *buffer);

  bool DeleteCachedFile(const char *path, int key_size);

 protected:
  virtual int32 HandleRecvPacket(vzconn::VSocket  *p_cli,
                                 const uint8      *p_data,
                                 uint32            n_data,
                                 uint16            n_flag);
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

  int32 get_ret_type() {
    return n_ret_type_;
  }

 protected:
  vzconn::EVT_LOOP          evt_loop_;    //

 protected:
  int32                     n_ret_type_;  // 回执结果,也做evt loop退出标签

  uint32                    n_cur_msg_;
  CacheMessage             *p_cur_msg_;
  
  Cached_GetFileCallback    p_callback_;
  void                     *p_usr_arg_;
  std::string               s_filepath_;

 protected:
  uint32                    n_message_id_;    // 包序号[32bit]
  uint32                    n_cur_msg_id_;    // 当前发送msg id

 protected:
  char                      s_addr_[64];
  unsigned short            n_port_;

 public:
  int EncCacheMsg(CacheMessage   *p_msg,
                  uint8           n_type,
                  const char     *p_path,
                  uint8           n_path,
                  uint32          n_data);
  CacheMessage *DecCacheMsg(const uint8 *p_data, uint32 n_data);
};
}

#endif  // LIBFCACHECLIENT_CACHEDCLIENT_H_
