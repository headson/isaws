/************************************************************************/
/* Author      : SoberPeng 2017-07-08
/* Description :
/************************************************************************/
#include "cnetctrl.h"

#include "vzbase/helper/stdafx.h"
#include "systemserver/clistenmessage.h"

CNetCtrl::CNetCtrl(vzbase::Thread *p_thread)
  : vzbase::MessageHandler()
  , p_thread_(p_thread)
  , p_mcast_sock_(NULL) {
}

CNetCtrl *CNetCtrl::Create(vzbase::Thread *p_thread) {
  if (p_thread == NULL) {
    LOG(L_ERROR) << "param is null.";
    return NULL;
  }

  return (new CNetCtrl(p_thread));
}

CNetCtrl::~CNetCtrl() {
  Stop();
}

bool CNetCtrl::Start() {
  if (NULL == p_thread_) {
    LOG(L_ERROR) << "create param is null.";
    return false;
  }

  p_mcast_sock_ = vzconn::CMCastSocket::Create(
                    p_thread_->socketserver()->GetEvtService(),
                    this);
  if (NULL == p_mcast_sock_) {
    LOG(L_ERROR) << "create mcast socket failed.";
    return false;
  }

  bool b_ret = p_mcast_sock_->Open((unsigned char*)DEF_MCAST_IP,
                                   DEF_MCAST_DEV_PORT);
  if (b_ret == false) {
    LOG(L_ERROR) << "multi socket open failed.";
  }
  return b_ret;
}

void CNetCtrl::Stop() {
  if (p_mcast_sock_) {
    delete p_mcast_sock_;
    p_mcast_sock_ = NULL;
  }
}

void CNetCtrl::OnMessage(vzbase::Message* msg) {

}

int32 CNetCtrl::HandleRecvPacket(vzconn::VSocket *p_cli,
                                 const char      *p_data,
                                 unsigned int     n_data,
                                 unsigned short   n_flag) {
  std::string s_json((char*)p_data, n_data);

  Json::Value  j_req;
  Json::Reader j_parse;
  if(!j_parse.parse(s_json, j_req)) {
    LOG(L_ERROR) << j_parse.getFormattedErrorMessages();
    return -1;
  }
  std::string s_type = j_req[MSG_TYPE].asString();
  if (strncmp(s_type.c_str(), MSG_SYSC_GET_DEVINFO, MSG_TYPE_MAX) == 0) {
    // 获取设备信息
    Json::Value j_resp;
    j_resp[MSG_TYPE]  = s_type;

    bool b_ret = CListenMessage::Instance()->GetDevInfo(j_resp[MSG_BODY]);
    if (b_ret) {
      j_resp[MSG_STATE] = 200;
      j_resp[MSG_ERR_MSG] = "all done";
    } else {
      j_resp[MSG_STATE] = 304;
      j_resp[MSG_ERR_MSG] = "get device info failed";
    }

    Json::FastWriter j_writer;
    s_json = j_writer.write(j_resp);

    p_mcast_sock_->SendUdpData(DEF_MCAST_IP, DEF_MCAST_CLI_PORT,
                               s_json.c_str(), s_json.size());
  } else if (strncmp(s_type.c_str(), MSG_SYSC_SET_DEVINFO, MSG_TYPE_MAX) == 0) {
    // 设置设备信息
    Json::Value j_resp;
    j_resp[MSG_TYPE]  = s_type;

    bool b_ret = CListenMessage::Instance()->SetDevInfo(j_req[MSG_BODY]);
    if (b_ret) {
      j_resp[MSG_STATE] = 200;
      j_resp[MSG_ERR_MSG] = "all done";
    } else {
      j_resp[MSG_STATE] = 304;
      j_resp[MSG_ERR_MSG] = "set device info failed";
    }

    Json::FastWriter j_writer;
    s_json = j_writer.write(j_resp);

    p_mcast_sock_->SendUdpData(DEF_MCAST_IP, DEF_MCAST_CLI_PORT,
                               s_json.c_str(), s_json.size());
  } else {
    LOG(L_ERROR) << "this message type is no function to process." << s_type;
  }

  return 0;
}
