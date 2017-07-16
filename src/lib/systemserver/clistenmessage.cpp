/************************************************************************/
/* Author      : SoberPeng 2017-06-24
/* Description :
/************************************************************************/
#include "clistenmessage.h"
#include "vzbase/helper/stdafx.h"
#include "vzbase/core/vmessage.h"
#include "vzbase/core/vdatetime.h"

#include <fstream>
#include "json/json.h"

#include "vzbase/base/helper.h"
#include "vzbase/base/base64.h"

namespace sys {

static const char  *K_METHOD_SET[] = {
  DP_SYS_CONFIG,
};

CListenMessage::CListenMessage()
  : dp_cli_(NULL)
  , net_ctrl_(NULL)
  , hw_clock_(NULL)
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

  GetHwInfo();  // 获取系统信息

  // fast thread
  thread_fast_ = vzbase::Thread::Current();

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
  net_ctrl_ = CNetCtrl::Create(thread_fast_);
  if (NULL == net_ctrl_) {
    LOG(L_ERROR) << "create net ctrl failed.";
    return false;
  }
  b_ret = net_ctrl_->Start();
  if (b_ret == false) {
    LOG(L_ERROR) << "net ctrl start failed.";
    return false;
  }

  // hw clock
  hw_clock_ = CHwclock::Create(thread_slow_);
  if (NULL == hw_clock_) {
    LOG(L_ERROR) << "create hw clock failed.";
    return false;
  }
  b_ret = hw_clock_->Start();
  if (false == b_ret) {
    LOG(L_ERROR) << "hwclock start failed.";
    return false;
  }

  // dp client
  DpClient_Init(s_dp_ip, n_dp_port);

  vzconn::EventService *p_evt_srv =
    thread_fast_->socketserver()->GetEvtService();
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

  thread_fast_->Release();
}

void CListenMessage::RunLoop() {
  thread_fast_->Run();
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
  std::string   sjson;
  sjson.append(dmp->data, dmp->data_size);

  Json::Value   jreq;
  Json::Reader  jread;
  if (!jread.parse(sjson, jreq)) {
    LOG(L_ERROR) << "iva info parse failed.";
    return;
  }

  Json::Value   jresp;
  jresp[MSG_CMD]   = jreq[MSG_CMD].asString();
  jresp[MSG_ID]    = jreq[MSG_ID].asInt();
  jresp[MSG_STATE] = RET_FAILED;

  bool breply = false; // 返回
  if (0 == strncmp(dmp->method, MSG_IVAC_GET_INFO, dmp->method_size)) {
    // 算法信息
    if (jreq[MSG_BODY]["version"].isString()) {
      sys_info_.iva_ver = jreq[MSG_BODY]["version"].asString();
    }
  } else if (0 == strncmp(dmp->method, MSG_SYSC_SET_INFO, dmp->method_size)) {
    breply = true;
    if (SetDevInfo(jreq[MSG_BODY])) {
      jresp[MSG_STATE] = RET_SUCCESS;
    }
  }

  if (breply) {
    sjson = jresp.toStyledString();
    DpClient_SendDpReply(dmp->method,
                         dmp->channel_id,
                         dmp->id,
                         sjson.c_str(), sjson.size());
  }
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
  Json::Reader jread;
  Json::Value  jroot;

  jroot.clear();
  std::ifstream ifs;

#ifdef _WIN32
  ifs.open("./system.json");
#else
  ifs.open("/mnt/etc/system.json");
#endif
  if(!ifs.is_open() ||
      !jread.parse(ifs, jroot)) {
    LOG(L_ERROR) << "system json parse failed.";
  }

  if (jroot["dev_name"].isString()) {
    sys_info_.dev_name = jroot["dev_name"].asString();
  } else {
    sys_info_.dev_name = "PC_001";
  }

  if (jroot["ins_addr"].isString()) {
    sys_info_.ins_addr = jroot["ins_addr"].asString();
  }

  if (jroot["dev_type"].isInt()) {
    sys_info_.dev_type  = jroot["dev_type"].asInt();
  } else {
    sys_info_.dev_type = 100100;
  }

  sys_info_.sw_ver    = SW_VERSION_;
  sys_info_.sw_ver   += __DATE__;
  sys_info_.hw_ver    = HW_VERSION_;

  // dp获取算法信息
  DpClient_SendDpRequest(MSG_IVAC_GET_INFO, 0,
                         NULL, 0,
                         dpcli_poll_msg_cb, this,
                         DEF_TIMEOUT_MSEC);
  if (sys_info_.iva_ver.empty()) {
    sys_info_.iva_ver = "V100.00.00 ";
    sys_info_.iva_ver   += __DATE__;
  }

  if (jroot["web_port"].isInt()) {
    sys_info_.web_port  = jroot["web_port"].asInt();
  } else {
    sys_info_.web_port = 80;
  }

  if (jroot["rtsp_port"].isInt()) {
    sys_info_.rtsp_port  = jroot["rtsp_port"].asInt();
  } else {
    sys_info_.rtsp_port = 8554;
  }

  // 读取磁盘大小
  sys_info_.rec_size  = 16000;

  if (jroot["ip_addr"].isString() &&
      jroot["netmask"].isString() &&
      jroot["gateway"].isString() &&
      jroot["dns"].isString()) {
    CNetCtrl::SetNet(inet_addr(jroot["ip_addr"].asCString()),
                     inet_addr(jroot["netmask"].asCString()),
                     inet_addr(jroot["gateway"].asCString()),
                     inet_addr(jroot["dns"].asCString())); // 配置参数
  }
}

