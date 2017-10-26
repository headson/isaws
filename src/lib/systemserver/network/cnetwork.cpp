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

#define MCAST_TIMEOUT  5*1000       // 5S

//////////////////////////////////////////////////////////////////////////
CNetwork::CNetwork(std::string suuid, vzbase::Thread *thread_fast)
  : vzbase::MessageHandler()
  , suuid_(suuid)
  , thread_fast_(thread_fast)
  , mcast_sock_(NULL) {
  nickname_ = PHY_ETH_NAME;
}

CNetwork *CNetwork::Create(std::string suuid, vzbase::Thread *thread_fast) {
  if (thread_fast == NULL) {
    LOG(L_ERROR) << "param is null.";
    return NULL;
  }

  return (new CNetwork(suuid, thread_fast));
}

CNetwork::~CNetwork() {
  Stop();
}

bool CNetwork::Start() {
  if (NULL == thread_fast_) {
    LOG(L_ERROR) << "create param is null.";
    return false;
  }

  //
  SetNetToSys();

  // 组播
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

bool CNetwork::GetNet(Json::Value &jret) {
  GetNetFromSys();

  jret["wifi_en"] = network_.wifi_en;
  jret["dhcp_en"] = network_.dhcp_en;

  jret["ip_addr"] = network_.ip_addr;
  jret["netmask"] = network_.netmask;
  jret["gateway"] = network_.gateway;
  jret["phy_mac"] = network_.phy_mac;

  jret["dns_addr"] = network_.dns_addr;

  jret["http_port"] = network_.http_port;
  return true;
}

bool CNetwork::SetNet(const Json::Value &jreq) {
  int res = 0;
  try {
    int wifi_en = jreq["wifi_en"].asInt();
    int dhcp_en = jreq["dhcp_en"].asInt();
    if (1 == wifi_en) {
      if (1 == dhcp_en) {
        // 开启wifi, 关闭eth0
        nickname_ = PHY_WIFI_NAME;

        ip_change_ = 1;
      }
    } else {
      if (1 == dhcp_en) {
        // 启动DHCP

        ip_change_ = 1;
      } else {
        std::string ip = jreq["ip_addr"].asString();
        SetIp(inet_addr(ip.c_str()));

        std::string netmask = jreq["netmask"].asString();
        SetNetmask(inet_addr(netmask.c_str()));

        std::string gateway = jreq["gateway"].asString();
        SetGateway(inet_addr(gateway.c_str()));

        std::string dns = jreq["dns_addr"].asString();
        SetDNS(inet_addr(dns.c_str()));
      }
    }
  } catch (...) {
  }
  if (ip_change_ > 0) {
    LOG(L_ERROR) << "network is change.";

    Json::FastWriter jfw;
    std::string snet = jfw.write(jreq);
    res = Kvdb_SetKey(KVDB_NETWORK,
                      strlen(KVDB_NETWORK),
                      snet.c_str(), snet.size());
    if (res == KVDB_RET_SUCCEED) {
      return true;
    }
  }
  return false;
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
                           sjson.c_str(), sjson.size(),
                           network_.ip_addr.c_str());
}

void CNetwork::OnMessage(vzbase::Message* msg) {
  static int bcast_times = 5;  // 5S
  if ((bcast_times--) <= 0) {
    bcast_times = 5;
    // Boardcast();
  }

  thread_fast_->PostDelayed(MCAST_TIMEOUT, this);
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
  std::string stype = jreq[MSG_CMD].asString();
  if (0 == strncmp(stype.c_str(), MSG_GET_DEVINFO, MSG_CMD_SIZE)) {
    // 获取设备信息
    // Boardcast();
  } else if (0 == strncmp(stype.c_str(), MSG_SET_DEV_NET, MSG_CMD_SIZE)) {
    // 设置设备信息
    Json::Value jret;
    jret[MSG_CMD]  = stype;

    std::string dev_uuid = jreq[MSG_BODY]["dev_uuid"].asString();
    if (0 == suuid_.compare(dev_uuid)) {
      bool b_ret = SetNet(jreq[MSG_BODY]);
      if (b_ret) {
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
    LOG(L_ERROR) << "this message type is no function to process." << stype;
  }

  return 0;
}

//////////////////////////////////////////////////////////////////////////
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

void CNetwork::SetNetToSys() {
  std::string snet;
  int res = Kvdb_GetKeyToString(
              KVDB_NETWORK, strlen(KVDB_NETWORK), &snet);

  Json::Value jnet;
  if (res > 0) {
    Json::Reader jread;
    if (!jread.parse(snet, jnet)) {
      res = -1;
    }
  }
  if (res <= 0) {
    jnet["wifi_en"] = 0;
    jnet["dhcp_en"] = 0;

    jnet["ip_addr"] = "192.168.1.88";
    jnet["netmask"] = "255.255.255.0";
    jnet["gateway"] = "192.168.1.1";

    jnet["dns_addr"] = "192.168.1.1";

    jnet["http_port"] = 80;
  }

  SetNet(jnet);
}

void CNetwork::GetNetFromSys() {
  if (!network_.ip_addr.empty() &&
      !network_.netmask.empty() &&
      !network_.gateway.empty() &&
      !network_.phy_mac.empty()) {
    LOG(L_WARNING) << "network is empty.";
    return;
  }

  NET_CONFIG net_cfg = { 0 };
#ifdef _LINUX
  net_get_info(nickname_.c_str(), &net_cfg);
#endif

  network_.ip_addr = inet_ntoa(*((struct in_addr*)&net_cfg.ifaddr));
  network_.netmask = inet_ntoa(*((struct in_addr*)&net_cfg.netmask));
  network_.gateway = inet_ntoa(*((struct in_addr*)&net_cfg.gateway));

  char smac[32] = { 0 };
  snprintf(smac, 31, "%02x:%02x:%02x:%02x:%02x:%02x\n",
           net_cfg.mac[0], net_cfg.mac[1], net_cfg.mac[2],
           net_cfg.mac[3], net_cfg.mac[4], net_cfg.mac[5]);
  network_.phy_mac = smac;

  network_.dns_addr =inet_ntoa(*((struct in_addr*)&net_cfg.dns));
}
}  // namespace sys
