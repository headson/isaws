/************************************************************************/
/* Author      : SoberPeng 2017-07-08
/* Description :
/************************************************************************/
#include "cnetctrl.h"

#include "net_cfg.h"

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "vzbase/base/mysystem.h"
#include "systemserver/clistenmessage.h"

namespace sys {

#define GET_IP_TIMEOUT  200       // 200ms

#define DEF_GET_PHY_ADDR 0x123456

in_addr_t    CNetCtrl::ip_addr_   = 0;
in_addr_t    CNetCtrl::netmask_   = 0;
in_addr_t    CNetCtrl::gateway_   = 0;
in_addr_t    CNetCtrl::dns_addr_  = 0;

std::string  CNetCtrl::phy_mac_   = "";

//////////////////////////////////////////////////////////////////////////
CNetCtrl::CNetCtrl(vzbase::Thread *thread_fast)
  : vzbase::MessageHandler()
  , thread_fast_(thread_fast)
  , mcast_sock_(NULL) {
}

CNetCtrl *CNetCtrl::Create(vzbase::Thread *thread_fast) {
  if (thread_fast == NULL) {
    LOG(L_ERROR) << "param is null.";
    return NULL;
  }

  return (new CNetCtrl(thread_fast));
}

CNetCtrl::~CNetCtrl() {
  Stop();
}

bool CNetCtrl::Start() {
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

  int ret = mcast_sock_->Open((unsigned char*)DEF_MCAST_IP,
                              DEF_MCAST_DEV_PORT);
  if (ret != false) {
    LOG(L_ERROR) << "multi socket open failed.";
  }

  thread_fast_->PostDelayed(GET_IP_TIMEOUT, this, DEF_GET_PHY_ADDR);
  return (ret == 0);
}

void CNetCtrl::Stop() {
  if (mcast_sock_) {
    delete mcast_sock_;
    mcast_sock_ = NULL;
  }
}

void CNetCtrl::OnMessage(vzbase::Message* msg) {
  if (msg->message_id == DEF_GET_PHY_ADDR) {
#ifdef _WIN32
    // LOG(L_WARNING) << "get phy addr.";
#else
    in_addr_t ip = net_get_ifaddr(PHY_ETH_NAME);
    if (0 == ip) {
      if (GetTickCount() > 30*1000)  {  // 30S
        LOG(L_INFO) << "set default address.";
        net_set_ifaddr(PHY_ETH_NAME, inet_addr("192.168.254.254"));
        net_set_netmask(PHY_ETH_NAME, inet_addr("255.255.0.0"));
        net_set_gateway(inet_addr("192.168.254.1"));
      }
    } else {
      if (ip_addr_ != ip) {
        netmask_  = net_get_netmask(PHY_ETH_NAME);
        gateway_  = net_get_gateway();
        dns_addr_ = net_get_dns();

        unsigned char smac[20] = {0};
        net_get_hwaddr(PHY_ETH_NAME, smac);
        phy_mac_ = (char*)smac;
      }
      ip_addr_ = ip;

      static unsigned int i = 0;
      if (inet_addr("192.168.254.254") != ip_addr_) {
        // 闪灯频率 200ms

      } else {
        // 闪灯频率 1000ms
        if ((++i % 5) == 0) {

        }
      }
    }
#endif
    thread_fast_->PostDelayed(GET_IP_TIMEOUT, this, DEF_GET_PHY_ADDR);
  }
}

int32 CNetCtrl::HandleRecvPacket(vzconn::VSocket  *p_cli,
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
    Json::Value j_resp;
    j_resp[MSG_CMD] = s_type;

    Json::Value j_body;
    bool b_ret = CListenMessage::Instance()->GetDevInfo(j_body);
    if (b_ret) {
      j_resp[MSG_STATE] = 0;
    } else {
      j_resp[MSG_STATE] = 6;
    }
    j_resp[MSG_BODY] = j_body;

    sjson = j_resp.toStyledString();
    mcast_sock_->SendUdpData((uint8*)DEF_MCAST_IP, DEF_MCAST_CLI_PORT,
                             (uint8*)sjson.c_str(), sjson.size());
  } else if (0 == strncmp(s_type.c_str(), MSG_SET_DEVINFO, MSG_CMD_SIZE)) {
    // 设置设备信息
    Json::Value j_resp;
    j_resp[MSG_CMD]  = s_type;

    bool b_ret = CListenMessage::Instance()->SetDevInfo(j_resp[MSG_BODY]);
    if (b_ret) {
      j_resp[MSG_STATE] = 0;
    } else {
      j_resp[MSG_STATE] = 6;
    }

    sjson = j_resp.toStyledString();
    mcast_sock_->SendUdpData((uint8*)DEF_MCAST_IP, DEF_MCAST_CLI_PORT,
                             (uint8*)sjson.c_str(), sjson.size());
  } else {
    LOG(L_ERROR) << "this message type is no function to process." << s_type;
  }

  return 0;
}

bool CNetCtrl::ModityNetwork(const TAG_SYS_INFO &sys_info) {
#ifdef _LINUX
  net_nic_up(PHY_ETH_NAME);
#endif

  if (1 == sys_info.net.dhcp_en) {
    vzbase::my_system("killall -9 udhcpc; udhcpc -i eth0 &");
    return true;
  } else {
    vzbase::my_system("killall -9 udhcpc");
  }

  bool addr_modify = false;

  in_addr_t ip_addr = inet_addr(sys_info.net.ip_addr.c_str());
  if (ip_addr_ != ip_addr) {
    addr_modify = true;
    ip_addr_ = ip_addr;

    LOG(L_WARNING) << "set ip addr " << sys_info.net.ip_addr;
#ifdef _LINUX
    net_set_ifaddr(PHY_ETH_NAME, ip_addr_);
#endif
  }

  in_addr_t netmask = inet_addr(sys_info.net.netmask.c_str());
  if (netmask_ != netmask) {
    addr_modify = true;
    netmask_ = netmask;

    LOG(L_WARNING) << "set netmask " << sys_info.net.netmask;
#ifdef _LINUX
    net_set_netmask(PHY_ETH_NAME, netmask);
#endif
  }

  in_addr_t gateway = inet_addr(sys_info.net.gateway.c_str());
  if (gateway_ != gateway) {
    addr_modify = true;
    gateway_ = gateway;

    LOG(L_WARNING) << "set gateway " << sys_info.net.gateway;
#ifdef _LINUX
    net_clean_gateway();
    net_set_gateway(gateway);
#endif
  }

  in_addr_t dns_addr = inet_addr(sys_info.net.dns_addr.c_str());
  if (dns_addr_ != dns_addr) {
    dns_addr_ = dns_addr;

    LOG(L_WARNING) << "set dns_1 " << sys_info.net.dns_addr;
#ifdef _LINUX
    net_set_dns(inet_ntoa(*((struct in_addr*)&dns_addr_)));
#endif
  }

  if (addr_modify) {
    DpClient_SendDpMessage(MSG_ADDR_CHANGE, 0, NULL, 0);
  }
  return addr_modify;
}

}  // namespace sys
