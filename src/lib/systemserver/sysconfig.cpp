/************************************************************************
*Author      : Sober.Peng 17-07-31
*Description : 系统参数读,存
************************************************************************/
#include "systemserver/clistenmessage.h"

#include <fstream>
#include "json/json.h"

#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"

#include "vzbase/base/helper.h"
#include "vzbase/base/base64.h"

#include "vzbase/system/mysystem.h"

#include "systemserver/network/net_cfg.h"

namespace sys {

void CListenMessage::GetAlgVer() {
  if (sys_info_.alg_version.empty()) {
    std::string sreq = "{\"cmd\":\"get_alginfo\" }";
    // dp获取算法信息
    std::string sresp = "";
    DpClient_SendDpReqToString(MSG_GET_ALGINFO, 0,
                               sreq.c_str(), sreq.size(),
                               &sresp,
                               DEF_TIMEOUT_MSEC);
    LOG(L_INFO) << sresp;
    Json::Value jresp;
    Json::Reader jread;
    if (jread.parse(sresp, jresp)) {
      sys_info_.alg_version = jresp[MSG_BODY]["version"].asString();
    } else {
      sys_info_.alg_version = "";
      LOG(L_ERROR) << "MSG_GET_ALGINFO failed.";
    }
  }
}

void CListenMessage::GetHwInfo() {
  Json::Reader jread;
  Json::Value  jinfo;
  std::ifstream ifs;
  ifs.open(SYS_SYSTEM_CONFIG);
  if (!ifs.is_open() ||
      !jread.parse(ifs, jinfo)) {
    LOG(L_ERROR) << "system json parse failed. create the default config";

    // 生成默认参数
    jinfo["dev_name"] = "BVS_001";
    jinfo["dev_type"] = 1300;
    jinfo["ins_addr"] = "";

    jinfo["net"]["wifi_en"] = 0;
    jinfo["net"]["dhcp_en"] = 0;
    jinfo["net"]["ip_addr"] = "192.168.1.88";
    jinfo["net"]["netmask"] = "255.255.255.0";
    jinfo["net"]["gateway"] = "192.168.1.1";
    jinfo["net"]["dns_addr"] = "192.168.1.1";
    jinfo["net"]["http_port"] = 80;
  }

  sys_info_.dev_name = jinfo["dev_name"].asString();
  sys_info_.dev_type = jinfo["dev_type"].asInt();
  sys_info_.ins_addr = jinfo["ins_addr"].asString();
  sys_info_.net.wifi_en = jinfo["net"]["wifi_en"].asInt();
  sys_info_.net.dhcp_en = jinfo["net"]["dhcp_en"].asInt();
  // 注释,后续SetDevInfo会获取网络信息
  //sys_info_.net.ip_addr = jinfo["net"]["ip_addr"].asString();
  //sys_info_.net.netmask = jinfo["net"]["netmask"].asString();
  //sys_info_.net.gateway = jinfo["net"]["gateway"].asString();
  //sys_info_.net.dns_addr = jinfo["net"]["dns_addr"].asString();
  sys_info_.net.http_port = jinfo["net"]["http_port"].asInt();

  SetDevInfo(jinfo);

  // save to kvdb
  //std::string sjson = jinfo.toStyledString();
  //Kvdb_SetKey(KVDB_HW_INFO,
  //            strlen(KVDB_HW_INFO),
  //            sjson.c_str(), sjson.size());

  // version
  GetAlgVer();

  // software
  vzbase::get_software(sys_info_.sw_version);

  // hardware/uuid
  vzbase::get_hardware(sys_info_.hw_version, sys_info_.dev_uuid);
}

bool CListenMessage::GetDevInfo(Json::Value &jbody) {
  jbody["dev_uuid"] = sys_info_.dev_uuid;
  jbody["dev_name"] = sys_info_.dev_name;
  jbody["dev_type"] = sys_info_.dev_type;

  jbody["ins_addr"] = sys_info_.ins_addr;

  jbody["sw_version"] = sys_info_.sw_version;
  jbody["hw_version"] = sys_info_.hw_version;

  GetAlgVer();  // if alg_version is empty
  jbody["alg_version"] = sys_info_.alg_version;

  // 系统设备状态
  jbody["sys_dev_sta"] = 0;
  
  // net
  jbody["net"]["wifi_en"] = sys_info_.net.wifi_en;
  jbody["net"]["dhcp_en"] = sys_info_.net.dhcp_en;

  NET_CONFIG net_cfg = { 0 };
#ifdef _LINUX
  net_get_info(nickname_.c_str(), &net_cfg);
#endif
  jbody["net"]["ip_addr"] = inet_ntoa(*((struct in_addr*)&net_cfg.ifaddr));
  jbody["net"]["netmask"] = inet_ntoa(*((struct in_addr*)&net_cfg.netmask));
  jbody["net"]["gateway"] = inet_ntoa(*((struct in_addr*)&net_cfg.gateway));
  char smac[32] = {0};
  snprintf(smac, 31, "%02x:%02x:%02x:%02x:%02x:%02x\n", 
           net_cfg.mac[0], net_cfg.mac[1], net_cfg.mac[2], 
           net_cfg.mac[3], net_cfg.mac[4], net_cfg.mac[5]);
  jbody["net"]["phy_mac"] = smac;

  jbody["net"]["dns_addr"] = inet_ntoa(*((struct in_addr*)&net_cfg.dns));

  jbody["net"]["http_port"] = sys_info_.net.http_port;

  // record
  jbody["rec_size"] = 0;
  return true;
}

bool CListenMessage::SetDevInfo(const Json::Value &jbody) {
  int bsave = 0;

  if (jbody.isMember("dev_name") &&
      jbody["dev_name"].isString() &&
      sys_info_.dev_name != jbody["dev_name"].asString()) {
    bsave++;
    sys_info_.dev_name = jbody["dev_name"].asString();
  }

  if (jbody.isMember("ins_addr") &&
      jbody["ins_addr"].isString() &&
      sys_info_.ins_addr != jbody["ins_addr"].asString()) {
    bsave++;
    sys_info_.ins_addr  = jbody["ins_addr"].asString();
  }

  //////////////////////////////////////////////////////////////////////////
  if (!jbody.isMember("net")) {
    LOG(L_ERROR) << "the param's format is error.";
    return false;
  }

  if (jbody["net"].isMember("wifi_en") &&
      jbody["net"]["wifi_en"].isInt() &&
      sys_info_.net.wifi_en != jbody["net"]["wifi_en"].asInt()) {
    bsave++;
    sys_info_.net.wifi_en = jbody["net"]["wifi_en"].asInt();
  }

  if (jbody["net"].isMember("dhcp_en") &&
      jbody["net"]["dhcp_en"].isInt() &&
      sys_info_.net.dhcp_en != jbody["net"]["dhcp_en"].asInt()) {
    bsave++;
    sys_info_.net.dhcp_en = jbody["net"]["dhcp_en"].asInt();
  }
  LOG(L_INFO) << "dhcp enable " << sys_info_.net.dhcp_en;

  int res = 0;
  if (jbody["net"].isMember("ip_addr") &&
      jbody["net"]["ip_addr"].isString() &&
      sys_info_.net.ip_addr != jbody["net"]["ip_addr"].asString()) {
    bsave++;

    res = SetIp(inet_addr(jbody["net"]["ip_addr"].asCString()));
  }

  if (jbody["net"].isMember("netmask") &&
      jbody["net"]["netmask"].isString() &&
      sys_info_.net.netmask != jbody["net"]["netmask"].asString()) {
    bsave++;

    res = SetNetmask(inet_addr(jbody["net"]["netmask"].asCString()));
  }

  if (jbody["net"].isMember("gateway") &&
      jbody["net"]["gateway"].isString() &&
      sys_info_.net.gateway != jbody["net"]["gateway"].asString()) {
    bsave++;

    res = SetGateway(inet_addr(jbody["net"]["gateway"].asCString()));
  }

  if (jbody["net"].isMember("dns_addr") &&
      jbody["net"]["dns_addr"].isString() &&
      sys_info_.net.dns_addr != jbody["net"]["dns_addr"].asString()) {
    bsave++;
    res = SetDNS(inet_addr(jbody["net"]["dns_addr"].asCString()));
  }

  //////////////////////////////////////////////////////////////////////////
  if (bsave == 0) {
    LOG(L_INFO) << "don't save config file.";
    return true;
  }

  FILE *file = fopen(SYS_SYSTEM_CONFIG, "wt+");
  if (file) {
    std::string ss = jbody.toStyledString();
    Kvdb_SetKey(KVDB_HW_INFO,
                strlen(KVDB_HW_INFO),
                ss.c_str(), ss.size());

    fwrite(ss.c_str(), 1, ss.size(), file);
    fclose(file);
    return true;
  }
  return false;
}

}