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

#include "vzbase/base/mysystem.h"
#include "systemserver/module/cmodulecontrol.h"

namespace sys {

#ifdef _WIN32
#define SYS_CFG_PATH  "./system.json"
#else  // _LINUX
#include "systemserver/network/net_cfg.h"

#define SYS_CFG_PATH  "/mnt/etc/system.json"
#endif

void CListenMessage::GetHwInfo() {
  Json::Reader jread;
  Json::Value  jinfo;
  std::ifstream ifs;
  ifs.open(SYS_CFG_PATH);
  if (!ifs.is_open() ||
      !jread.parse(ifs, jinfo)) {
    LOG(L_ERROR) << "system json parse failed. create the default config";

    // 生成默认参数
    jinfo["dev_name"] = "PC_001";
    jinfo["dev_type"] = 100100;
    jinfo["ins_addr"] = "";
    jinfo["sw_version"] = "1.0.0.1001707310";
    jinfo["hw_version"] = "1.0.0.1001707310";
    jinfo["alg_version"] = "1.0.0.1001707310";
    jinfo["net"]["wifi_en"] = 0;
    jinfo["net"]["dhcp_en"] = 0;
    jinfo["net"]["ip_addr"] = "192.168.1.100";
    jinfo["net"]["netmask"] = "255.255.255.0";
    jinfo["net"]["gateway"] = "192.168.1.1";
    jinfo["net"]["phy_mac"] = "01:12:23:34:45:67";
    jinfo["net"]["dns_addr"] = "192.168.1.1";
    jinfo["net"]["http_port"] = 80;
    jinfo["net"]["rtsp_port"] = 554;
  }

  sys_info_.dev_name = jinfo["dev_name"].asString();
  sys_info_.dev_type = jinfo["dev_type"].asInt();
  sys_info_.ins_addr = jinfo["ins_addr"].asString();
  // sys_info_.sw_version
  // sys_info_.hw_version
  // sys_info_.alg_version
  sys_info_.net.wifi_en = jinfo["net"]["wifi_en"].asInt();
  sys_info_.net.dhcp_en = jinfo["net"]["dhcp_en"].asInt();
  sys_info_.net.ip_addr = jinfo["net"]["ip_addr"].asString();
  sys_info_.net.netmask = jinfo["net"]["netmask"].asString();
  sys_info_.net.gateway = jinfo["net"]["gateway"].asString();
  // sys_info_.net.phy_mac = jinfo["net"]["phy_mac"].asString();
  sys_info_.net.dns_addr = jinfo["net"]["dns_addr"].asString();
  sys_info_.net.http_port = jinfo["net"]["http_port"].asInt();
  sys_info_.net.rtsp_port = jinfo["net"]["rtsp_port"].asInt();

  // dp获取算法信息
  std::string sresp = "";
  DpClient_SendDpReqToString(MSG_GET_IVAINFO, 0,
                             NULL, 0, &sresp,
                             DEF_TIMEOUT_MSEC);
  Json::Value jresp;
  if (!jread.parse(sresp, jresp)) {
    LOG(L_ERROR) << "MSG_GET_IVAINFO failed.";
    return;
  }
  if (jresp.isMember("version")) {
    jinfo["alg_version"] = jresp["version"].asString();
  }

  // software
  vzbase::get_software(sys_info_.sw_version);

  // hardware/uuid
  vzbase::get_hardware(sys_info_.hw_version,
                       sys_info_.dev_uuid);

  // save size

}

bool CListenMessage::GetDevInfo(Json::Value &jbody) {
  jbody["dev_name"] = sys_info_.dev_name;
  jbody["dev_type"] = sys_info_.dev_type;
  
  jbody["ins_addr"] = sys_info_.ins_addr;
  
  jbody["sw_version"] = sys_info_.sw_version;
  jbody["hw_version"] = sys_info_.hw_version;
  jbody["alg_version"] = sys_info_.alg_version;
  
  jbody["net"]["wifi_en"] = sys_info_.net.wifi_en;
  jbody["net"]["dhcp_en"] = sys_info_.net.dhcp_en;
  
  jbody["net"]["ip_addr"] = inet_ntoa(*((struct in_addr*)&CNetCtrl::ip_addr_));
  jbody["net"]["netmask"] = inet_ntoa(*((struct in_addr*)&CNetCtrl::netmask_));
  jbody["net"]["gateway"] = inet_ntoa(*((struct in_addr*)&CNetCtrl::gateway_));
  jbody["net"]["phy_mac"]  = CNetCtrl::phy_mac_;

  jbody["net"]["dns_addr"] = inet_ntoa(*((struct in_addr*)&CNetCtrl::dns_addr_));

  jbody["net"]["http_port"] = sys_info_.net.http_port;
  jbody["net"]["rtsp_port"] = sys_info_.net.rtsp_port;
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

  // sys_info_.dev_type
  // sys_info_.dev_uuid

  if (jbody.isMember("ins_addr") &&
      jbody["ins_addr"].isString() &&
      sys_info_.ins_addr != jbody["ins_addr"].asString()) {
    bsave++;
    sys_info_.ins_addr  = jbody["ins_addr"].asString();
  }

  // sys_info_.sw_version
  // sys_info_.hw_version
  // sys_info_.alg_version

  //////////////////////////////////////////////////////////////////////////
  if (!jbody.isMember("net")) {
    LOG(L_ERROR) << "the param's format is error.";
    return false;
  }

  if (jbody["net"].isMember("wifi_en") &&
      jbody["net"]["wifi_en"].isUInt() &&
      sys_info_.net.wifi_en != jbody["net"]["wifi_en"].asUInt()) {
    bsave++;
    sys_info_.net.wifi_en = jbody["net"]["wifi_en"].asUInt();
  }

  if (jbody["net"].isMember("dhcp_en") &&
      jbody["net"]["dhcp_en"].isInt() &&
      sys_info_.net.dhcp_en != jbody["net"]["dhcp_en"].asInt()) {
    bsave++;
    sys_info_.net.dhcp_en = jbody["net"]["dhcp_en"].asInt();
  }
  LOG(L_INFO) << "dhcp enable " << sys_info_.net.dhcp_en;

  if (jbody["net"].isMember("ip_addr") &&
      jbody["net"]["ip_addr"].isString() &&
      sys_info_.net.ip_addr != jbody["net"]["ip_addr"].asString()) {
    bsave++;
    sys_info_.net.ip_addr = jbody["net"]["ip_addr"].asString();
  }

  if (jbody["net"].isMember("netmask") &&
      jbody["net"]["netmask"].isString() &&
      sys_info_.net.netmask != jbody["net"]["netmask"].asString()) {
    bsave++;
    sys_info_.net.netmask = jbody["net"]["netmask"].asString();
  }

  if (jbody["net"].isMember("gateway") &&
      jbody["net"]["gateway"].isString() &&
      sys_info_.net.gateway != jbody["net"]["gateway"].asString()) {
    bsave++;
    sys_info_.net.gateway = jbody["net"]["gateway"].asString();
  }

  // sys_info_.net.phy_mac

  if (jbody["net"].isMember("dns_addr") &&
      jbody["net"]["dns_addr"].isString() &&
      sys_info_.net.dns_addr != jbody["net"]["dns_addr"].asString()) {
    bsave++;
    sys_info_.net.dns_addr = jbody["net"]["dns_addr"].asString();
  }

  // sys_info_.net.http_port
  // sys_info_.net.rtsp_port

  //////////////////////////////////////////////////////////////////////////
  if (bsave == 0) {
    LOG(L_INFO) << "don't save config file.";
    return true;
  }

  FILE *file = fopen(SYS_CFG_PATH, "wt+");
  if (file) {
    Json::FastWriter jfw;
    std::string ss = jfw.write(jbody);

    fwrite(ss.c_str(), 1, ss.size(), file);
    fclose(file);
  }

  if (net_ctrl_->ModityNetwork(sys_info_)) {
    CModuleMonitor::ReStartModule();
  }
  return true;
}

}