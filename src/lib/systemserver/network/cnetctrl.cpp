/************************************************************************/
/* Author      : SoberPeng 2017-07-08
/* Description :
/************************************************************************/
#include "cnetctrl.h"

#include "vzbase/helper/stdafx.h"

#include "systemserver/clistenmessage.h"

namespace sys {

#define GET_IP_TIMEOUT  200       // 200ms

#define DEF_GET_IP_ADDR 0x123456

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

  p_thread_->PostDelayed(GET_IP_TIMEOUT, this, DEF_GET_IP_ADDR);
  return b_ret;
}

void CNetCtrl::Stop() {
  if (p_mcast_sock_) {
    delete p_mcast_sock_;
    p_mcast_sock_ = NULL;
  }
}

void CNetCtrl::OnMessage(vzbase::Message* msg) {
  if (msg->message_id == DEF_GET_IP_ADDR) {
#ifdef _WIN32
    // LOG(L_WARNING) << "set netmask.";
#else
    ip_ = net_get_ifaddr(PHY_IF_NAME);
    if (0 == ip_) {
      if (GetTickCount() > 30*1000)  {  // 30S
        LOG(L_INFO) << "set default address.";
        net_set_ifaddr(PHY_IF_NAME, inet_addr("192.168.254.254"));
        net_set_netmask(PHY_IF_NAME, inet_addr("255.255.0.0"));
        net_set_gateway(PHY_IF_NAME, inet_addr("192.168.254.1"));
      }
    } else {
      static unsigned int i = 0;
      if (inet_addr("192.168.254.254") != ip_) {
        // 闪灯频率 200ms

      } else {
        // 闪灯频率 1000ms
        if ((++i % 5) == 0) {

        }
      }
    }
  }
#endif
  }
}

int32 CNetCtrl::HandleRecvPacket(vzconn::VSocket  *p_cli,
                                 const uint8      *p_data,
                                 uint32            n_data,
                                 uint16            n_flag) {
  std::string s_json((char*)p_data, n_data);

  Json::Value  j_req;
  Json::Reader j_parse;
  if(!j_parse.parse(s_json, j_req)) {
    LOG(L_ERROR) << j_parse.getFormattedErrorMessages();
    return -1;
  }
  std::string s_type = j_req[MSG_CMD].asString();
  if (0 == strncmp(s_type.c_str(), MSG_SYSC_GET_DEVINFO, MSG_CMD_SIZE)) {
    // 获取设备信息
    Json::Value j_resp;
    j_resp[MSG_CMD]  = s_type;

    Json::Value j_body;
    bool b_ret = CListenMessage::Instance()->GetDevInfo(j_body);
    if (b_ret) {
      j_resp[MSG_STATE] = 0;
    } else {
      j_resp[MSG_STATE] = 6;
    }
    j_resp[MSG_BODY] = j_body;

    s_json = j_resp.toStyledString();
    p_mcast_sock_->SendUdpData((uint8*)DEF_MCAST_IP, DEF_MCAST_CLI_PORT,
                               (uint8*)s_json.c_str(), s_json.size());
  } else if (0 == strncmp(s_type.c_str(), MSG_SYSC_SET_DEVINFO, MSG_CMD_SIZE)) {
    // 设置设备信息
    Json::Value j_resp;
    j_resp[MSG_CMD]  = s_type;


    SetNet(inet_addr(j_resp[MSG_BODY]["ip_addr"].asCString()),
           inet_addr(j_resp[MSG_BODY]["netmask"].asCString()),
           inet_addr(j_resp[MSG_BODY]["gateway"].asCString()),
           inet_addr(j_resp[MSG_BODY]["dns"].asCString())); // 配置参数

    bool b_ret = CListenMessage::Instance()->SetDevInfo(j_resp[MSG_BODY]);
    if (b_ret) {
      j_resp[MSG_STATE] = 0;
    } else {
      j_resp[MSG_STATE] = 6;
    }

    s_json = j_resp.toStyledString();
    p_mcast_sock_->SendUdpData((uint8*)DEF_MCAST_IP, DEF_MCAST_CLI_PORT,
                               (uint8*)s_json.c_str(), s_json.size());
  } else {
    LOG(L_ERROR) << "this message type is no function to process." << s_type;
  }

  return 0;
}

void CNetCtrl::SetNet(in_addr_t   ip,
                      in_addr_t   netmask,
                      in_addr_t   gateway,
                      in_addr_t   dns) {
  if (ip_.S_un.S_addr != ip) {
    ip_.S_un.S_addr = ip;
#ifdef _WIN32
    LOG(L_WARNING) << "set ip addr " << inet_ntoa(ip_);
#else
    net_set_ifaddr(PHY_IF_NAME, ip);
#endif
  }

  if (netmask_.S_un.S_addr != netmask) {
    netmask_.S_un.S_addr = netmask;
#ifdef _WIN32
    LOG(L_WARNING) << "set netmask " << inet_ntoa(netmask_);
#else

    net_set_netmask(PHY_IF_NAME, netmask);
#endif
  }

  if (gateway_.S_un.S_addr != gateway) {
    gateway_.S_un.S_addr = gateway;
#ifdef _WIN32
    LOG(L_WARNING) << "set gateway " << inet_ntoa(gateway_);
#else
    net_clean_gateway();
    net_set_gateway(PHY_IF_NAME, gateway);
#endif
  }

  if (dns_.S_un.S_addr != dns) {
    dns_.S_un.S_addr = dns;
#ifdef _WIN32
    LOG(L_WARNING) << "set dns_1 " << inet_ntoa(dns_);
#else
    net_set_dns(inet_ntoa(dns_));
#endif
  }
}

}  // namespace sys
