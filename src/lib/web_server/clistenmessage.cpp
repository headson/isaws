/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "vzbase/helper/stdafx.h"

#include "json/json.h"

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

bool CListenMessage::Start(const unsigned char *s_dp_ip,
                           unsigned short       n_dp_port,
                           const unsigned char *s_http_port,
                           const unsigned char *s_http_path) {

  bool b_ret = false;
  b_ret = c_web_srv_.Start(s_http_path, s_http_port);
  if (b_ret == false) {
    LOG(L_ERROR) << "start web server failed.";
    exit(EXIT_FAILURE);
  }

  // dp client poll
  int32 n_ret = VZNETDP_SUCCEED;
  DpClient_Init((char*)s_dp_ip, n_dp_port);
  n_ret = DpClient_Start(0);
  if (n_ret == VZNETDP_FAILURE) {
    LOG(L_ERROR) << "dp client start failed.";
    return false;
  }

  if (p_dp_cli_ == NULL) {
    vzconn::EventService *p_evt_srv =
      vzbase::Thread::Current()->socketserver()->GetEvtService();

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
  }
  return (n_ret == VZNETDP_SUCCEED);
}

void CListenMessage::Stop() {
  if (p_dp_cli_) {
    DpClient_ReleasePollHandle(p_dp_cli_);
    p_dp_cli_ = NULL;
  }
  DpClient_Stop();

  c_web_srv_.Stop();

  vzbase::Thread::Current()->Release();
}

void CListenMessage::RunLoop() {
  vzbase::Thread::Current()->Run();
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
