/************************************************************************
*Author      : Sober.Peng 17-07-12
*Description :
************************************************************************/
#ifndef LIBSYSTEMSERVER_PKGHEAD_H
#define LIBSYSTEMSERVER_PKGHEAD_H

#include "vzbase/base/basictypes.h"

#include <string>

typedef struct TAG_DEV_NET{
  unsigned int  wifi_en;      // wifi使能

  unsigned int  dhcp_en;      // DHCP使能

  std::string   ip_addr;      // inet_addr() inet_ntoa
  std::string   netmask;      // 掩码
  std::string   gateway;      // 网关
  std::string   phy_mac;      // 12:23:34:45:56:67

  std::string   dns_addr;     // DNS地址

  unsigned int  http_port;    // HTTP端口
  unsigned int  rtsp_port;    // RTSP端口
} TAG_DEV_NET;

typedef struct _TAG_SYS_INFO {
  std::string   dev_name;       // 设备名称 utf-8转码后base64编码
  unsigned int  dev_type;       // 设备类型
  std::string   dev_uuid;       // 设备串号

  std::string   ins_addr;       // 安装地址 utf-8转码后base64编码

  std::string   sw_version;     // 软件版本
  std::string   hw_version;     // 硬件版本
  std::string   alg_version;    // 算法版本

  TAG_DEV_NET   net;

  unsigned int  save_size;      // 存储大小
} TAG_SYS_INFO;

#endif  // LIBSYSTEMSERVER_PKGHEAD_H