bool CListenMessage::GetDevInfo(Json::Value &j_body) {
  j_body["dev_name"]  = sys_info_.dev_name;
  j_body["dev_type"]  = sys_info_.dev_type;
  j_body["sw_ver"]    = sys_info_.sw_ver;
  j_body["hw_ver"]    = sys_info_.hw_ver;
  j_body["iva_ver"]   = sys_info_.iva_ver;
  j_body["dhcp"]      = sys_info_.dhcp_en;
  j_body["ip_addr"]   = inet_ntoa(*((struct in_addr*)&CNetCtrl::ip_));
  j_body["netmask"]   = inet_ntoa(*((struct in_addr*)&CNetCtrl::netmask_));
  j_body["gateway"]   = inet_ntoa(*((struct in_addr*)&CNetCtrl::gateway_));
  j_body["dns"]       = inet_ntoa(*((struct in_addr*)&CNetCtrl::dns_));
  j_body["mac"]       = CNetCtrl::mac_;
  j_body["web_port"]  = sys_info_.web_port;
  j_body["rtsp_port"] = sys_info_.rtsp_port;
  j_body["rec_size"]  = sys_info_.rec_size;
  j_body["ins_addr"]  = sys_info_.ins_addr;
  return true;
}

bool CListenMessage::SetDevInfo(const Json::Value &j_body) {
  if (j_body["dev_name"].isString()) {
    sys_info_.dev_name    = j_body["dev_name"].asString();
  }

  if (j_body["ins_addr"].isString()) {
    sys_info_.ins_addr    = j_body["ins_addr"].asString();
  }

  if (j_body["dev_type"].isInt()) {
    sys_info_.dev_type    = j_body["dev_type"].asUInt();
  }

  if (sys_info_.web_port != j_body["web_port"].asUInt()) {
  }
  if (sys_info_.rtsp_port != j_body["rtsp_port"].asUInt()) {
  }

  // dhcp判断
  if (0 < j_body["dhcp"].asInt()) {

  } else {
    if (j_body["ip_addr"].isString() &&
      j_body["netmask"].isString() &&
      j_body["gateway"].isString() &&
      j_body["dns"].isString()) {
        CNetCtrl::SetNet(inet_addr(j_body["ip_addr"].asCString()),
          inet_addr(j_body["netmask"].asCString()),
          inet_addr(j_body["gateway"].asCString()),
          inet_addr(j_body["dns"].asCString())); // 配置参数
    }
  }
  
  std::ofstream ofs;
#ifdef _WIN32
  ofs.open("./system.json");
#else
  ofs.open("/mnt/etc/system.json");
#endif
  ofs<<j_body.toStyledString();
  ofs.close();
  return true;
}

}  // namespace sys
