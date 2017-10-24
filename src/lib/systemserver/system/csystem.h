/************************************************************************
* Author      : SoberPeng 2017-10-21
* Description :
************************************************************************/
#ifndef LIBSYSTEMSERVER_CSYSTEM_H
#define LIBSYSTEMSERVER_CSYSTEM_H

#include "dispatcher/base/pkghead.h"

#include <json/json.h>

#include "vzbase/base/boost_settings.hpp"
#include "systemserver/base/basedefines.h"

namespace sys {
class CSystem : public vzbase::noncopyable {
 protected:
  CSystem();
 public:
  ~CSystem();

 public:
  static CSystem *Create();

  bool Start();
  void Stop();

  bool OnDpMessage(const DpMessage *dmp,
                   const Json::Value &jreq, Json::Value &jret);

 public:
  bool DefSysInfo();

  bool InitAlgInfo();
  bool OnGetSysInfo(Json::Value &jinfo);
  bool OnSetSysInfo(const Json::Value &jinfo);

 private:
  TAG_DEV_INFO dev_info_;
};
}

#endif  // LIBSYSTEMSERVER_CSYSTEM_H
