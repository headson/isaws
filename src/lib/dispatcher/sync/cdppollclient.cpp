/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "cdppollclient.h"

#include "vzbase/helper/stdafx.h"
#include "dispatcher/base/pkghead.h"

CDpPollClient::CDpPollClient(const char *server, unsigned short port,
                             DpClient_MessageCallback   p_msg_cb,
                             void                      *p_msg_usr_arg,
                             DpClient_PollStateCallback p_state_cb,
                             void                      *p_state_usr_arg,
                             vzconn::EVT_LOOP          *p_evt_loop)
  : CDpClient(server, port, p_evt_loop)
  , c_evt_timer_()
  , had_reg_msg_(0)
  , p_poll_msg_cb_(p_msg_cb)
  , p_poll_msg_usr_arg_(p_msg_usr_arg)
  , p_state_cb_(p_state_cb)
  , p_state_usr_arg_(p_state_usr_arg) {
  c_evt_timer_.Init(p_evt_loop_, evt_timer_cb, this);
  c_evt_timer_.Start(DEF_TIMEOUT_MSEC, DEF_TIMEOUT_MSEC);
}

CDpPollClient* CDpPollClient::Create(const char *server, unsigned short port,
                                     DpClient_MessageCallback   p_msg_cb,
                                     void                      *p_msg_usr_arg,
                                     DpClient_PollStateCallback p_state_cb,
                                     void                      *p_state_usr_arg,
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

  return (new CDpPollClient(server, port,
                            p_msg_cb, p_msg_usr_arg,
                            p_state_cb, p_state_usr_arg, p_evt_loop));
}

CDpPollClient::~CDpPollClient() {
  c_evt_timer_.Stop();
  c_evt_recv_.Stop();
  c_evt_send_.Stop();

  /*if (p_evt_loop_) {
    p_evt_loop_->Stop();
    p_evt_loop_ = NULL;
    }*/
}

int32 CDpPollClient::ListenMessage(uint8        e_type,
                                   const char  *method_set[],
                                   unsigned int set_size,
                                   uint16       n_flag) {
  // 组method包
  uint32 n_data = 0;
  char   s_data[MAX_METHOD_COUNT*MAX_METHOD_SIZE] = { 0 };
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
    memcpy(s_data + n_data, method_set[i], n_method);
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
  iov[0].iov_len = sizeof(c_dp_msg);
  iov[1].iov_base = (void*)s_data;
  iov[1].iov_len = n_data;

  n_ret_type_ = (uint32)TYPE_INVALID;
  int32 n_ret = AsyncWrite(iov, 2, FLAG_DISPATCHER_MESSAGE);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "async write failed " << n_dp_msg + n_data;
    return n_ret;
  }
  RunLoop(DEF_TIMEOUT_MSEC);
  return n_ret;
}

bool CDpPollClient::CheckAndConnected() {
  if (isClose()) {
    Close();

    n_session_id_ = -1;
    n_ret_type_ = (uint32)TYPE_INVALID;
    vzconn::CInetAddr c_remote_addr(dp_addr_, dp_port_);
    bool b_ret = Connect(&c_remote_addr, false, true, DEF_TIMEOUT_MSEC);
    if (b_ret == false) {
      LOG(L_ERROR) << "can't connect kvdb server.";
      return false;
    }

    // 获取session id
    SendMessage(TYPE_GET_SESSION_ID,
                "GET_SEESION_ID",
                new_msg_id(),
                "body_data",
                strlen("body_data"));
    RunLoop(DEF_TIMEOUT_MSEC);
  }
  return true;
}

int32 CDpPollClient::HandleRecvPacket(vzconn::VSocket *p_cli,
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

  // 注:理论上poll只会收到TYPE_MESSAGE\TYPE_REQUEST 消息
  // TYPE_GET_SESSION_ID\TYPE_ADD_MESSAGE的回执结果
  if (p_cur_dp_msg_->type == TYPE_MESSAGE ||
      p_cur_dp_msg_->type == TYPE_REQUEST) {  // 获取ID不回调
    // 回调处理消息\请求
    if (p_poll_msg_cb_) {
      p_poll_msg_cb_(this, p_cur_dp_msg_, p_poll_msg_usr_arg_);
    }
  } else if (n_flag == FLAG_GET_CLIENT_ID ||
             p_cur_dp_msg_->type == TYPE_GET_SESSION_ID) {
    // 获取ID
    n_session_id_ = (p_cur_dp_msg_->channel_id << 24);
    LOG(L_WARNING) << "get session id " << p_cur_dp_msg_->channel_id;
  }
  // 注册消息成功;
  if (p_cur_dp_msg_->reply_type == TYPE_ADD_MESSAGE) {
    had_reg_msg_ = 1;
  }

  // 如果是轮询,接收到一个包就退出event的run_loop
  if (NULL != p_evt_loop_) {
    p_evt_loop_->LoopExit(0);
  }
  return 0;
}

int32 CDpPollClient::OnEvtTimer() {
  bool b_close = isClose();
  if (b_close || had_reg_msg_ == 0) {
    n_session_id_ = -1;
    had_reg_msg_  = 0;
    if (p_state_cb_) {
      p_state_cb_(this, DP_CLIENT_DISCONNECT, p_state_usr_arg_);
    }
  }
  return 0;
}
