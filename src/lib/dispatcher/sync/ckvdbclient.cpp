/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "ckvdbclient.h"

#include "vzbase/helper/stdafx.h"
#include "dispatcher/base/pkghead.h"

CKvdbClient::CKvdbClient(const char *server, unsigned short port)
  : vzconn::CTcpClient(&evt_loop_, this)
  , n_ret_type_((uint32)-1)
  , p_callback_(NULL)
  , p_usr_arg_(NULL)
  , p_get_data_(NULL)
  , n_get_data_(0) {
  n_key_ = 0;

  n_message_id_ = 0;
  evt_loop_.Start();

  kvdb_port_ = port;

  memset(kvdb_addr_, 0, 64);
  strncpy(kvdb_addr_, server, 63);
}

CKvdbClient* CKvdbClient::Create(const char *server, unsigned short port) {
  if (NULL == server || port <= 0) {
    LOG(L_ERROR) << "param is error.";
    return NULL;
  }

  return (new CKvdbClient(server, port));
}

CKvdbClient::~CKvdbClient() {
  c_evt_recv_.Stop();
  c_evt_send_.Stop();

  evt_loop_.Stop();
}

int32 CKvdbClient::RunLoop(uint32 n_timeout) {
  int32 n_ret = 0;
  for (uint32 i = 0; i < n_timeout / 10; i++) {
    n_ret = evt_loop_.RunLoop(10);
    if (n_ret_type_ != (uint32)-1) {
      return 1;
    }
  }
  return 0;
}

void CKvdbClient::Reset(Kvdb_GetKeyCallback  callback,
                        void                *p_usr_arg,
                        uint8               *p_get_data,
                        uint32               n_get_data) {
  n_ret_type_ = (uint32)-1;

  p_callback_ = callback;
  p_usr_arg_  = p_usr_arg;

  p_get_data_ = p_get_data;
  n_get_data_ = n_get_data;

  p_get_strm_ = NULL;

  // loop 没在运行,清空标记
  if (!evt_loop_.isRuning()) {
    //c_recv_data_.Clear();
    //c_send_data_.Clear();

    //c_evt_recv_.Stop();
    //c_evt_send_.Stop();
  }
}

void CKvdbClient::Reset(Kvdb_GetKeyCallback p_callback,
                        void               *p_user_arg,
                        std::string        *p_get_data) {
  n_ret_type_ = (uint32)-1;

  p_callback_ = p_callback;
  p_usr_arg_  = p_user_arg;

  p_get_data_ = NULL;
  n_get_data_ = 0;

  p_get_strm_ = p_get_data;

  // loop 没在运行,清空标记
  if (!evt_loop_.isRuning()) {
    //c_recv_data_.Clear();
    //c_send_data_.Clear();

    //c_evt_recv_.Stop();
    //c_evt_send_.Stop();
  }
}

bool CKvdbClient::CheckAndConnected() {
  if (isClose()) {
    Close();

    vzconn::CInetAddr c_remote_addr(kvdb_addr_, kvdb_port_);
    bool b_ret = Connect(&c_remote_addr, false, true, DEF_TIMEOUT_MSEC);
    if (b_ret == false) {
      LOG(L_ERROR) << "can't connect kvdb server.";
      return b_ret;
    }
  }
  return true;
}

bool CKvdbClient::SetKey(const char *p_key,
                         uint8       n_key,
                         const char *p_value,
                         uint32      n_value) {
  if (CheckAndConnected() == false) {
    return false;
  }

  int32 n_ret = 0;
  KvdbMessage c_head;
  n_ret = EncKvdbMsg(&c_head,
                     KVDB_REPLACE,
                     p_key, n_key,
                     n_value);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "param is error.";
    return false;
  }

  iovec iov[3];
  iov[0].iov_base = &c_head;
  iov[0].iov_len  = sizeof(c_head);
  iov[1].iov_base = (void*)p_value;
  iov[1].iov_len  = (size_t)n_value;

  Reset(NULL, NULL, NULL, 0);
  n_ret = AsyncWrite(iov, 2, 0);
  if (n_ret < 0) {
    LOG(L_ERROR) << "async write failed "<<p_key;
    return false;
  }
  n_ret = RunLoop(DEF_TIMEOUT_MSEC);
  if (n_ret == 0) {
    LOG(L_ERROR) << "set key time out";
    return false;
  }
  return (n_ret_type_ == KVDB_SUCCEED);
}

