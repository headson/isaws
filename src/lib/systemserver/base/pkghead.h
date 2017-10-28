/************************************************************************
*Author      : Sober.Peng 17-07-12
*Description :
************************************************************************/
#ifndef LIBSYSTEMSERVER_PKGHEAD_H
#define LIBSYSTEMSERVER_PKGHEAD_H

#include "vzbase/base/basictypes.h"

#include <string>

typedef struct TAG_DEV_NET {
  unsigned int  wifi_en;      // wifiʹ��
  unsigned int  dhcp_en;      // DHCPʹ��

  std::string   ip_addr;      // inet_addr() inet_ntoa
  std::string   netmask;      // ����
  std::string   gateway;      // ����
  std::string   phy_mac;      // 12:23:34:45:56:67

  std::string   dns_addr;     // DNS��ַ

  unsigned int  http_port;    // HTTP�˿�
  TAG_DEV_NET() {
    wifi_en = 0;
    dhcp_en = 0;
    ip_addr = "";
    netmask = "";
    gateway = "";
    phy_mac = "";
    dns_addr= "";
    http_port=80;
  }
} TAG_DEV_NET;

typedef struct TAG_SYS_INFO {
  std::string   dev_name;       // �豸���� utf-8ת���base64����
  unsigned int  dev_type;       // �豸����
  std::string   dev_uuid;       // �豸����

  std::string   ins_addr;       // ��װ��ַ utf-8ת���base64����

  std::string   sw_version;     // ����汾
  std::string   hw_version;     // Ӳ���汾
  std::string   alg_version;    // �㷨�汾
  TAG_SYS_INFO() {
    dev_name = "";
    dev_uuid = "";
    ins_addr = "";
    sw_version = "";
    hw_version = "";
    alg_version= "";
  }
} TAG_SYS_INFO;

#endif  // LIBSYSTEMSERVER_PKGHEAD_H
