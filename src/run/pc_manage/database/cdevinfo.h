/************************************************************************
*Author      : Sober.Peng 17-07-13
*Description :
************************************************************************/
#ifndef PC_MANAGE_CDEVINFO_H
#define PC_MANAGE_CDEVINFO_H

#include <QObject>
#include <QString>
#include <QPixmap>

#include "json/json.h"

class CDevInfo {
 public:
  CDevInfo() {
    id_             = "";
  }

  CDevInfo(const CDevInfo& cDev) {
    id_   = cDev.id_;
    ip_   = cDev.ip_;
    port_ = cDev.port_;
    name_ = cDev.name_;
    json_ = cDev.json_;
  }

  CDevInfo operator=(const CDevInfo& cDev) {
    id_   = cDev.id_;
    ip_   = cDev.ip_;
    port_ = cDev.port_;
    name_ = cDev.name_;
    json_ = cDev.json_;
    return *this;
  }

  bool operator==(const CDevInfo& cDev) {
    if (id_ == cDev.id_ &&
        ip_ == cDev.ip_ &&
        port_ == cDev.port_ &&
        name_ == cDev.name_) {
      return true;
    }
    return false;
  }

  std::string GetUrl() {
    char surl[128] = {0};
    sprintf(surl, "http://%s:%d/dispatch",
             ip_.c_str(), port_);
    return surl;
  }

 public:
  std::string   id_;                // …Ë±∏ID
  std::string   ip_;
  int           port_;
  std::string   name_;
  Json::Value   json_;
};


#endif  // PC_MANAGE_CDEVINFO_H
