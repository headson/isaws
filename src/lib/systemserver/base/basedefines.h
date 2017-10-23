/************************************************************************
*Author      : Sober.Peng 17-07-12
*Description :
************************************************************************/
#ifndef LIBSYSTEMSERVER_PKGHEAD_H
#define LIBSYSTEMSERVER_PKGHEAD_H

#include "vzbase/base/basictypes.h"

#include <string>

typedef struct TAG_DEV_INFO {
  std::string dev_name;     // �豸���� utf-8ת���base64����
  std::string dev_uuid;     // �豸����

  std::string ins_addr;     // ��װ��ַ utf-8ת���base64����

  std::string sw_ver;       // ����汾
  std::string hw_ver;       // Ӳ���汾
  std::string alg_ver;      // �㷨�汾
} TAG_DEV_INFO;

typedef struct TAG_DEV_NET {
  int  wifi_en;                 // WIFIʹ��
  int  dhcp_en;                 // DHCPʹ��

  std::string ip_addr;   // inet_addr() inet_ntoa
  std::string netmask;   // ����
  std::string gateway;   // ����
  std::string phy_mac;   // 12:23:34:45:56:67
  std::string dns_addr;  // DNS��ַ

  int  http_port;               // HTTP�˿�
} TAG_DEV_NET;

typedef struct TAG_DEV_TIME {
  int     timezone;             // ʱ��
  struct {
    int           enable;                 // ntpʹ��
    std::string   server;  // ntp������
    int           timeout;                // ntp��ʱms
  } ntp;  
} TAG_DEV_TIME;

#endif  // LIBSYSTEMSERVER_PKGHEAD_H
