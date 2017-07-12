/* ===========================================================================
* @path $(IPNCPATH)\sys_adm\system_server
* Copyright (c) Appro Photoelectron Inc.  2008
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied
* =========================================================================== */
#ifndef __NET_CONFIG_H_
#define __NET_CONFIG_H_

#ifdef _LINUX
#include <paths.h>

#define PHY_IF_NAME            "eth0"

#define RESOLV_CONF           "/etc/resolv.conf"
#define PROCNET_ROUTE_PATH    "/proc/net/route"
/**
* @brief Structure to save network status
*/
#ifdef __cplusplus
extern "C" {
#endif

struct NET_CONFIG {
  unsigned char   mac[6];
  in_addr_t       ifaddr;
  in_addr_t       netmask;
  in_addr_t       gateway;
  in_addr_t       dns;
};

//int       net_set_flag(const char *ifname, short flag);
int         net_nic_up(const char *ifname);
//int       net_clr_flag(const char *ifname, short flag);
int         net_get_flag(const char *ifname);
int         net_nic_down(const char *ifname);
in_addr_t   net_get_ifaddr(const char *ifname);
int         net_set_ifaddr(const char *ifname, in_addr_t addr);
in_addr_t   net_get_netmask(const char *ifname);
int         net_set_netmask(const char *ifname, in_addr_t addr);
int         net_get_hwaddr(const char *ifname, unsigned char *mac);
in_addr_t   net_get_gateway(void);
in_addr_t   net_get_gateway_byname(char *ifname);
in_addr_t   net_search_gateway_byname(char *ifname);
int         net_set_gateway_byname(char *ifname,in_addr_t addr);
#ifdef WIFI_LAN
int         net_set_route_byname(char *ifname,in_addr_t gateaddr,in_addr_t ipaddr,in_addr_t maskaddr);
int         net_clean_gateway_byname(char *ifname);
#endif
int         net_set_gateway(in_addr_t addr);
int         net_clean_gateway(void);
//int       net_add_gateway(in_addr_t addr);
//int       net_del_gateway(in_addr_t addr);
in_addr_t   net_get_dns(void);
int         net_set_dns(const char *dnsname);
int         net_get_info(const char *ifname, struct NET_CONFIG *netcfg);

struct in_addr net_get_ip(int skfd, const char *ifname);

#ifdef __cplusplus
}
#endif

#endif  // _LINUX

#endif // __NET_CONFIG_H_