bool CKvdbClient::GetKey(const char *p_key,
                         uint8       n_key,
                         void       *p_value,
                         uint32      n_value,
                         bool        absolute) {
  if (CheckAndConnected() == false) {
    return false;
  }

  int32 n_ret = 0;
  KvdbMessage c_head;
  n_ret = EncKvdbMsg(&c_head,
                     KVDB_SELECT,
                     p_key, n_key,
                     0);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "param is error.";
    return false;
  }

  Reset(NULL, NULL, (uint8*)p_value, n_value);
  n_ret = AsyncWrite(&c_head, sizeof(c_head), 0);
  if (n_ret < 0) {
    LOG(L_ERROR) << "async write failed " << p_key;
    return false;
  }

  n_ret = RunLoop(DEF_TIMEOUT_MSEC);
  if (n_ret == 0) {
    LOG(L_ERROR) << "get key time out";
    return false;
  }
  return (n_ret_type_ == KVDB_SUCCEED);
}

bool CKvdbClient::GetKey(const char          *p_key,
                         uint8                n_key,
                         Kvdb_GetKeyCallback  p_callback,
                         void                *p_usr_arg,
                         bool                 absolute /*= false*/) {
  if (CheckAndConnected() == false) {
    return false;
  }

  if (n_key > (MAX_KVDB_KEY_SIZE-1)) {
    LOG(L_ERROR) << "key is length than "<<MAX_KVDB_KEY_SIZE;
    return false;
  }
  strncpy((char*)s_key_, p_key, MAX_KVDB_KEY_SIZE);
  s_key_[n_key] = '\0';
  n_key_ = n_key;

  int32 n_ret = 0;
  KvdbMessage c_head;
  n_ret = EncKvdbMsg(&c_head,
                     KVDB_SELECT,
                     p_key,
                     n_key,
                     0);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "param is error.";
    return false;
  }

  Reset(p_callback, p_usr_arg, NULL, 0);
  n_ret = AsyncWrite(&c_head, sizeof(c_head), 0);
  if (n_ret < 0) {
    LOG(L_ERROR) << "async write failed " << p_key;
    return false;
  }

  n_ret = RunLoop(DEF_TIMEOUT_MSEC);
  if (n_ret == 0) {
    LOG(L_ERROR) << "get key time out";
    return false;
  }
  return (n_ret_type_ == KVDB_SUCCEED);
}

bool CKvdbClient::GetKey(const char  *p_key, 
                         uint8        n_key, 
                         std::string *p_value, 
                         bool         absolute /*= false*/) {
  if (CheckAndConnected() == false) {
    return false;
  }

  int32 n_ret = 0;
  KvdbMessage c_head;
  n_ret = EncKvdbMsg(&c_head,
                     KVDB_SELECT,
                     p_key, n_key,
                     0);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "param is error.";
    return false;
  }

  Reset(NULL, NULL, p_value);
  n_ret = AsyncWrite(&c_head, sizeof(c_head), 0);
  if (n_ret < 0) {
    LOG(L_ERROR) << "async write failed " << p_key;
    return false;
  }

  n_ret = RunLoop(DEF_TIMEOUT_MSEC);
  if (n_ret == 0) {
    LOG(L_ERROR) << "get key time out";
    return false;
  }
  return (n_ret_type_ == KVDB_SUCCEED);
}

bool CKvdbClient::GetKey(const std::string s_key, 
                         std::string      *p_value, 
                         bool              absolute /*= false*/) {
  return GetKey(s_key.c_str(), s_key.size(), p_value, absolute);
}

bool CKvdbClient::Delete(const char *p_key, uint8 n_key) {
  if (CheckAndConnected() == false) {
    return false;
  }

  int32 n_ret = 0;
  KvdbMessage c_head;
  n_ret = EncKvdbMsg(&c_head,
                     KVDB_DELETE,
                     p_key, n_key,
                     0);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "param is error.";
    return false;
  }

  Reset(NULL, NULL, NULL, 0);
  n_ret = AsyncWrite(&c_head, sizeof(c_head), 0);
  if (n_ret < 0) {
    LOG(L_ERROR) << "async write failed " << p_key;
    return false;
  }

  n_ret = RunLoop(DEF_TIMEOUT_MSEC);
  if (n_ret == 0) {
    LOG(L_ERROR) << "delete key time out";
    return false;
  }
  return (n_ret_type_ == KVDB_SUCCEED);
}

