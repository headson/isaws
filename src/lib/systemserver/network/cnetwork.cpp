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
CNetwork::CNetwork(vzbase::Thread *thread_fast)
  : vzbase::MessageHandler()
  , thread_fast_(thread_fast)
  , mcast_sock_(NULL) {
  nickname_ = PHY_ETH_NAME;
}

CNetwork *CNetwork::Create(vzbase::Thread *thread_fast) {
  if (thread_fast == NULL) {
    LOG(L_ERROR) << "param is null.";
    return NULL;
  }
  CNetwork *net = new CNetwork(thread_fast);
  if (NULL == net) {
    LOG(L_ERROR) << "CNetwork create failed.";
  }
  return net;
}

CNetwork::~CNetwork() {
  Stop();
}

bool CNetwork::Start() {
  if (NULL == thread_fast_) {
    LOG(L_ERROR) << "create param is null.";
    return false;
  }
  SysBngSetNet();

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

bool CNetwork::OnDpMessage(const DpMessage *dmp,
                           const Json::Value &jreq, Json::Value &jret) {
  std::string scmd = jreq[MSG_CMD].asString();
  if (0 == scmd.compare(MSG_GET_NETWORK)) {
    return OnGetNet(jreq, jret);
  } else if (0 == scmd.compare(MSG_SET_NETWORK)) {
    return OnSetNet(jreq, jret);
  }
  LOG(L_ERROR) << "not this cmd to process: " << scmd;
  return false;
}

void CNetwork::OnMessage(vzbase::Message* msg) {
  static int bcast_times = 5;  // 5S
  if ((bcast_times--) <= 0) {
    bcast_times = 5;
    BcastDevInfo();
  }

  OnCheckNetAddr();

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
  std::string s_type = jreq[MSG_CMD].asString();
  if (0 == strncmp(s_type.c_str(), MSG_GET_DEVINFO, MSG_CMD_SIZE)) {
    // 获取设备信息
    BcastDevInfo();
  } else if (0 == strncmp(s_type.c_str(), MSG_SET_DEVINFO, MSG_CMD_SIZE)) {
    // 设置设备信息
    Json::Value j_resp;
    j_resp[MSG_CMD]  = s_type;

    std::string net_dev_uuid = jbody["dev_uuid"].asString();
    std::string dev_uuid = CListenMessage::Instance()->dev_info_.dev_uuid;
    if (0 == dev_uuid.compare(net_dev_uuid)) {
      bool b_ret = CListenMessage::Instance()->SetDevInfo(jbody);
      if (b_ret) {
        j_resp[MSG_STATE] = 0;
      } else {
        j_resp[MSG_STATE] = 6;
      }

      sjson = j_resp.toStyledString();

      NET_CONFIG net_cfg;
      mcast_sock_->SendUdpData(DEF_MCAST_IP, DEF_MCAST_CLI_PORT,
                               sjson.c_str(), sjson.size(),
                               dev_net_.ip_addr.c_str());
    }
  } else {
    LOG(L_ERROR) << "this message type is no function to process." << s_type;
  }

  return 0;
}

void CNetwork::BcastDevInfo() {
  Json::Value jret;
  jret[MSG_CMD] = MSG_GET_DEVINFO;

  Json::Value jbody;
  bool b_ret = CListenMessage::Instance()->GetSystemAndNetwork(jbody);
  if (b_ret) {
    jret[MSG_STATE] = 0;
  } else {
    jret[MSG_STATE] = 6;
  }
  jret[MSG_BODY] = jbody;

  std::string sjson = jret.toStyledString();
  mcast_sock_->SendUdpData(DEF_MCAST_IP, DEF_MCAST_CLI_PORT,
                           sjson.c_str(), sjson.size());
}


bool CNetwork::OnGetNet(Json::Value &jbody) {
  try {
    NET_CONFIG net_cfg = {0};
#ifdef _LINUX
    net_get_info(nickname_.c_str(), &net_cfg);
#endif
    jbody["ip_addr"] = inet_ntoa(*((struct in_addr*)&net_cfg.ifaddr));
    jbody["netmask"] = inet_ntoa(*((struct in_addr*)&net_cfg.netmask));
    jbody["gateway"] = inet_ntoa(*((struct in_addr*)&net_cfg.gateway));
    char smac[20] = {0};
    snprintf(smac, 31, "%02x:%02x:%02x:%02x:%02x:%02x\n",
             net_cfg.mac[0], net_cfg.mac[1], net_cfg.mac[2],
             net_cfg.mac[3], net_cfg.mac[4], net_cfg.mac[5]);
    jbody["phy_mac"] = smac;

    jbody["dns_addr"] = inet_ntoa(*((struct in_addr*)&net_cfg.dns));

    jbody["http_port"] = dev_net_.http_port;
    return true;
  } catch (...) {
  }
  return false;
}

bool CNetwork::OnSetNet(const Json::Value &jbody) {
  try {
    unsigned int nsave = 0;
    if (dev_net_.wifi_en != jbody["wifi_en"].asInt()) {
      nsave++;
      dev_net_.wifi_en = jbody["wifi_en"].asInt();
    }

    if (dev_net_.dhcp_en != jbody["dhcp_en"].asInt()) {
      nsave++;
      dev_net_.dhcp_en = jbody["dhcp_en"].asInt();
    }
    LOG(L_INFO) << "dhcp enable " << dev_net_.dhcp_en;

    int res = 0;
    if (dev_net_.ip_addr != jbody["ip_addr"].asString()) {
      nsave++;
      res = SetIp(inet_addr(jbody["ip_addr"].asCString()));
    }

    if (dev_net_.netmask != jbody["netmask"].asString()) {
      nsave++;
      res = SetNetmask(inet_addr(jbody["netmask"].asCString()));
    }

    if (dev_net_.gateway != jbody["gateway"].asString()) {
      nsave++;
      res = SetGateway(inet_addr(jbody["gateway"].asCString()));
    }

    if (dev_net_.dns_addr != jbody["dns_addr"].asString()) {
      nsave++;
      res = SetDNS(inet_addr(jbody["dns_addr"].asCString()));
    }

    if (nsave > 0) {
      Kvdb_SetKey(KVDB_NETWORK, strlen(KVDB_NETWORK),
                  (char*)&dev_net_, sizeof(dev_net_));
    }
    return true;
  } catch (...) {
  }
  return false;
}

bool CNetwork::SysBngSetNet() {
  Kvdb_GetKeyToBuffer(KVDB_NETWORK, strlen(KVDB_NETWORK),
                      (char*)&dev_net_, sizeof(dev_net_));
  if (0 == dev_net_.dhcp_en) {
    nickname_ = PHY_ETH_NAME;
    system("killall udhcpc");

    SetIp(inet_addr(dev_net_.ip_addr.c_str()));
    SetNetmask(inet_addr(dev_net_.netmask.c_str()));
    SetGateway(inet_addr(dev_net_.gateway.c_str()));
    SetDNS(inet_addr(dev_net_.dns_addr.c_str()));
  } else {
    if (1 == dev_net_.wifi_en) {    // WIFI      
      nickname_ = PHY_WIFI_NAME;
    } else {                        // EHT0      
      nickname_ = PHY_ETH_NAME;
    }
  }
}

bool CNetwork::OnCheckNetAddr() {
  NET_CONFIG net_cfg = {0};
#ifdef _LINUX
  net_get_info(nickname_.c_str(), &net_cfg);
#endif
  if (0 == net_cfg.ifaddr || 0xffffffff == net_cfg.ifaddr) {

    return false;
  }

  dev_net_.ip_addr = inet_ntoa(*((struct in_addr*)&net_cfg.ifaddr));
  dev_net_.gateway = inet_ntoa(*((struct in_addr*)&net_cfg.gateway));
  dev_net_.netmask = inet_ntoa(*((struct in_addr*)&net_cfg.netmask));
  char smac[20] = {0};
  snprintf(smac, 19, "%02x:%02x:%02x:%02x:%02x:%02x\n",
           net_cfg.mac[0], net_cfg.mac[1], net_cfg.mac[2],
           net_cfg.mac[3], net_cfg.mac[4], net_cfg.mac[5]);
  dev_net_.phy_mac = smac;
  return true;
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
        dev_net_.ip_addr = inet_ntoa(*((struct in_addr*)&ip));
        ret = 0;
      }
    } else {
      ret = 0;
    }
  }
#else
  dev_net_.ip_addr = inet_ntoa(*((struct in_addr*)&ip));
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
        (snetmask != dev_net_.netmask)) {
      dev_net_.netmask = snetmask;
      \
      ret = 0;
    } else {
      ret = 0;
    }
  }
#else
  dev_net_.netmask = snetmask;
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
      dev_net_.gateway = inet_ntoa(*((struct in_addr*)&ipaddr));
      ret = 0;
    }
  } else {
    ret = 0;
  }
#else
  dev_net_.gateway = inet_ntoa(*((struct in_addr*)&ipaddr));
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
        dev_net_.dns_addr = sdns;
        ret = 0;
      }
    } else {
      ret = 0;
    }
  }
#else
  dev_net_.dns_addr = sdns;
  ret = 0;
#endif
  return ret;
}


}  // namespace sys
