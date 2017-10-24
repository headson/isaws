/************************************************************************
* Author      : SoberPeng 2017-10-22
* Description :
************************************************************************/
#include "systemserver/system/csystem.h"

#include "vzbase/helper/stdafx.h"

namespace sys {
sys::CSystem::CSystem() {
}

CSystem::~CSystem() {
}

CSystem *CSystem::Create() {
  CSystem *sys = new CSystem();
  if (NULL == sys) {
    LOG(L_ERROR) << "create CSystem failed.";
  }
  return sys;
}

bool CSystem::Start() {
  Json::Value jsys;
  Json::Reader jread;
  return true;
}

void CSystem::Stop() {
}

bool CSystem::OnDpMessage(const DpMessage *dmp,
                          const Json::Value &jreq, Json::Value &jret) {
  return true;
}

bool CSystem::DefSysInfo() {
  return true;
}

bool CSystem::InitAlgInfo() {
  return true;
}

bool CSystem::OnGetSysInfo(Json::Value &jinfo) {
  return true;
}

bool CSystem::OnSetSysInfo(const Json::Value &jinfo) {
  return true;
}

}