bool CKvdbClient::BackupDatabase() {
  if (CheckAndConnected() == false) {
    return false;
  }

  int32 n_ret = 0;
  KvdbMessage c_head;
  n_ret = EncKvdbMsg(&c_head,
                     KVDB_BACKUP,
                     "fk", 2,
                     0);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "param is error.";
    return false;
  }

  Reset(NULL, NULL, NULL, 0);
  n_ret = AsyncWrite(&c_head, sizeof(c_head), 0);
  if (n_ret < 0) {
    LOG(L_ERROR) << "async write failed ";
    return false;
  }

  n_ret = RunLoop(DEF_TIMEOUT_MSEC);
  if (n_ret == 0) {
    LOG(L_ERROR) << "bakup time out";
    return false;
  }
  return (n_ret_type_ == KVDB_SUCCEED);
}

bool CKvdbClient::RestoreDatabase() {
  if (CheckAndConnected() == false) {
    return false;
  }

  int32 n_ret = 0;
  KvdbMessage c_head;
  n_ret = EncKvdbMsg(&c_head,
                     KVDB_RESTORE,
                     "fk", 2,
                     0);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "param is error.";
    return false;
  }

  Reset(NULL, NULL, NULL, 0);
  n_ret = AsyncWrite(&c_head, sizeof(c_head), 0);
  if (n_ret < 0) {
    LOG(L_ERROR) << "async write failed ";
    return false;
  }

  n_ret = RunLoop(DEF_TIMEOUT_MSEC);
  if (n_ret == 0) {
    LOG(L_ERROR) << "restore time out";
    return false;
  }
  return (n_ret_type_ == KVDB_SUCCEED);
}

int32 CKvdbClient::HandleRecvPacket(vzconn::VSocket *p_cli,
                                    const uint8     *p_data,
                                    uint32           n_data,
                                    uint16           n_flag) {
  //LOG(L_WARNING) << "recv packet length " << n_data;
  if (!p_cli || !p_data || n_data == 0) {
    return -1;
  }

  if (n_data >= sizeof(KvdbMessage)) {
    KvdbMessage *p_msg = DecKvdbMsg(p_data, n_data);
    if (p_msg->id != get_msg_id()) {
      LOG(L_WARNING) << "id is not current message id.";
      return 0;
    }

    evt_loop_.LoopExit(0);
    n_ret_type_ = (uint32)p_msg->type;

    int32 n_value  = n_data - sizeof(KvdbMessage);
    // 回调
    if (p_callback_) {
      p_callback_((char*)s_key_, n_key_, (char*)p_msg->value, n_value, p_usr_arg_);
    }
    // C
    if (p_get_data_) {
      if (n_value <= n_get_data_) {
        memcpy(p_get_data_, p_msg->value, n_value);
        n_get_data_ = n_value;
      } else {
        LOG(L_ERROR) << "buffer small than result.";
      }
    }
    // C++
    if (p_get_strm_) {
      p_get_strm_->append(p_msg->value, n_value);
    }
  } else {
    n_ret_type_ = KVDB_FAILURE;
    LOG(L_ERROR) << "unkown the response.";
  }
  return 0;
}

int CKvdbClient::EncKvdbMsg(KvdbMessage   *p_msg,
                            uint8          n_type,
                            const char    *p_key,
                            uint8          n_key,
                            uint32         n_value) {
  memset(p_msg, 0, sizeof(KvdbMessage));

  p_msg->type   = n_type;
  p_msg->id     = get_msg_id();

  n_cur_msg_id_ = p_msg->id;

  strncpy(p_msg->key, p_key, MAX_KVDB_KEY_SIZE);
  return sizeof(KvdbMessage);
}

KvdbMessage *CKvdbClient::DecKvdbMsg(const uint8 *p_data,
                                     uint32       n_data) {
  KvdbMessage *p_msg = (KvdbMessage*)p_data;

  return p_msg;
}
