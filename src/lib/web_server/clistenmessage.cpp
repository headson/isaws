/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "vzbase/helper/stdafx.h"

#include "json/json.h"

namespace web {

static const unsigned int METHOD_SET_SIZE = 3;
static const char  *METHOD_SET[] = {
  "TEST_MSG_TYPE_01",
  "TEST_MSG_TYPE_02",
  "TEST_MSG_TYPE_03",
};

CListenMessage::CListenMessage()
  : p_dp_cli_(NULL)
  , c_web_srv_() {
}

CListenMessage::~CListenMessage() {
  Stop();
}

CListenMessage *CListenMessage::Instance() {
  VZBASE_DEFINE_STATIC_LOCAL(CListenMessage, listen_message, ());
  return &listen_message;
}

bool CListenMessage::Start(unsigned short  n_http_port,
                           const char     *s_http_path) {
  bool b_ret = false;
  char s_port[9] = {0};
  snprintf(s_port, 8, "%d", n_http_port);
  b_ret = c_web_srv_.Start(s_http_path, s_port);
  if (b_ret == false) {
    LOG(L_ERROR) << "start web server failed.";
    exit(EXIT_FAILURE);
  }
  p_main_thread_ = vzbase::Thread::Current();

  if (p_dp_cli_ == NULL) {
    vzconn::EventService *p_evt_srv =
      p_main_thread_->socketserver()->GetEvtService();

    p_dp_cli_ = DpClient_CreatePollHandle(dpcli_poll_msg_cb, this,
                                          dpcli_poll_state_cb, this,
                                          p_evt_srv);
    if (p_dp_cli_ == NULL) {
      LOG(L_ERROR) << "dp client create poll handle failed.";

      DpClient_ReleasePollHandle(p_dp_cli_);
      p_dp_cli_ = NULL;
      return false;
    }

    DpClient_HdlAddListenMessage(p_dp_cli_, METHOD_SET, METHOD_SET_SIZE);
    return VZNETDP_SUCCEED;
  }
  return VZNETDP_FAILURE;
}

void CListenMessage::Stop() {
  if (p_dp_cli_) {
    DpClient_ReleasePollHandle(p_dp_cli_);
    p_dp_cli_ = NULL;
  }
  DpClient_Stop();

  c_web_srv_.Stop();

  if (p_main_thread_) {
    p_main_thread_->Release();
    p_main_thread_ = NULL;
  }
}

void CListenMessage::RunLoop() {
  p_main_thread_->Run();
}

vzbase::Thread *CListenMessage::MainThread() {
  return p_main_thread_;
}
void CListenMessage::dpcli_poll_msg_cb(DPPollHandle p_hdl, const DpMessage *dmp, void* p_usr_arg) {
  if (p_usr_arg) {
    ((CListenMessage*)p_usr_arg)->OnDpCliMsg(p_hdl, dmp);
    return;
  }
  LOG(L_ERROR) << "param is error.";
}

void CListenMessage::OnDpCliMsg(DPPollHandle p_hdl, const DpMessage *dmp) {

}

void CListenMessage::dpcli_poll_state_cb(DPPollHandle p_hdl, unsigned int n_state, void* p_usr_arg) {
  if (p_usr_arg) {
    ((CListenMessage*)p_usr_arg)->OnDpCliState(p_hdl, n_state);
  }
}

void CListenMessage::OnDpCliState(DPPollHandle p_hdl, unsigned int n_state) {
  if (n_state == DP_CLIENT_DISCONNECT) {
    int32 n_ret = DpClient_HdlReConnect(p_hdl);
    if (n_ret == VZNETDP_SUCCEED) {
      DpClient_HdlAddListenMessage(p_dp_cli_, METHOD_SET, METHOD_SET_SIZE);
    }
  }
}

void CListenMessage::OnMessage(vzbase::Message* p_msg) {
  //if (p_msg->message_id == THREAD_MSG_SET_DEV_ADDR) {
  /*vzbase::TypedMessageData<std::string>::Ptr msg_ptr =
    boost::static_pointer_cast<vzbase::TypedMessageData< std::string >> (p_msg->pdata);*/

  //Restart("127.0.0.1", 5291);
  //vzbase::Thread::Current()->PostDelayed(2*1000, this, THREAD_MSG_SET_DEV_ADDR);
  //}
}
}  // namespace web
