/************************************************************************/
/* Author      : Sober.Peng 17-06-21
/* Description :
/************************************************************************/
#include "cdpclient.h"

#include "stdafx.h"
#include "dispatcher/base/pkghead.h"

int32 CClientProcess::HandleRecvPacket(vzconn::VSocket  *p_cli,
                                       const uint8      *p_data,
                                       uint32            n_data,
                                       uint16            n_flag) {
  //LOG(L_WARNING) << "recv packet length " << n_data;
  if (!p_cli || !p_data || n_data == 0) {
    return -1;
  }
  DpMessage *p_msg = CTcpClient::DecDpMsg(p_data, n_data);
  if (p_msg == NULL) {
    return -2;
  }

  CTcpClient *p_tcp = (CTcpClient*)p_cli;
  //LOG(L_INFO) << "message seq "<<p_msg->id <<"  "<<p_tcp->get_msg_id();

  // 在回调中,避免使用同一个socket send数据,造成递归evt loop
  if (p_tcp->callback_) {
    p_tcp->callback_(p_msg, p_tcp->p_usr_arg_);
  }

  if (p_msg->id == p_tcp->get_msg_id()) {  // 接收到正确的包
    p_tcp->n_resp_type_ = p_msg->type;     // evt_loop判断不为0时退出loop
    //LOG(L_INFO) << "message type "<<p_tcp->n_resp_type_;
  }

  if (n_flag == FLAG_GET_CLIENT_ID
      || p_msg->type == TYPE_GET_SESSION_ID) {
    p_tcp->n_session_id_ = (p_msg->channel_id << 24);
    LOG(L_WARNING) << "get session id "<< p_msg->channel_id;
  }

  p_tcp->n_recv_packet_++;  // 收包计数
  return 0;
}

void CClientProcess::HandleClose(vzconn::VSocket *p_cli) {

}

//////////////////////////////////////////////////////////////////////////
CTcpClient::CTcpClient()
  : vzconn::CEvtTcpClient(&evt_loop_, &c_cli_proc_)
  , callback_(NULL)
  , p_usr_arg_(NULL)
  , n_session_id_(-1)
  , n_message_id_(1) {
  n_resp_type_ = 0;
  evt_loop_.Start();
}

CTcpClient* CTcpClient::Create() {
  return (new CTcpClient());
}

CTcpClient::~CTcpClient() {
  c_evt_recv_.Stop();
  c_evt_send_.Stop();

  evt_loop_.Stop();
}

bool CTcpClient::Open(SOCKET s, bool b_block /*= false*/) {
  if (NULL == p_evt_loop_) {
    LOG(L_ERROR) << "event loop is NULL.";
    return false;
  }
  if (INVALID_SOCKET == s) {
    LOG(L_ERROR) << "param is error.";
    return false;
  }

  SetSocket(s);

  //设置异步模式
  if (false == b_block) {
    set_socket_nonblocking(GetSocket());
  } else {
    set_socket_blocking(GetSocket());
  }

  int32 ret = 0;
  // 关联SOCKET的READ事件
  c_evt_recv_.Init(p_evt_loop_, EvtRecv, this);
  ret = c_evt_recv_.Start(GetSocket(), EVT_READ | EVT_PERSIST);
  if (0 != ret) {
    LOG(L_ERROR) << "set recv event failed." << error_no();
    return false;
  }

  // 关联SOCKET的SEND事件
  c_evt_send_.Init(p_evt_loop_, EvtSend, this);
  /*ret = c_evt_send_.Start(GetSocket(), EVT_WRITE);
  if (0 != ret) {
  LOG(L_ERROR) << "set send event failed." << error_no();
  return ret;
  }*/
  evt_loop_.Start();
  return true;
}

void CTcpClient::Reset(DpClient_MessageCallback callback,
                       void *p_usr_arg) {
  callback_ = callback;
  p_usr_arg_ = p_usr_arg;

  // loop 没在运行,清空标记
  if (!evt_loop_.isRuning()) {
    n_resp_type_   = 0;
    n_recv_packet_ = 0;

    //c_recv_data_.Clear();
    //c_send_data_.Clear();

    //c_evt_recv_.Stop();
    c_evt_send_.Stop();
  }
}

int32 CTcpClient::RunLoop(uint32 n_timeout) {
  int32 n_ret = 0;
  for (uint32 i = 0; i < n_timeout/10; i++) {
    n_ret = evt_loop_.RunLoop(10);
    if (get_resp_type() != 0) {
      break;
    }
  }
  return n_ret;
}

int32 CTcpClient::PollRunLoop(uint32 n_timeout) {
  int32 n_ret = 0;
  for (uint32 i = 0; i < n_timeout / 10; i++) {
    n_ret = evt_loop_.RunLoop(10);
    if (n_recv_packet_ > 0) {
      break;
    }
  }
  return n_ret;
}

