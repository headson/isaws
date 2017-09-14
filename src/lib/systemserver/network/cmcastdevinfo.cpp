/************************************************************************/
/* Author      : SoberPeng 2017-07-08
/* Description :
/************************************************************************/
#include "cmcastdevinfo.h"

#include "net_cfg.h"

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "vzbase/base/mysystem.h"
#include "systemserver/clistenmessage.h"

namespace sys {

#define MCAST_TIMEOUT  5*1000       // 5S

//////////////////////////////////////////////////////////////////////////
CMCastDevInfo::CMCastDevInfo(vzbase::Thread *thread_fast)
  : vzbase::MessageHandler()
  , thread_fast_(thread_fast)
  , mcast_sock_(NULL) {
}

CMCastDevInfo *CMCastDevInfo::Create(vzbase::Thread *thread_fast) {
  if (thread_fast == NULL) {
    LOG(L_ERROR) << "param is null.";
    return NULL;
  }

  return (new CMCastDevInfo(thread_fast));
}

CMCastDevInfo::~CMCastDevInfo() {
  Stop();
}

bool CMCastDevInfo::Start() {
  if (NULL == thread_fast_) {
    LOG(L_ERROR) << "create param is null.";
    return false;
  }

  mcast_sock_ = vzconn::CMCastSocket::Create(
                  thread_fast_->socketserver()->GetEvtService(),
                  this);
  if (NULL == mcast_sock_) {
    LOG(L_ERROR) << "create mcast socket failed.";
    return false;
  }

  int ret = mcast_sock_->Open(DEF_MCAST_IP,
                              DEF_MCAST_DEV_PORT);
  if (ret != false) {
    LOG(L_ERROR) << "multi socket open failed.";
  }

  OnMessage(NULL);
  return (ret == 0);
}

void CMCastDevInfo::Stop() {
  if (mcast_sock_) {
    delete mcast_sock_;
    mcast_sock_ = NULL;
  }
}

bool CMCastDevInfo::AfterAdjustNetwork() {
  return true;
}

void CMCastDevInfo::OnMessage(vzbase::Message* msg) {
  static int bcast_times = 5;  // 5S
  if ((bcast_times--) <= 0) {
    bcast_times = 5;
    BcastDevInfo();
  }

  thread_fast_->PostDelayed(MCAST_TIMEOUT, this);
}

int32 CMCastDevInfo::HandleRecvPacket(vzconn::VSocket  *p_cli,
                                 const uint8      *p_data,
                                 uint32            n_data,
                                 uint16            n_flag) {
  std::string sjson((char*)p_data, n_data);
  LOG(L_INFO) << sjson.c_str();

  Json::Value  jreq;
  Json::Reader jread;
  if(!jread.parse(sjson, jreq)) {
    LOG(L_ERROR) << jread.getFormattedErrorMessages();
    return -1;
  }
  std::string s_type = jreq[MSG_CMD].asString();
  if (0 == strncmp(s_type.c_str(), MSG_GET_DEVINFO, MSG_CMD_SIZE)) {
    // 获取设备信息
    BcastDevInfo();
  } else if (0 == strncmp(s_type.c_str(), MSG_SET_DEVINFO, MSG_CMD_SIZE)) {
    // 设置设备信息
    Json::Value j_resp;
    j_resp[MSG_CMD]  = s_type;

    std::string net_dev_uuid = jreq[MSG_BODY]["dev_uuid"].asString();
    std::string dev_uuid = CListenMessage::Instance()->sys_info_.dev_uuid;
    if (0 == dev_uuid.compare(net_dev_uuid)) {
      bool b_ret = CListenMessage::Instance()->SetDevInfo(jreq[MSG_BODY]);
      if (b_ret) {
        j_resp[MSG_STATE] = 0;
      } else {
        j_resp[MSG_STATE] = 6;
      }

      sjson = j_resp.toStyledString();
      mcast_sock_->SendUdpData(DEF_MCAST_IP, DEF_MCAST_CLI_PORT,
                               sjson.c_str(), sjson.size());
    }
  } else {
    LOG(L_ERROR) << "this message type is no function to process." << s_type;
  }

  return 0;
}

void CMCastDevInfo::BcastDevInfo() {
  Json::Value j_resp;
  j_resp[MSG_CMD] = MSG_GET_DEVINFO;

  Json::Value j_body;
  bool b_ret = CListenMessage::Instance()->GetDevInfo(j_body);
  if (b_ret) {
    j_resp[MSG_STATE] = 0;
  } else {
    j_resp[MSG_STATE] = 6;
  }
  j_resp[MSG_BODY] = j_body;

  std::string sjson = j_resp.toStyledString();
  mcast_sock_->SendUdpData(DEF_MCAST_IP, DEF_MCAST_CLI_PORT,
                           sjson.c_str(), sjson.size());
}

}  // namespace sys
