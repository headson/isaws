/************************************************************************
*Author      : Sober.Peng 17-07-12
*Description :
************************************************************************/
#ifndef LIBSYSTEMSERVER_PKGHEAD_H
#define LIBSYSTEMSERVER_PKGHEAD_H

#include "vzbase/base/basictypes.h"

#include <string>

typedef struct TAG_DEV_INFO {
  std::string dev_name;     // 设备名称 utf-8转码后base64编码
  std::string dev_uuid;     // 设备串号

  std::string ins_addr;     // 安装地址 utf-8转码后base64编码

  std::string sw_ver;       // 软件版本
  std::string hw_ver;       // 硬件版本
  std::string alg_ver;      // 算法版本
} TAG_DEV_INFO;

typedef struct TAG_DEV_NET {
  int  wifi_en;                 // WIFI使能
  int  dhcp_en;                 // DHCP使能

  std::string ip_addr;   // inet_addr() inet_ntoa
  std::string netmask;   // 掩码
  std::string gateway;   // 网关
  std::string phy_mac;   // 12:23:34:45:56:67
  std::string dns_addr;  // DNS地址

  int  http_port;               // HTTP端口
} TAG_DEV_NET;

typedef struct TAG_DEV_TIME {
  int     timezone;             // 时区
  struct {
    int           enable;                 // ntp使能
    std::string   server;  // ntp服务器
    int           timeout;                // ntp超时ms
  } ntp;  
} TAG_DEV_TIME;

#endif  // LIBSYSTEMSERVER_PKGHEAD_H
