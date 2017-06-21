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
  if (!p_cli || !p_data || n_data == 0) {
    return -1;
  }

  DpMessage dp_msg;
  CTcpClient::DecDpMsg(&dp_msg, p_data, n_data);

  CTcpClient *p_tcp = (CTcpClient*)p_cli;
  if (p_tcp->callback_) {
    p_tcp->callback_(&dp_msg, p_tcp->p_usr_arg_);
  }

  LOG(L_INFO) << "message seq "<<dp_msg.id <<"  "<<p_tcp->get_msg_req();
  // 接收到正确的包,停止接收
  if (1 == p_tcp->b_wait_recv_) {
    if (dp_msg.id == p_tcp->get_msg_req()) {
      p_tcp->n_resp_type_ = dp_msg.type;
      p_tcp->evt_loop_.LoopExit(NULL);
    }
  } else {
    p_tcp->evt_loop_.LoopExit(NULL);
  }

  if (n_flag == FLAG_GET_CLIENT_ID
      || dp_msg.type == TYPE_GET_SESSION_ID) {
    p_tcp->n_chn_id_ = (dp_msg.channel_id << 24);
  }
  return 0;
}

void CClientProcess::HandleClose(vzconn::VSocket *p_cli) {

}

//////////////////////////////////////////////////////////////////////////
CTcpClient::CTcpClient()
  : vzconn::CEvtTcpClient(&evt_loop_, &c_cli_proc_)
  , callback_(NULL)
  , p_usr_arg_(NULL)
  , n_chn_id_(0) 
  , n_msg_seq_(1) {
  n_recv_one_packet_  = 0;
  b_send_all_buffer_  = 0;
  b_wait_recv_        = 0;
  n_resp_type_        = TYPE_FAILURE;
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
  /*ret = c_evt_recv_.Start(GetSocket(), EVT_READ | EVT_PERSIST);
  if (0 != ret) {
  LOG(L_ERROR) << "set recv event failed." << error_no();
  return false;
  }*/

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

void CTcpClient::Reset(bool b_wait_recv/*=false*/) {
  callback_    = NULL;
  p_usr_arg_   = NULL;

  n_recv_one_packet_ = 0;
  b_send_all_buffer_ = 0;

  b_wait_recv_ = b_wait_recv ? 1 : 0;
  n_resp_type_ = TYPE_FAILURE;

  c_recv_data_.Clear();
  c_send_data_.Clear();

  c_evt_recv_.Stop();
  c_evt_send_.Stop();
}

int32 CTcpClient::RunLoop(uint32 n_timeout) {
  int32 n_ret = 0;
  n_ret = evt_loop_.RunLoop(n_timeout);
  return n_ret;
}

int32 CTcpClient::PollRunLoop(uint32 n_timeout) {
  int32 n_ret = 0;
  n_ret = c_evt_recv_.Start(GetSocket(), EVT_READ | EVT_PERSIST);
  if (0 != n_ret) {
    LOG(L_ERROR) << "set recv event failed." << error_no();
    return n_ret;
  }

  n_ret = evt_loop_.RunLoop(n_timeout);
  return n_ret;
}

void CTcpClient::SetCallback(DpClient_MessageCallback callback, void *p_usr_arg) {
  callback_ = callback;
  p_usr_arg_ = p_usr_arg;
}

int32 CTcpClient::ListenMessage(DpMessage   *dP_msg,
                                const char  *method_set[],
                                unsigned int set_size,
                                uint16       n_flag) {
  if (IsOpen()) {
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

    c_send_data_.WriteBytes((uint8*)dP_msg, sizeof(DpMessage));

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
      // 补空值
      c_send_data_.WriteBytes(s_revd, MAX_METHOD_SIZE-n_method);
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

int32 CTcpClient::OnRecv() {
  int32 n_ret = c_recv_data_.RecvData(this);
  if (n_ret > 0) {
    n_ret = c_recv_data_.ParseSplitData(this); // 通过回调反馈给用户层
    if (n_ret > 0) {
      n_recv_one_packet_++;
    }
  }

  if (n_ret < 0) {
    if (cli_hdl_ptr_) {
      cli_hdl_ptr_->HandleClose(this);
    }
  }
  return 0;
}

int32 CTcpClient::OnSend() {
  int32 n_ret = 0;
  n_ret = vzconn::CEvtTcpClient::OnSend();

  if (c_send_data_.UsedSize() <= 0) {
    b_send_all_buffer_ = 1;

    if (1 == b_wait_recv_) {
      n_ret = c_evt_recv_.Start(GetSocket(), EVT_READ | EVT_PERSIST);
      if (0 != n_ret) {
        LOG(L_ERROR) << "set recv event failed." << error_no();
        return 0;
      }
    } else {
      evt_loop_.LoopExit(0);
    }
  }
  return 0;
}

int CTcpClient::EncDpMsg(DpMessage      *p_msg, 
                         unsigned char   n_type, 
                         unsigned char   n_chn_id, 
                         const char     *method, 
                         unsigned int    n_msg_id,
                         int             data_size) {
  if (!p_msg) {
    return -1;
  }
  memset(p_msg, 0, sizeof(DpMessage));

  p_msg->type       = (unsigned char)n_type;
  p_msg->channel_id = (unsigned char)n_chn_id;
  p_msg->reserved   = (unsigned char)0;
  if (method != NULL) {
    p_msg->method_size = (unsigned char)strlen(method);
    strncpy(p_msg->method, method, 31);
  }
  p_msg->id = (vzconn::VZ_ORDER_BYTE == vzconn::ORDER_NETWORK)
    ? vzconn::HostToNetwork32(n_msg_id) : n_msg_id;
  if (data_size > MAX_DPMESSAGE_DATA_SIZE) {
    return -2;
  }
  p_msg->data_size = (vzconn::VZ_ORDER_BYTE == vzconn::ORDER_NETWORK)
    ? vzconn::HostToNetwork32(data_size) : data_size;
  return sizeof(DpMessage);
}

int CTcpClient::DecDpMsg(DpMessage *p_msg, const void *p_data, uint32 n_data) {
  if (!p_msg || !p_data) {
    return -1;
  }

  if (n_data >= sizeof(DpMessage)) {
    memcpy(p_msg, p_data, sizeof(DpMessage));
    p_msg->id = (vzconn::VZ_ORDER_BYTE == vzconn::ORDER_NETWORK)
      ? vzconn::NetworkToHost32(p_msg->id) : p_msg->id;
    p_msg->data_size = (vzconn::VZ_ORDER_BYTE == vzconn::ORDER_NETWORK)
      ? vzconn::NetworkToHost32(p_msg->data_size) : p_msg->data_size;
    return sizeof(DpMessage);
  }
  return -2;
}

