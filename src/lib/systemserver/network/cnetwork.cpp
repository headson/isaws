/************************************************************************/
/* Author      : SoberPeng 2017-07-08
/* Description :
/************************************************************************/
#include "cnetwork.h"

#include "net_cfg.h"

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "systemserver/clistenmessage.h"

namespace sys {

#define BOARDCAST_TIMEOUT  5*1000       // 5S

//////////////////////////////////////////////////////////////////////////
CNetwork::CNetwork(vzbase::Thread *thread_fast)
  : vzbase::MessageHandler()
  , vzconn::CClientInterface()
  , thread_fast_(thread_fast)
  , mcast_sock_(NULL) {
  nickname_ = PHY_ETH_NAME;
}

CNetwork *CNetwork::Create(vzbase::Thread *thread_fast) {
  if (thread_fast == NULL) {
    LOG(L_ERROR) << "param is null.";
    return NULL;
  }

  return (new CNetwork(thread_fast));
}

CNetwork::~CNetwork() {
  Stop();
}

bool CNetwork::Start() {
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

void CNetwork::Stop() {
  if (mcast_sock_) {
    delete mcast_sock_;
    mcast_sock_ = NULL;
  }
}

void CNetwork::OnMessage(vzbase::Message* msg) {

}

int32 CNetwork::HandleRecvPacket(vzconn::VSocket  *p_cli,
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
    // Boardcast();
  } else if (0 == strncmp(s_type.c_str(), MSG_SET_DEVINFO, MSG_CMD_SIZE)) {
    // 设置设备信息
    Json::Value jret;
    jret[MSG_CMD]  = s_type;

    std::string net_dev_uuid = jreq[MSG_BODY]["dev_uuid"].asString();
    std::string dev_uuid = CListenMessage::Instance()->sys_info_.dev_uuid;
    if (0 == dev_uuid.compare(net_dev_uuid)) {
      bool bres = CListenMessage::Instance()->SetDevInfo(jreq[MSG_BODY]);
      if (bres) {
        jret[MSG_STATE] = 0;
      } else {
        jret[MSG_STATE] = 6;
      }

      sjson = jret.toStyledString();

      NET_CONFIG net_cfg;
      mcast_sock_->SendUdpData(DEF_MCAST_IP, DEF_MCAST_CLI_PORT,
                               sjson.c_str(), sjson.size(),
                               network_.ip_addr.c_str());
    }
  } else {
    LOG(L_ERROR) << "this message type is no function to process." << s_type;
  }

  return 0;
}

void CNetwork::Boardcast(const char *pData, int nData) {
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


bool CNetwork::GetNet(Json::Value &jnet) {
  NET_CONFIG net_cfg = {0};
#ifdef _LINUX
  net_get_info(nickname_.c_str(), &net_cfg);
#endif
  static Json::Value jnet_;
  if (network_.ip_addr != inet_ntoa(*((struct in_addr*)&net_cfg.ifaddr)) ||
      network_.netmask != inet_ntoa(*((struct in_addr*)&net_cfg.netmask)) ||
      network_.gateway != inet_ntoa(*((struct in_addr*)&net_cfg.gateway)) ||
      network_.dns_addr != inet_ntoa(*((struct in_addr*)&net_cfg.dns))) {
    network_.ip_addr = inet_ntoa(*((struct in_addr*)&net_cfg.ifaddr));
    network_.netmask = inet_ntoa(*((struct in_addr*)&net_cfg.netmask));
    network_.gateway = inet_ntoa(*((struct in_addr*)&net_cfg.gateway));
    network_.dns_addr = inet_ntoa(*((struct in_addr*)&net_cfg.dns));

    char smac[32] = {0};
    snprintf(smac, 31, "%02x:%02x:%02x:%02x:%02x:%02x\n",
             net_cfg.mac[0], net_cfg.mac[1], net_cfg.mac[2],
             net_cfg.mac[3], net_cfg.mac[4], net_cfg.mac[5]);
    network_.phy_mac = smac;

    jnet_.clear();
    jnet_["ip_addr"] = network_.ip_addr;
    jnet_["netmask"] = network_.netmask;
    jnet_["gateway"] = network_.gateway;
    jnet_["dns_addr"] = network_.gateway;
    jnet_["phy_mac"] = network_.dns_addr;
  }
  jnet = jnet_;
}

bool CNetwork::SetNet(const Json::Value &jnet) {

}

int CNetwork::SetIp(in_addr_t ip) {
  int ret = -1;
  struct in_addr sys_ip;
  const char* nicname = nickname_.c_str();

#ifdef _LINUX
  if ((sys_ip.s_addr = net_get_ifaddr(nicname)) != -1) {
    if (ip != sys_ip.s_addr) {

      if (net_set_ifaddr(nicname, ip) >= 0) {
        // 通知星型结构ip have changed
        network_.ip_addr = inet_ntoa(*((struct in_addr*)&ip));
        ip_change_ = 1;
        ret = 0;
      }
    } else {
      ret = 0;
    }
  }
#else
  network_.ip_addr = inet_ntoa(*((struct in_addr*)&ip));
  ip_change_ = 1;
  ret = 0;
#endif
  return ret;
}

int CNetwork::SetNetmask(in_addr_t ip) {
  int ret = -1;
  struct in_addr sys_ip;
  const char* nicname = nickname_.c_str();
  std::string snetmask = inet_ntoa(*((struct in_addr*)&ip));

#ifdef _LINUX
  if ((sys_ip.s_addr = net_get_netmask(nicname)) != -1) {
    int sucess_flag = 1;
    if (ip != sys_ip.s_addr) {
      if (net_set_netmask(nicname, ip) < 0) {
        sucess_flag = 0;
      }
    }

    if ((sucess_flag == 1) &&
        (snetmask != network_.netmask)) {
      network_.netmask = snetmask;
      ip_change_ = 1;
      ret = 0;
    } else {
      ret = 0;
    }
  }
#else
  network_.netmask = snetmask;
  ip_change_ = 1;
  ret = 0;
#endif
  return ret;
}

int CNetwork::SetGateway(in_addr_t ipaddr) {
  int ret = -1;
  struct in_addr sys_ip;
  const char* nicname = nickname_.c_str();

#ifdef _LINUX
  sys_ip.s_addr = net_get_gateway_byname((char *)nicname);
  LOG(L_INFO) << "ipaddr:" << ipaddr << ", sys_ip.s_addr:" << sys_ip.s_addr;
  if (ipaddr != sys_ip.s_addr) {
    struct in_addr ip, netmask, gateway;
    ip.s_addr = net_get_ifaddr(nicname);
    netmask.s_addr = net_get_netmask(nicname);
    if ((unsigned int)(netmask.s_addr&ip.s_addr) != (unsigned int)(
          ipaddr&netmask.s_addr)) {
      // look a useable gateway
      gateway.s_addr = net_search_gateway_byname((char *)nicname);
      if ((unsigned int)(netmask.s_addr&ip.s_addr) == (unsigned int)(
            gateway.s_addr&netmask.s_addr)) {
        ipaddr = gateway.s_addr;
      } else {
        ipaddr = (netmask.s_addr&ip.s_addr) | 0x1000000;
      }
    }
    LOG(L_INFO) << "ipaddr:" << ipaddr;
    if (net_set_gateway_byname((char *)nicname, ipaddr) >= 0) {
      network_.gateway = inet_ntoa(*((struct in_addr*)&ipaddr));
      ip_change_ = 1;
      ret = 0;
    }
  } else {
    ret = 0;
  }
#else
  network_.gateway = inet_ntoa(*((struct in_addr*)&ipaddr));
  ip_change_ = 1;
  ret = 0;
#endif
  LOG(L_INFO) << "SetGateway return:" << ret;
  return ret;
}

int CNetwork::SetDNS(in_addr_t ip) {
  int ret = -1;
  struct in_addr sys_ip;
  std::string sdns = inet_ntoa(*((struct in_addr*)&ip));
#ifdef _LINUX
  if ((sys_ip.s_addr = net_get_dns()) != -1) {
    if (ip != sys_ip.s_addr) {
      if (net_set_dns(sdns.c_str()) >= 0) {
        network_.dns_addr = sdns;
        ret = 0;
      }
    } else {
      ret = 0;
    }
  }
#else
  network_.dns_addr = sdns;
  ret = 0;
#endif
  return ret;
}

}  // namespace sys

