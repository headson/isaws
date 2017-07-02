/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "cdpclient.h"

#include "vzbase/helper/stdafx.h"
#include "dispatcher/base/pkghead.h"

CDpClient::CDpClient()
  : vzconn::CTcpClient(&evt_loop_, this)
  , callback_(NULL)
  , p_usr_arg_(NULL)
  , n_session_id_(-1)
  , n_message_id_(1)
  , n_ret_type_((uint32)TYPE_INVALID)
  , b_poll_enabel_(false)
  , n_recv_packet_(0) {
  evt_loop_.Start();
}

CDpClient* CDpClient::Create() {
  return (new CDpClient());
}

CDpClient::~CDpClient() {
  c_evt_recv_.Stop();
  c_evt_send_.Stop();

  evt_loop_.Stop();
}

int32 CDpClient::RunLoop(uint32 n_timeout) {
  int32 n_ret = 0;
  //for (uint32 i = 0; i < n_timeout/10; i++) {
  //  n_ret = evt_loop_.RunLoop(10);
  //  if (n_ret_type_ != (uint32)TYPE_INVALID) {
  //    return 1;
  //  }
  //}

  n_ret = evt_loop_.RunLoop(n_timeout);
  if (n_ret_type_ != (uint32)TYPE_INVALID) {
    return 1;
  }
  return 0;
}

int32 CDpClient::PollRunLoop(uint32 n_timeout) {
  int32 n_ret = 0;
  b_poll_enabel_ = 1;
  //for (uint32 i = 0; i < n_timeout / 10; i++) {
  //  n_ret = evt_loop_.RunLoop(10);
  //  if (n_recv_packet_ > 0) {
  //    return 1;
  //  }
  //}

  n_ret = evt_loop_.RunLoop(n_timeout);
  if (n_recv_packet_ > 0) {
    return 1;
  }
  return 0;
}

void CDpClient::Reset(DpClient_MessageCallback callback,
                      void *p_usr_arg) {
  callback_ = callback;
  p_usr_arg_ = p_usr_arg;

  // loop 没在运行,清空标记
  if (!evt_loop_.isRuning()) {
    n_ret_type_ = (uint32)TYPE_INVALID;
    n_recv_packet_ = 0;

    //c_recv_data_.Clear();
    //c_send_data_.Clear();

    //c_evt_recv_.Stop();
    c_evt_send_.Stop();
  }
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

  Reset(NULL, NULL);
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

  Reset(call_back, user_data);
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
                                  uint16 n_flag) {
  //LOG(L_WARNING) << "recv packet length " << n_data;
  if (!p_cli || !p_data || n_data == 0) {
    LOG(L_ERROR) << "param is NULL";
    return -1;
  }
  DpMessage *p_msg = CDpClient::DecDpMsg(p_data, n_data);
  if (p_msg == NULL) {
    return -2;
  }

  CDpClient *p_tcp = (CDpClient*)p_cli;
  //LOG(L_INFO) << "message seq "<<p_msg->id <<"  "<<p_tcp->get_msg_id();

  // 在回调中,避免使用同一个socket send数据,造成递归evt loop
  if (p_tcp->callback_) {
    p_tcp->callback_(p_msg, p_tcp->p_usr_arg_);
  }

  if (p_msg->id == p_tcp->get_msg_id()) {  // 接收到正确的包
    evt_loop_.LoopExit(0);

    p_tcp->n_ret_type_ = static_cast<uint32>(p_msg->type);
  }

  if (n_flag == FLAG_GET_CLIENT_ID
      || p_msg->type == TYPE_GET_SESSION_ID) {
    p_tcp->n_session_id_ = (p_msg->channel_id << 24);
    LOG(L_WARNING) << "get session id " << p_msg->channel_id;
  }

  p_tcp->n_recv_packet_++;  // 收包计数
  // 如果是轮询,接收到一个包就退出event的run_loop
  if (b_poll_enabel_ > 0) {
    evt_loop_.LoopExit(0);
    b_poll_enabel_ = 0;
  }
  return 0;
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
