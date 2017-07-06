/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "cdpclient.h"

#include "vzbase/helper/stdafx.h"
#include "dispatcher/base/pkghead.h"

CDpClient::CDpClient(const char *server, unsigned short port,
                     vzconn::EVT_LOOP          *p_evt_loop)
  : p_evt_loop_(p_evt_loop)
  , vzconn::CTcpClient(p_evt_loop, this)
  , n_session_id_(-1)
  , n_message_id_(1)
  , n_ret_type_((uint32)TYPE_INVALID) {
  dp_port_ = port;
  memset(dp_addr_, 0, 64);
  strncpy(dp_addr_, server, 63);
}

CDpClient* CDpClient::Create(const char *server, unsigned short port,
                             vzconn::EVT_LOOP          *p_evt_loop) {
  if (NULL == p_evt_loop) {
    p_evt_loop = new vzconn::EVT_LOOP();
    if (p_evt_loop != NULL) {
      if (p_evt_loop->Start() != 0) {
        delete p_evt_loop;
        p_evt_loop = NULL;
      }
    }
  }

  if (NULL == p_evt_loop) {
    LOG(L_ERROR) << "evt loop is null.";
    return NULL;
  }

  return (new CDpClient(server, port, p_evt_loop));
}

CDpClient::~CDpClient() {
  c_evt_recv_.Stop();
  c_evt_send_.Stop();

  if (p_evt_loop_) {
    p_evt_loop_->Stop();
    p_evt_loop_ = NULL;
  }
}

int32 CDpClient::RunLoop(uint32 n_timeout) {
  int32 n_ret = 0;
  if (NULL == p_evt_loop_) {
    return -1;
  }
  n_ret = p_evt_loop_->RunLoop(n_timeout);
  if (n_ret_type_ != (uint32)TYPE_INVALID) {
    return 1;
  }
  return 0;
}

bool CDpClient::CheckAndConnected() {
  if (isClose()) {
    Close();

    vzconn::CInetAddr c_remote_addr(dp_addr_, dp_port_);
    bool b_ret = Connect(&c_remote_addr, false, true, DEF_TIMEOUT_MSEC);
    if (b_ret == false) {
      LOG(L_ERROR) << "can't connect kvdb server.";
      return b_ret;
    }
  }
  return true;
}

int32 CDpClient::ListenMessage(uint8        e_type,
                               const char  *method_set[],
                               unsigned int set_size,
                               uint16       n_flag) {
  // 组method包
  uint32 n_data = 0;
  char   s_data[MAX_METHOD_COUNT*MAX_METHOD_SIZE] = {0};
  for (uint32 i = 0; i < set_size; i++) {
    if (method_set[i] == NULL) {
      continue;
    }
    if (method_set[i][0] == '\0') {
      continue;
    }
    uint32 n_method = strlen(method_set[i]) + 1;
    if (n_method >= MAX_METHOD_SIZE) {
      continue;
    }
    memcpy(s_data+n_data, method_set[i], n_method);
    n_data += MAX_METHOD_SIZE;
  }

  //
  n_cur_msg_id_ = new_msg_id();
  // dp message
  DpMessage c_dp_msg;
  int32 n_dp_msg = CDpClient::EncDpMsg(&c_dp_msg,
                                       e_type,
                                       get_session_id(),
                                       NULL,
                                       n_cur_msg_id_,
                                       n_data);
  if (n_dp_msg < 0) {
    LOG(L_ERROR) << "create dp msg head failed." << n_dp_msg;
    return VZNETDP_FAILURE;
  }

  // body
  iovec iov[2];
  iov[0].iov_base = &c_dp_msg;
  iov[0].iov_len  = sizeof(c_dp_msg);
  iov[1].iov_base = (void*)s_data;
  iov[1].iov_len  = n_data;

  n_ret_type_ = (uint32)TYPE_INVALID;
  int32 n_ret = AsyncWrite(iov, 2, FLAG_DISPATCHER_MESSAGE);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "async write failed " << n_dp_msg + n_data;
    return n_ret;
  }
  return n_ret;
}

