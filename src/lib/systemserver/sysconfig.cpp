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

namespace sys {

void CListenMessage::GetHwInfo() {
  Json::Reader jread;

  std::ifstream ifs;
#ifdef _WIN32
  ifs.open("./system.json");
#else
  ifs.open("/mnt/etc/system.json");
#endif
  if (!ifs.is_open() ||
      !jread.parse(ifs, hw_json_)) {
    LOG(L_ERROR) << "system json parse failed. create the default config";

    // 生成默认参数
    hw_json_["dev_name"] = "PC_001";
    hw_json_["dev_type"] = 100100;
    hw_json_["dev_uuid"] = "PC000120170731184822";
    hw_json_["ins_addr"] = "";
    hw_json_["sw_version"] = "1.0.0.1001707310";
    hw_json_["hw_version"] = "1.0.0.1001707310";
    hw_json_["alg_version"] = "1.0.0.1001707310";
    hw_json_["net"]["wifi_en"] = 0;
    hw_json_["net"]["dhcp_en"] = 0;
    hw_json_["net"]["ip_addr"] = "192.168.1.100";
    hw_json_["net"]["netmask"] = "255.255.255.0";
    hw_json_["net"]["gateway"] = "192.168.1.1";
    hw_json_["net"]["phy_mac"] = "01:12:23:34:45:67";
    hw_json_["net"]["dns_addr"] = "192.168.1.1";
    hw_json_["net"]["http_port"] = 80;
    hw_json_["net"]["rtsp_port"] = 554;
  }

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
    hw_json_["alg_version"] = jresp["version"].asString();
  }

  // dev uuid\mac

  // save size

}

bool CListenMessage::GetDevInfo(Json::Value &jbody) {
  jbody = hw_json_;
  return true;
}

bool CListenMessage::SetDevInfo(const Json::Value &jbody) {
  int bsave = 0;

  if (jbody.isMember("dev_name") &&
      jbody["dev_name"].isString() &&
      hw_json_["dev_name"].asString() != jbody["dev_name"].asString()) {
    bsave++;
    hw_json_["dev_name"] = jbody["dev_name"].asString();
  }

  //if (jbody.isMember("dev_type") &&
  //    jbody["dev_type"].isInt() &&
  //    hw_json_["dev_type"].asInt() != jbody["dev_type"].asInt()) {
  //  bsave++;
  //  hw_json_["dev_type"] = jbody["dev_type"].asInt();
  //}

  //if (jbody.isMember("dev_uuid") &&
  //    jbody["dev_uuid"].isString() &&
  //    hw_json_["dev_uuid"].asString() != jbody["dev_uuid"].asString()) {
  //  bsave++;
  //  hw_json_["dev_uuid"] = jbody["dev_uuid"].asString();
  //}

  if (jbody.isMember("ins_addr") &&
      jbody["ins_addr"].isString() &&
      hw_json_["ins_addr"].asString() != jbody["ins_addr"].asString()) {
    bsave++;
    hw_json_["ins_addr"] = jbody["ins_addr"].asString();
  }

  //if (jbody.isMember("sw_version") &&
  //    jbody["sw_version"].isString() &&
  //    hw_json_["sw_version"].asString() != jbody["sw_version"].asString()) {
  //  bsave++;
  //  hw_json_["sw_version"] = jbody["sw_version"].asString();
  //}

  //if (jbody.isMember("hw_version") &&
  //    jbody["hw_version"].isString() &&
  //    hw_json_["hw_version"].asString() != jbody["hw_version"].asString()) {
  //  bsave++;
  //  hw_json_["hw_version"] = jbody["hw_version"].asString();
  //}

  //if (jbody.isMember("alg_version") &&
  //    jbody["alg_version"].isString() &&
  //    hw_json_["alg_version"].asString() != jbody["alg_version"].asString()) {
  //  bsave++;
  //  hw_json_["alg_version"] = jbody["alg_version"].asString();
  //}

  //////////////////////////////////////////////////////////////////////////
  if (!jbody.isMember("net")) {
    LOG(L_ERROR) << "the param's format is error.";
    return false;
  }

  if (jbody["net"].isMember("wifi_en") &&
      jbody["net"]["wifi_en"].isUInt() &&
      hw_json_["net"]["wifi_en"].asUInt() != jbody["net"]["wifi_en"].asUInt()) {
    bsave++;
    hw_json_["net"]["wifi_en"] = jbody["net"]["wifi_en"].asUInt();
  }

  if (jbody["net"].isMember("dhcp_en") &&
      jbody["net"]["dhcp_en"].isUInt() &&
      hw_json_["net"]["dhcp_en"].asUInt() != jbody["net"]["dhcp_en"].asUInt()) {
    bsave++;
    hw_json_["net"]["dhcp_en"] = jbody["net"]["dhcp_en"].asUInt();
  }

  if (jbody["net"].isMember("ip_addr") &&
      jbody["net"]["ip_addr"].isUInt() &&
      hw_json_["net"]["ip_addr"].asUInt() != jbody["net"]["ip_addr"].asUInt()) {
    bsave++;
    hw_json_["net"]["ip_addr"] = jbody["net"]["ip_addr"].asString();
  }

  if (jbody["net"].isMember("netmask") &&
      jbody["net"]["netmask"].isUInt() &&
      hw_json_["net"]["netmask"].asUInt() != jbody["net"]["netmask"].asUInt()) {
    bsave++;
    hw_json_["net"]["netmask"] = jbody["net"]["netmask"].asString();
  }

  if (jbody["net"].isMember("gateway") &&
      jbody["net"]["gateway"].isUInt() &&
      hw_json_["net"]["gateway"].asUInt() != jbody["net"]["gateway"].asUInt()) {
    bsave++;
    hw_json_["net"]["gateway"] = jbody["net"]["gateway"].asString();
  }

  //if (jbody["net"].isMember("phy_mac") &&
  //    jbody["net"]["phy_mac"].isString() &&
  //    hw_json_["net"]["phy_mac"].asString() != jbody["net"]["phy_mac"].asString()) {
  //  bsave++;
  //  hw_json_["net"]["phy_mac"] = jbody["net"]["phy_mac"].asString();
  //}

  if (jbody["net"].isMember("dns_addr") &&
      jbody["net"]["dns_addr"].isUInt() &&
      hw_json_["net"]["dns_addr"].asUInt() != jbody["net"]["dns_addr"].asUInt()) {
    bsave++;
    hw_json_["net"]["dns_addr"] = jbody["net"]["dns_addr"].asString();
  }

  if (jbody["net"].isMember("http_port") &&
      jbody["net"]["http_port"].isInt() &&
      hw_json_["net"]["http_port"].asInt() != jbody["net"]["http_port"].asInt()) {
    bsave++;
    hw_json_["net"]["http_port"] = jbody["net"]["http_port"].asInt();
  }

  if (jbody["net"].isMember("rtsp_port") &&
      jbody["net"]["rtsp_port"].isInt() &&
      hw_json_["net"]["rtsp_port"].asInt() != jbody["net"]["rtsp_port"].asInt()) {
    bsave++;
    hw_json_["net"]["rtsp_port"] = jbody["net"]["rtsp_port"].asInt();
  }

  //////////////////////////////////////////////////////////////////////////
  if (bsave == 0) {
    return true;
  }

  FILE *file = fopen("./system.json", "wt+");
  if (file) {
    std::string ss = hw_json_.toStyledString();

    fwrite(ss.c_str(), 1, ss.size(), file);
    fclose(file);
  }
  net_ctrl_->ModityNetwork(hw_json_["net"]);
  return true;
}

}