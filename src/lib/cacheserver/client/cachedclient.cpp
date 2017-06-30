/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "cachedclient.h"

#include "vzbase/helper/stdafx.h"

namespace cached {

CachedClient::CachedClient()
  : vzconn::CTcpClient(&evt_loop_, this)
  , p_callback_(NULL)
  , p_usr_arg_(NULL) {
  evt_loop_.Start();
}

CachedClient* CachedClient::Create() {
  return (new CachedClient());
}

CachedClient::~CachedClient() {
  c_evt_recv_.Stop();
  c_evt_send_.Stop();

  evt_loop_.Stop();
}

int32 CachedClient::RunLoop(uint32 n_timeout) {
  int32 n_ret = 0;
  for (uint32 i = 0; i < n_timeout / 10; i++) {
    n_ret = evt_loop_.RunLoop(10);
    if (n_ret_type_ != (uint32)-1) {
      return 1;
    }
  }
  return 0;
}

void CachedClient::Reset(Cached_GetFileCallback  callback,
                         void                   *p_usr_arg) {
  n_ret_type_ = (uint32)-1;

  p_callback_ = callback;
  p_usr_arg_  = p_usr_arg;

  // loop 没在运行,清空标记
  if (!evt_loop_.isRuning()) {
    //c_recv_data_.Clear();
    //c_send_data_.Clear();

    //c_evt_recv_.Stop();
    c_evt_send_.Stop();
  }
}

bool CachedClient::SaveCachedFile(const char *p_path, int n_path,
                                  const char *p_pata, int n_data) {
  try {
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

    Reset(NULL, NULL);
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
    strncpy((char*)s_path_, p_path, n_path);
    s_path_[n_path] = '\0';
    n_path_ = n_path;

    // Write Requestion
    int32 n_ret = 0;
    CacheMessage c_head;
    EncCacheMsg(&c_head, CACHED_SELECT, p_path, n_path, 0);

    Reset(call_back, user_data);
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
    return (n_ret_type_ == CACHED_SUCCEED);
  } catch (std::exception &e) {
    LOG(L_ERROR) << e.what();
  }
  return false;
}

bool CachedClient::DeleteCachedFile(const char *p_path, int n_path) {
  static std::string result;
  try {
    // Write Requestion
    int32 n_ret = 0;
    CacheMessage c_head;
    EncCacheMsg(&c_head, CACHED_DELETE, p_path, n_path, 0);

    Reset(NULL, NULL);
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
    
    CacheMessage *p_msg = DecCacheMsg(p_data, n_data);
    if (p_msg->id != get_msg_id()) {
      LOG(L_WARNING) << "id is not current message id.";
      return 0;
    }

    evt_loop_.LoopExit(0);
    n_ret_type_ = (uint32)p_msg->type;

    int32 n_length = n_data - sizeof(CacheMessage);
    if (p_callback_) {
      p_callback_((char*)s_path_, n_path_,
                  (char*)p_msg->data, n_length, p_usr_arg_);
    }
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