int32 CDpClient::SendMessage(unsigned char             n_type,
                             const char               *p_method,
                             unsigned int              n_msg_id,
                             const char               *p_data,
                             int                       n_data,
                             DpClient_MessageCallback  call_back,
                             void                     *user_data) {
  n_cur_msg_id_ = n_msg_id;
  // dp head
  DpMessage c_dp_msg;
  int32 n_dp_msg = CDpClient::EncDpMsg(&c_dp_msg,
                                       n_type,
                                       get_session_id(),
                                       p_method,
                                       n_cur_msg_id_,
                                       n_data);
  if (n_dp_msg < 0) {
    LOG(L_ERROR) << "create dp msg head failed." << n_dp_msg;
    return VZNETDP_FAILURE;
  }
  // LOG(L_WARNING) << "send message id "<<n_cur_msg_id_;

  // body
  iovec iov[2];
  iov[0].iov_base = &c_dp_msg;
  iov[0].iov_len  = n_dp_msg;
  iov[1].iov_base = (void*)p_data;
  iov[1].iov_len  = n_data;

  n_ret_type_ = (uint32)TYPE_INVALID;
  int32 n_ret = AsyncWrite(iov, 2, FLAG_DISPATCHER_MESSAGE);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "async write failed " << n_dp_msg + n_data;
    return n_ret;
  }
  return n_ret;
}

int32 CDpClient::HandleRecvPacket(vzconn::VSocket *p_cli,
                                  const uint8     *p_data,
                                  uint32           n_data,
                                  uint16           n_flag) {
  //LOG(L_WARNING) << "recv packet length " << n_data;
  if (!p_cli || !p_data || n_data == 0) {
    LOG(L_ERROR) << "param is NULL";
    return -1;
  }
  p_cur_dp_msg_ = CDpClient::DecDpMsg(p_data, n_data);
  if (p_cur_dp_msg_ == NULL) {
    return -2;
  }

  LOG(L_INFO) << "message seq "<<p_cur_dp_msg_->method <<"  "<<get_msg_id();
  
  if (p_cur_dp_msg_->id == get_msg_id()) {  // 接收到正确的包
    if (NULL != p_evt_loop_) {
      p_evt_loop_->LoopExit(0);
    }
    n_ret_type_ = static_cast<uint32>(p_cur_dp_msg_->type);
  }

  if (n_flag == FLAG_GET_CLIENT_ID
      || p_cur_dp_msg_->type == TYPE_GET_SESSION_ID) {
    n_session_id_ = (p_cur_dp_msg_->channel_id << 24);
    LOG(L_WARNING) << "get session id " << p_cur_dp_msg_->channel_id;
  }
  return 0;
}

void CDpClient::HandleClose(vzconn::VSocket *p_cli) {
}

int CDpClient::EncDpMsg(DpMessage      *p_msg,
                        unsigned char   n_type,
                        unsigned char   n_session_id,
                        const char     *method,
                        unsigned int    n_message_id,
                        int             data_size) {
  if (!p_msg) {
    return -1;
  }
  memset(p_msg, 0, sizeof(DpMessage));

  p_msg->type       = (unsigned char)n_type;
  p_msg->channel_id = (unsigned char)n_session_id;
  p_msg->reserved   = (unsigned char)0;
  if (method != NULL) {
    p_msg->method_size = (unsigned char)strlen(method);
    strncpy(p_msg->method, method, 31);
  }
  p_msg->id = (vzconn::VZ_ORDER_BYTE == vzconn::ORDER_NETWORK)
              ? vzconn::HostToNetwork32(n_message_id) : n_message_id;
  if (data_size > MAX_DPMESSAGE_DATA_SIZE) {
    return -2;
  }
  p_msg->data_size = (vzconn::VZ_ORDER_BYTE == vzconn::ORDER_NETWORK)
                     ? vzconn::HostToNetwork32(data_size) : data_size;
  return sizeof(DpMessage);
}

DpMessage *CDpClient::DecDpMsg(const void *p_data, uint32 n_data) {
  if (!p_data || n_data < sizeof(DpMessage)) {
    return NULL;
  }

  DpMessage *p_msg = (DpMessage*)p_data;
  p_msg->id = (vzconn::VZ_ORDER_BYTE == vzconn::ORDER_NETWORK)
              ? vzconn::NetworkToHost32(p_msg->id) : p_msg->id;
  p_msg->data_size = (vzconn::VZ_ORDER_BYTE == vzconn::ORDER_NETWORK)
                     ? vzconn::NetworkToHost32(p_msg->data_size) : p_msg->data_size;
  return p_msg;
}
