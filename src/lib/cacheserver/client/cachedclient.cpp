/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "cachedclient.h"

#include "vzbase/helper/stdafx.h"

namespace cached {

CachedClient::CachedClient(const char *server, unsigned short port)
  : vzconn::CTcpClient(&evt_loop_, this)
  , n_cur_msg_(0)
  , p_cur_msg_(NULL) {
  evt_loop_.Start();

  n_port_ = port;

  memset(s_addr_, 0, 64);
  strncpy(s_addr_, server, 63);
}

CachedClient* CachedClient::Create(const char *server, unsigned short port) {
  if (NULL == server || port <= 0) {
    LOG(L_ERROR) << "param is error.";
    return NULL;
  }

  return (new CachedClient(server, port));
}

CachedClient::~CachedClient() {
  c_evt_recv_.Stop();
  c_evt_send_.Stop();

  evt_loop_.Stop();
}

int32 CachedClient::RunLoop(uint32 n_timeout) {
  int32 n_ret = 0;
  if (NULL == p_evt_loop_) {
    return -1;
  }
  n_ret = evt_loop_.RunLoop(n_timeout);
  if (n_ret_type_ != CACHED_INVALID) {
    return 1;
  }
  return 0;
}

bool CachedClient::CheckAndConnected() {
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

bool CachedClient::SaveCachedFile(const char *p_path, int n_path,
                                  const char *p_pata, int n_data) {
  try {
    if (CheckAndConnected() == false) {
      return false;
    }

    // Write Requestion
    int32 n_ret = 0;
    CacheMessage c_head;
    EncCacheMsg(&c_head,
                CACHED_REPLACE,
                p_path,
                n_path,
                n_data);

    iovec iov[2];
    iov[0].iov_base = &c_head;
    iov[0].iov_len  = sizeof(c_head);
    iov[1].iov_base = (void*)p_pata;
    iov[1].iov_len = (size_t)n_data;

    n_ret = AsyncWrite(iov, 2, 0);
    if (n_ret < 0) {
      LOG(L_ERROR) << "async write failed";
      return false;
    }

    n_ret = RunLoop(DEF_TIMEOUT_MSEC);
    if (n_ret == 0) {
      LOG(L_ERROR) << "save time out";
      return false;
    }
    return (n_ret_type_ == CACHED_SUCCEED);
  } catch (std::exception &e) {
    LOG(L_ERROR) << e.what();
  }
  return false;
}

bool CachedClient::GetCachedFile(const char *p_path, int n_path,
                                 Cached_GetFileCallback call_back,
                                 void *user_data) {
  try {
    if (n_path > (MAX_CACHED_PATH_SIZE-1)) {
      LOG(L_ERROR) << "key is length than "<<MAX_CACHED_PATH_SIZE;
      return false;
    }

    if (CheckAndConnected() == false) {
      return false;
    }

    // Write Requestion
    int32 n_ret = 0;
    CacheMessage c_head;
    EncCacheMsg(&c_head, CACHED_SELECT, p_path, n_path, 0);

    n_ret = AsyncWrite(&c_head, sizeof(c_head), 0);
    if (n_ret < 0) {
      LOG(L_ERROR) << "async write failed.";
      return false;
    }

    n_ret = RunLoop(DEF_TIMEOUT_MSEC);
    if (n_ret == 0) {
      LOG(L_ERROR) << "get cache time out";
      return false;
    }
    if (n_ret_type_ == CACHED_SUCCEED) {
      call_back(p_path, n_path,
                p_cur_msg_->data,
                n_cur_msg_ - sizeof(CacheMessage),
                user_data);
      return true;
    }
  } catch (std::exception &e) {
    LOG(L_ERROR) << e.what();
  }
  return false;
}

void CachedGetCallback(const char *s_path,
                       int         n_path,
                       const char *p_data,
                       int         n_data,
                       void       *p_usr_arg) {
  std::string *buffer = (std::string *)p_usr_arg;
  buffer->append(p_data, n_data);
}

bool CachedClient::GetCachedFile(const char *path,
                                 int path_size,
                                 std::string *buffer) {
  return GetCachedFile(path, path_size, CachedGetCallback, buffer);
}

bool CachedClient::DeleteCachedFile(const char *p_path, int n_path) {
  static std::string result;
  try {
    if (CheckAndConnected() == false) {
      return false;
    }

    // Write Requestion
    int32 n_ret = 0;
    CacheMessage c_head;
    EncCacheMsg(&c_head, CACHED_DELETE, p_path, n_path, 0);

    n_ret_type_ = CACHED_INVALID;
    n_ret = AsyncWrite(&c_head, sizeof(c_head), 0);
    if (n_ret < 0) {
      LOG(L_ERROR) << "async write failed " << p_path;
      return false;
    }

    n_ret = RunLoop(DEF_TIMEOUT_MSEC);
    if (n_ret == 0) {
      LOG(L_ERROR) << "delete time out";
      return false;
    }
    return (n_ret_type_ == CACHED_SUCCEED);
  } catch (std::exception &e) {
    LOG(L_ERROR) << e.what();
  }
  return false;
}

int32 CachedClient::HandleRecvPacket(vzconn::VSocket *p_cli,
                                     const uint8 *p_data,
                                     uint32 n_data,
                                     uint16 n_flag) {
  //LOG(L_WARNING) << "recv packet length " << n_data;
  if (!p_cli || !p_data || n_data == 0) {
    return -1;
  }

  if (n_data >= sizeof(CacheMessage)) {
    n_cur_msg_ = n_data;
    p_cur_msg_ = DecCacheMsg(p_data, n_data);
    if (p_cur_msg_->id != get_msg_id()) {
      LOG(L_WARNING) << "id is not current message id.";
      return 0;
    }

    evt_loop_.LoopExit(0);
    n_ret_type_ = (uint32)p_cur_msg_->type;
  } else {
    n_ret_type_ = CACHED_FAILURE;
    LOG(L_ERROR) << "unkown the response.";
  }
  return 0;
}

int CachedClient::EncCacheMsg(CacheMessage   *p_msg,
                              uint8           n_type,
                              const char     *p_path,
                              uint8           n_path,
                              uint32          n_data) {
  memset(p_msg, 0, sizeof(CacheMessage));

  p_msg->type = n_type;
  p_msg->id = get_msg_id();

  n_cur_msg_id_ = p_msg->id;

  strncpy(p_msg->path, p_path, MAX_CACHED_PATH_SIZE);
  return sizeof(CacheMessage);
}

CacheMessage * CachedClient::DecCacheMsg(const uint8 *p_data, uint32 n_data) {
  CacheMessage* p_msg = (CacheMessage*)(p_data);

  return p_msg;
}

}
