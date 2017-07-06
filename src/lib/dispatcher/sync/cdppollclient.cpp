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
  CDpClient::~CDpClient();
  c_evt_timer_.Stop();
}

int32 CDpPollClient::PollRunLoop(uint32 n_timeout) {
  int32 n_ret = 0;
  if (NULL == p_evt_loop_) {
    return -1;
  }

  n_ret = p_evt_loop_->RunLoop(n_timeout);
  return 0;
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

  // 在回调中,避免使用同一个socket send数据,造成递归evt loop
  if (p_poll_msg_cb_) {
    p_poll_msg_cb_(this, p_cur_dp_msg_, p_poll_msg_usr_arg_);
  }

  // 注册消息成功
  if (p_cur_dp_msg_->reply_type == TYPE_ADD_MESSAGE) {
    SetResMsgFlag(1);
  }

  // 获取session id
  if (n_flag == FLAG_GET_CLIENT_ID
      || p_cur_dp_msg_->type == TYPE_GET_SESSION_ID) {
    n_session_id_ = (p_cur_dp_msg_->channel_id << 24);
    LOG(L_WARNING) << "get session id " << p_cur_dp_msg_->channel_id;
  }

  // 如果是轮询,接收到一个包就退出event的run_loop
  if (NULL != p_evt_loop_) {
    p_evt_loop_->LoopExit(0);
  }
  return 0;
}

int32 CDpPollClient::OnEvtTimer() {
  if (isClose() || had_reg_msg_ == 0) {
    n_session_id_ = -1;
    had_reg_msg_  = 0;
    if (p_state_cb_) {
      p_state_cb_(this, DP_CLIENT_DISCONNECT, p_state_usr_arg_);
    }
  }
  return 0;
}
