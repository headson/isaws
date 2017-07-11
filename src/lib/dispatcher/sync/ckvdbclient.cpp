/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "ckvdbclient.h"

#include "vzbase/helper/stdafx.h"
#include "dispatcher/base/pkghead.h"

CKvdbClient::CKvdbClient(const char *server, unsigned short port)
  : vzconn::CTcpClient(&evt_loop_, this)
  , n_ret_type_((uint32)-1) {
  n_message_id_ = 0;
  evt_loop_.Start();

  n_port_ = port;

  memset(s_addr_, 0, 64);
  strncpy(s_addr_, server, 63);
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
  if (NULL == p_evt_loop_) {
    return -1;
  }
  n_ret = evt_loop_.RunLoop(n_timeout);
  if (n_ret_type_ != (uint32)TYPE_INVALID) {
    return 1;
  }
  return 0;
}

bool CKvdbClient::CheckAndConnected() {
  if (isClose()) {
    Close();

    vzconn::CInetAddr c_remote_addr(s_addr_, n_port_);
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

  n_ret_type_ = (uint32)KVDB_INVALID;
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

  n_ret_type_ = (uint32)KVDB_INVALID;
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

  if ((n_ret_type_ == KVDB_SUCCEED)) {
    if (n_value > (n_cur_msg_-sizeof(KvdbMessage))) {
      memcpy(p_value, p_cur_msg_->value, n_cur_msg_-sizeof(KvdbMessage));
      return true;
    }
    LOG(L_ERROR) << "return value is small than kvdb's value.";
  }
  return false;
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

  n_ret_type_ = (uint32)KVDB_INVALID;
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
  if (n_ret_type_ == KVDB_SUCCEED) {
    p_callback(p_key, n_key,
               p_cur_msg_->value,
               n_cur_msg_-sizeof(KvdbMessage),
               p_usr_arg);
    return true;
  }
  return false;
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

  n_ret_type_ = (uint32)KVDB_INVALID;
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

  if ((n_ret_type_ == KVDB_SUCCEED)) {
    p_value->append(p_cur_msg_->value,
                    (n_cur_msg_ - sizeof(KvdbMessage)));
    return true;
  }
  return false;
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

  n_ret_type_ = (uint32)KVDB_INVALID;
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

  n_ret_type_ = (uint32)KVDB_INVALID;
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

  n_ret_type_ = (uint32)KVDB_INVALID;
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
    n_cur_msg_ = n_data;
    p_cur_msg_ = DecKvdbMsg(p_data, n_data);
    if (p_cur_msg_->id != get_msg_id()) {
      LOG(L_WARNING) << "id is not current message id.";
      return 0;
    }
    n_ret_type_ = (uint32)p_cur_msg_->type;

    evt_loop_.LoopExit(0);
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
