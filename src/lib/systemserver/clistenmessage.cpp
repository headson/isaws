/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "vzbase/helper/stdafx.h"
#include "vzbase/helper/vmessage.h"

#include "json/json.h"

static const char  *K_METHOD_SET[] = {
  DP_SYS_CONFIG,
};

CListenMessage::CListenMessage()
  : p_dp_cli_(NULL) {
}

CListenMessage::~CListenMessage() {
}

CListenMessage *CListenMessage::Instance() {
  VZBASE_DEFINE_STATIC_LOCAL(CListenMessage, listen_message, ());
  return &listen_message;
}

bool CListenMessage::Start(const unsigned char *s_dp_ip, unsigned short n_dp_port) {
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

    unsigned int n_method_set = sizeof(K_METHOD_SET) / sizeof(char*);
    DpClient_HdlAddListenMessage(p_dp_cli_, K_METHOD_SET, n_method_set);
  }
  return (n_ret == VZNETDP_SUCCEED);
}

void CListenMessage::Stop() {
  if (p_dp_cli_) {
    DpClient_ReleasePollHandle(p_dp_cli_);
    p_dp_cli_ = NULL;
  }
  DpClient_Stop();

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
      unsigned int n_method_set = sizeof(K_METHOD_SET) / sizeof(char*);
      DpClient_HdlAddListenMessage(p_dp_cli_, K_METHOD_SET, n_method_set);
    }
  }
}