int32 CTcpClient::ListenMessage(uint8        e_type,
                                const char  *method_set[],
                                unsigned int set_size,
                                uint16       n_flag) {
  Reset(NULL, NULL);
  if (isOpen()) {
    // 需写数据大小
    uint32 n_data = sizeof(DpMessage);
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
      n_data += MAX_METHOD_SIZE;
    }

    // 数据不够,分配大一点的空间
    int32 n_head = cli_hdl_ptr_->NetHeadSize();
    if ((n_head + n_data) > c_send_data_.FreeSize()) {
      c_send_data_.Recycle();
      if ((n_head + n_data) > c_send_data_.FreeSize()) {
        c_send_data_.ReallocBuffer((n_head + n_data));
      }
    }
    if ((n_head + n_data) > c_send_data_.FreeSize()) {
      return -1;
    }

    // 包头
    int32 n_head_size = cli_hdl_ptr_->NetHeadPacket(c_send_data_.GetWritePtr(),
                        c_send_data_.FreeSize(),
                        n_data,
                        n_flag);
    c_send_data_.MoveWritePtr(n_head_size);

    n_cur_msg_id_ = new_msg_id();
    // dp message
    DpMessage c_dp_msg;
    int32 n_dp_msg = CTcpClient::EncDpMsg(&c_dp_msg,
                                          e_type,
                                          get_session_id(),
                                          NULL,
                                          n_cur_msg_id_,
                                          n_data);
    c_send_data_.WriteBytes((uint8*)&c_dp_msg, sizeof(DpMessage));
    /*memcpy(c_send_data_.GetWritePtr(), &c_dp_msg, sizeof(DpMessage));
    c_send_data_.MoveWritePtr(sizeof(DpMessage));*/

    // body
    uint8 s_revd[32] = {0};
    for (uint32 i = 0; i < set_size; i++) {
      if (method_set[i] == NULL) {
        LOG(L_WARNING) << "method is null";
        continue;
      }
      if (method_set[i][0] == '\0') {
        LOG(L_WARNING) << "method is zero";
        continue;
      }

      uint32 n_method = strlen(method_set[i]) + 1;
      if (n_method >= MAX_METHOD_SIZE) {
        LOG(L_WARNING) << "method size is large than "<<MAX_METHOD_SIZE;
        continue;
      }
      // 写method
      c_send_data_.WriteBytes((uint8*)method_set[i], n_method);
      /*memcpy(c_send_data_.GetWritePtr(), method_set[i], n_method);
      c_send_data_.MoveWritePtr(n_method);*/

      // 补空值
      c_send_data_.WriteBytes(s_revd, MAX_METHOD_SIZE-n_method);
      /*memcpy(c_send_data_.GetWritePtr(), method_set[i], n_method);
      c_send_data_.MoveWritePtr(n_method);*/
    }

    // 打开事件
    if (c_send_data_.UsedSize() > 0) {
      int32 n_ret = c_evt_send_.Start(GetSocket(), EVT_WRITE | EVT_PERSIST);
      if (n_ret == 0) {
        c_evt_send_.ActiceEvent();
      }
    }
    return (n_head + n_data);
  }
  return -1;
}

int32 CTcpClient::SendMessage(unsigned char             n_type,
                              const char               *p_method,
                              unsigned int              n_msg_id,
                              const char               *p_data,
                              int                       n_data,
                              DpClient_MessageCallback  call_back,
                              void                     *user_data) {
  n_cur_msg_id_ = n_msg_id;
  // dp head
  DpMessage c_dp_msg;
  int32 n_dp_msg = CTcpClient::EncDpMsg(&c_dp_msg,
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
  iov[0].iov_len = n_dp_msg;
  iov[1].iov_base = (void*)p_data;
  iov[1].iov_len = n_data;

  Reset(call_back, user_data);
  int32 n_ret = AsyncWrite(iov, 2, FLAG_DISPATCHER_MESSAGE);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "async write failed " << n_dp_msg + n_data;
    return n_ret;
  }
  return n_ret;
}

int32 CTcpClient::OnRecv() {
  CEvtTcpClient::OnRecv();
  return 0;
}

int32 CTcpClient::OnSend() {
  CEvtTcpClient::OnSend();
  return 0;
}

int CTcpClient::EncDpMsg(DpMessage      *p_msg,
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

int CTcpClient::DecDpMsg(DpMessage *p_msg, const void *p_data, uint32 n_data) {
  if (!p_msg) {
    return -1;
  }
  if (!p_data || n_data < sizeof(DpMessage)) {
    return -2;
  }

  memcpy(p_msg, p_data, sizeof(DpMessage));
  p_msg->id = (vzconn::VZ_ORDER_BYTE == vzconn::ORDER_NETWORK)
              ? vzconn::NetworkToHost32(p_msg->id) : p_msg->id;
  p_msg->data_size = (vzconn::VZ_ORDER_BYTE == vzconn::ORDER_NETWORK)
                     ? vzconn::NetworkToHost32(p_msg->data_size) : p_msg->data_size;
  return sizeof(DpMessage);
}

DpMessage *CTcpClient::DecDpMsg(const void *p_data, uint32 n_data) {
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
