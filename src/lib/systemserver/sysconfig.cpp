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

//////////////////////////////////////////////////////////////////////////
void CListenMessage::GetAlgVer() {
  if (sys_info_.alg_version.empty()) {
    std::string sreq = "{\"cmd\" : \"get_alginfo\" }";
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
  std::string sjson;
  Kvdb_GetKeyToString(KVDB_HW_INFO, strlen(KVDB_HW_INFO),
                      &sjson);

  Json::Value  jhw;
  Json::Reader jread;
  if (sjson.empty() ||
      !jread.parse(sjson, jhw)) {
    LOG(L_ERROR) << "system json parse failed. create the default config";

    // 生成默认参数
    jhw["dev_name"] = "BVS_001";
    jhw["ins_addr"] = "";
    SetDevInfo(jhw);
  }

  sys_info_.dev_name = jhw["dev_name"].asString();
  sys_info_.ins_addr = jhw["ins_addr"].asString();

  // version
  // GetAlgVer();

  // software
  vzbase::get_software(sys_info_.sw_version);

  // hardware/uuid
  vzbase::get_hardware(sys_info_.hw_version,
                       sys_info_.dev_type,
                       sys_info_.dev_uuid);
}

bool CListenMessage::GetDevInfo(Json::Value &jret) {
  jret["dev_uuid"] = sys_info_.dev_uuid;
  jret["dev_type"] = sys_info_.dev_type;
  jret["dev_name"] = sys_info_.dev_name;
  jret["ins_addr"] = sys_info_.ins_addr;

  jret["sw_version"] = sys_info_.sw_version;
  jret["hw_version"] = sys_info_.hw_version;
  GetAlgVer();  // if alg_version is empty
  jret["alg_version"] = sys_info_.alg_version;

  Json::Value jnet;
  if (network_ &&
      network_->GetNet(jnet)) {
    jret["net"] = jnet;
  }

  // 系统设备状态
  jret["sys_dev_sta"] = 0;

  return true;
}

bool CListenMessage::SetDevInfo(const Json::Value &jreq) {
  int bsave = 0;

  if (jreq.isMember("dev_name") &&
      jreq["dev_name"].isString() &&
      sys_info_.dev_name != jreq["dev_name"].asString()) {
    bsave++;
    sys_info_.dev_name = jreq["dev_name"].asString();
  }

  if (jreq.isMember("ins_addr") &&
      jreq["ins_addr"].isString() &&
      sys_info_.ins_addr != jreq["ins_addr"].asString()) {
    bsave++;
    sys_info_.ins_addr  = jreq["ins_addr"].asString();
  }

  if (bsave == 0) {
    LOG(L_INFO) << "don't save config file.";
    return true;
  }

  Json::FastWriter jfw;
  std::string ss = jfw.write(jreq);
  Kvdb_SetKey(KVDB_HW_INFO,
              strlen(KVDB_HW_INFO),
              ss.c_str(), ss.size());
  return true;
}

}