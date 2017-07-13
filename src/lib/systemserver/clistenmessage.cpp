/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "vzbase/helper/stdafx.h"
#include "vzbase/helper/vmessage.h"

#include "json/json.h"

#include "vzbase/base/helper.h"
#include "vzbase/base/base64.h"

namespace sys {

static const char  *K_METHOD_SET[] = {
  DP_SYS_CONFIG,
};

CListenMessage::CListenMessage()
  : dp_cli_(NULL)
  , net_ctrl_()
  , thread_slow_(NULL) {
}

CListenMessage::~CListenMessage() {
  Stop();
}

CListenMessage *CListenMessage::Instance() {
  VZBASE_DEFINE_STATIC_LOCAL(CListenMessage, listen_message, ());
  return &listen_message;
}

bool CListenMessage::Start(const char *s_dp_ip, unsigned short n_dp_port) {
  bool b_ret = false;

  // slow thread
  thread_slow_ = new vzbase::Thread();
  if (NULL == thread_slow_) {
    LOG(L_ERROR) << "create thread failed.";
    return false;
  }
  b_ret = thread_slow_->Start();
  if (b_ret == false) {
    LOG(L_ERROR) << "slow thread start failed.";
    return false;
  }

  // net ctrl
  net_ctrl_ = CNetCtrl::Create(vzbase::Thread::Current());
  if (NULL == net_ctrl_) {
    LOG(L_ERROR) << "create net ctrl failed.";
    return false;
  }
  b_ret = net_ctrl_->Start();
  if (b_ret == false) {
    LOG(L_ERROR) << "net ctrl start failed.";
    return false;
  }

  // dp client
  DpClient_Init(s_dp_ip, n_dp_port);

  vzconn::EventService *p_evt_srv =
    vzbase::Thread::Current()->socketserver()->GetEvtService();
  dp_cli_ = DpClient_CreatePollHandle(dpcli_poll_msg_cb, this,
                                      dpcli_poll_state_cb, this,
                                      p_evt_srv);
  if (dp_cli_ == NULL) {
    LOG(L_ERROR) << "dp client create poll handle failed.";

    DpClient_ReleasePollHandle(dp_cli_);
    dp_cli_ = NULL;
    return false;
  }

  unsigned int n_method_set = sizeof(K_METHOD_SET) / sizeof(char*);
  DpClient_HdlAddListenMessage(dp_cli_, K_METHOD_SET, n_method_set);
  return true;
}

void CListenMessage::Stop() {
  if (dp_cli_) {
    DpClient_ReleasePollHandle(dp_cli_);
    dp_cli_ = NULL;
  }

  if (net_ctrl_) {
    delete net_ctrl_;
    net_ctrl_ = NULL;
  }

  if (thread_slow_) {
    thread_slow_->Release();
    thread_slow_ = NULL;
  }
  DpClient_Stop();

  vzbase::Thread::Current()->Release();
}

void CListenMessage::RunLoop() {
  vzbase::Thread::Current()->Run();
}

void CListenMessage::dpcli_poll_msg_cb(DPPollHandle p_hdl,
                                       const DpMessage *dmp,
                                       void* p_usr_arg) {
  if (p_usr_arg) {
    ((CListenMessage*)p_usr_arg)->OnDpCliMsg(p_hdl, dmp);
    return;
  }
  LOG(L_ERROR) << "param is error.";
}

void CListenMessage::OnDpCliMsg(DPPollHandle p_hdl, const DpMessage *dmp) {

}

void CListenMessage::dpcli_poll_state_cb(DPPollHandle p_hdl,
    unsigned int n_state,
    void* p_usr_arg) {
  if (p_usr_arg) {
    ((CListenMessage*)p_usr_arg)->OnDpCliState(p_hdl, n_state);
  }
}

void CListenMessage::OnDpCliState(DPPollHandle p_hdl, unsigned int n_state) {
  if (n_state == DP_CLIENT_DISCONNECT) {
    int32 n_ret = DpClient_HdlReConnect(p_hdl);
    if (n_ret == VZNETDP_SUCCEED) {
      unsigned int n_method_set = sizeof(K_METHOD_SET) / sizeof(char*);
      DpClient_HdlAddListenMessage(dp_cli_, K_METHOD_SET, n_method_set);
    }
  }
}

void CListenMessage::GetHwInfo() {
  sys_info_.dev_name =
    vzbase::Base64::Encode(vzbase::Gb2312ToUtf8("ÄãºÃ"));
  sys_info_.ins_addr =
    vzbase::Base64::Encode(vzbase::Gb2312ToUtf8("ÄãºÃ"));

  sys_info_.dev_type  = 100100;
  sys_info_.sw_ver    = 100000;
  sys_info_.hw_ver    = 100000;
  sys_info_.iva_ver   = 100000;
  sys_info_.web_port  = 8080;
  sys_info_.rtsp_port = 8554;
  sys_info_.rec_size  = 16000;
}

bool CListenMessage::GetDevInfo(Json::Value &j_body) {
  j_body["dev_name"]  = sys_info_.dev_name;
  j_body["dev_type"]  = sys_info_.dev_type;
  j_body["sw_ver"]    = sys_info_.sw_ver;
  j_body["hw_ver"]    = sys_info_.hw_ver;
  j_body["iva_ver"]   = sys_info_.iva_ver;
  j_body["ip_addr"]   = inet_ntoa(*((struct in_addr*)&net_ctrl_->ip_));
  j_body["netmask"]   = inet_ntoa(*((struct in_addr*)&net_ctrl_->netmask_));
  j_body["gateway"]   = inet_ntoa(*((struct in_addr*)&net_ctrl_->gateway_));
  j_body["dns"]       = inet_ntoa(*((struct in_addr*)&net_ctrl_->dns_));
  j_body["mac"]       = net_ctrl_->mac_;
  j_body["web_port"]  = sys_info_.web_port;
  j_body["rtsp_port"] = sys_info_.rtsp_port;
  j_body["rec_size"]  = sys_info_.rec_size;
  j_body["ins_addr"]  = sys_info_.ins_addr;
  return true;
}

bool CListenMessage::SetDevInfo(const Json::Value &j_body) {
  sys_info_.dev_name    = j_body["dev_name"].asString();
  sys_info_.ins_addr    = j_body["ins_addr"].asString();
  sys_info_.dev_type    = j_body["dev_type"].asUInt();

  if (sys_info_.web_port != j_body["web_port"].asUInt()) {

  }
  if (sys_info_.rtsp_port != j_body["rtsp_port"].asUInt()) {

  }
  return true;
}

}  // namespace sys
