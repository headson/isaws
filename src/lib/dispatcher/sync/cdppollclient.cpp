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
  : had_reg_msg_(0)
  , p_poll_msg_cb_(p_msg_cb)
  , p_poll_msg_usr_arg_(p_msg_usr_arg)
  , p_state_cb_(p_state_cb)
  , p_state_usr_arg_(p_state_usr_arg)
  , CDpClient(server, port, p_evt_loop) {
  c_evt_timer_.Init(p_evt_loop_, evt_timer_cb, this);
  c_evt_timer_.Start(DEF_TIMEOUT_MSEC, DEF_TIMEOUT_MSEC);
}

CDpPollClient* CDpPollClient::Create(const char *server, unsigned short port,
                                     DpClient_MessageCallback   p_msg_cb,
                                     void                      *p_msg_usr_arg,
                                     DpClient_PollStateCallback p_state_cb,
                                     void                      *p_state_usr_arg,
                                     vzconn::EVT_LOOP          *p_evt_loop) {
  if (!p_msg_cb || !p_state_cb) {
    LOG(L_ERROR) << "param is null.";
    return NULL;
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

  if (p_evt_loop_) {
    p_evt_loop_ = NULL;
  }
}

int CDpPollClient::ListenMessage(unsigned char     e_type,
                                 const char       *p_method[],
                                 unsigned int      n_method_cnt,
                                 unsigned short    n_flag) {
  // 组method包
  unsigned int n_data = 0;
  char   s_data[MAX_METHOD_COUNT*MAX_METHOD_SIZE] = {0};
  for (unsigned int i = 0; i < n_method_cnt; i++) {
    if (p_method[i] == NULL) {
      continue;
    }
    if (p_method[i][0] == '\0') {
      continue;
    }
    unsigned int n_method = strlen(p_method[i]) + 1;
    if (n_method >= MAX_METHOD_SIZE) {
      continue;
    }
    memcpy(s_data+n_data, p_method[i], n_method);
    n_data += MAX_METHOD_SIZE;
  }

  //
  n_cur_msg_id_ = new_msg_id();
  // dp message
  DpMessage c_dp_msg;
  int n_dp_msg = CDpClient::EncDpMsg(&c_dp_msg,
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

  Reset();
  int n_ret = AsyncWrite(iov, 2, FLAG_DISPATCHER_MESSAGE);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "async write failed " << n_dp_msg + n_data;
    return n_ret;
  }
  return n_ret;
}

int CDpPollClient::RunLoop(unsigned int n_timeout) {
  int n_ret = 0;
  if (NULL == p_evt_loop_) {
    return -1;
  }

  n_ret = p_evt_loop_->RunLoop(n_timeout);
  return 0;
}

int CDpPollClient::HandleRecvPacket(vzconn::VSocket *p_cli,
                                    const unsigned char     *p_data,
                                    unsigned int           n_data,
                                    unsigned short           n_flag) {
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

  // 在回调中,避免使用同一个socket send数据,造成递归evt loop
  if (n_flag != FLAG_GET_CLIENT_ID
      && p_cur_dp_msg_->type == TYPE_GET_SESSION_ID) {  // 获取ID不回调
    if (p_poll_msg_cb_) {
      p_poll_msg_cb_(this, p_cur_dp_msg_, p_poll_msg_usr_arg_);
    }
  } else if (n_flag == FLAG_GET_CLIENT_ID
             || p_cur_dp_msg_->type == TYPE_GET_SESSION_ID) {  // 获取ID不回调
    n_session_id_ = (p_cur_dp_msg_->channel_id << 24);
    LOG(L_WARNING) << "get session id " << p_cur_dp_msg_->channel_id;
  }

  // 注册消息成功
  if (p_cur_dp_msg_->reply_type == TYPE_ADD_MESSAGE) {
    SetResMsgFlag(1);
  }
  // 如果是轮询,接收到一个包就退出event的run_loop
  if (NULL != p_evt_loop_) {
    p_evt_loop_->LoopExit(0);
  }
  return 0;
}

int CDpPollClient::OnEvtTimer() {
  if (isClose() || had_reg_msg_ == 0) {
    n_session_id_ = -1;
    had_reg_msg_  = 0;
    if (p_state_cb_) {
      p_state_cb_(this, DP_CLIENT_DISCONNECT, p_state_usr_arg_);
    }
  }
  return 0;
}
