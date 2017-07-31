/************************************************************************
*Author      : Sober.Peng 17-07-12
*Description :
************************************************************************/
#ifndef LIBSYSTEMSERVER_PKGHEAD_H
#define LIBSYSTEMSERVER_PKGHEAD_H

#include "vzbase/base/basictypes.h"

#include <string>

typedef struct _TAG_SYS_INFO {
  std::string   dev_name;       // �豸���� utf-8ת���base64����
  unsigned int  dev_type;       // �豸����
  std::string   dev_uuid;       // �豸����

  std::string   ins_addr;       // ��װ��ַ utf-8ת���base64����

  std::string   sw_version;     // ����汾
  std::string   hw_version;     // Ӳ���汾
  std::string   alg_version;    // �㷨�汾

  struct {
    unsigned int  wifi_en;      // wifiʹ��

    unsigned int  dhcp_en;      // DHCPʹ��
    in_addr_t     ip_addr;      // inet_addr() inet_ntoa
    in_addr_t     netmask;      // ����
    in_addr_t     gateway;      // ����
    std::string   phy_mac;      // 12:23:34:45:56:67

    in_addr_t     dns_addr;     // DNS��ַ

    unsigned int  http_port;    // HTTP�˿�
    unsigned int  rtsp_port;    // RTSP�˿�
  } net;

  unsigned int  save_size;      // �洢��С
} TAG_SYS_INFO;

#endif  // LIBSYSTEMSERVER_PKGHEAD_H
