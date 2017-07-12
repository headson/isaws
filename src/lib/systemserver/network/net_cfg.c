/** ===========================================================================
* @file net_config.c
* @path $(IPNCPATH)\sys_adm\system_server
* Copyright (c) Appro Photoelectron Inc.  2008
* Use of this software is controlled by the terms and conditions found
* in the license agreement under which this software has been supplied
* =========================================================================== */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#ifdef _LINUX
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>
#include <unistd.h>
#include <signal.h>
//#include <asm/arch/v4l_ioctl.h>
//#include "lancam.h"
#include "net_cfg.h"

//#define NET_CONFIG_DEBUG

#ifdef NET_CONFIG_DEBUG
#define DBG_ERR(x...)     perror(x)
#define DBG_NET(x...)     printf(x)
#define DBG(x...)         printf(x)
#else
#define DBG_ERR(x...)
#define DBG_NET(x...)
#define DBG(x...)
#endif

/***************************************************************************
 *                                                                         *
 ***************************************************************************/
static struct sockaddr_in sa = {
sin_family:
  PF_INET,
  sin_port: 0
};

/**
 * @brief	Set a certain interface flag.
 * @param	"char *ifname" : interface name
 * @param	"short flag" : flag
 * @retval	0 : success ; -1 : fail
 */
int net_set_flag(const char *ifname, short flag) {
  struct ifreq ifr;
  int skfd;

  if ( (skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    DBG_ERR("socket error");
    return -1;
  }
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
    DBG_ERR("net_set_flag: ioctl SIOCGIFFLAGS");
    close(skfd);
    return (-1);
  }
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  ifr.ifr_flags |= flag;
  if (ioctl(skfd, SIOCSIFFLAGS, &ifr) < 0) {
    DBG_ERR("net_set_flag: ioctl SIOCSIFFLAGS");
    close(skfd);
    return -1;
  }
  close(skfd);
  return (0);
}

/**
 * @brief	Clear a certain interface flag.
 * @param	"char *ifname" : interface name
 * @param	"short flag" : flag
 * @retval	0 : success ; -1 : fail
 */
int net_clr_flag(const char *ifname, short flag) {
  struct ifreq ifr;
  int skfd;

  if ( (skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    DBG_ERR("socket error");
    return -1;
  }
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
    DBG_ERR("net_clr_flag: ioctl SIOCGIFFLAGS");
    close(skfd);
    return -1;
  }
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  ifr.ifr_flags &= ~flag;
  if (ioctl(skfd, SIOCSIFFLAGS, &ifr) < 0) {
    DBG_ERR("net_clr_flag: ioctl SIOCSIFFLAGS");
    close(skfd);
    return -1;
  }
  close(skfd);
  return (0);
}

/**
 * @brief	Get an interface flag.
 * @param	"char *ifname" : interface name
 * @retval	ifr.ifr_flags
 * @retval	-1 : fail
 */
int net_get_flag(const char *ifname) {
  struct ifreq ifr;
  int skfd;

  if ( (skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    DBG_ERR("socket error");
    return -1;
  }
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  if (ioctl(skfd, SIOCGIFFLAGS, &ifr) < 0) {
    DBG_ERR("net_get_flag: ioctl SIOCGIFFLAGS");
    close(skfd);
    return -1;
  }
  close(skfd);
  return ifr.ifr_flags;
}

/**
 * @brief	Set an interface up & running
 * @param	"char *ifname" : interface name
 * @retval	0 : success ; -1 : fail
 */
int net_nic_up(const char *ifname) {
  return net_set_flag(ifname, (IFF_UP | IFF_RUNNING));
}

/**
 * @brief	Set an interface down
 * @param	"char *ifname" : interface name
 * @retval	0 : success ; -1 : fail
 */
int net_nic_down(const char *ifname) {
  return net_clr_flag(ifname, IFF_UP);
}

/**
 * @brief	get address of an interface
 * @param	"char *ifname" : interface name
 * @retval	net address
 */
in_addr_t net_get_ifaddr(const char *ifname) {
  struct ifreq ifr;
  int skfd;
  struct sockaddr_in *saddr;

  if ( (skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    DBG_ERR("socket error");
    return -1;
  }

  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0) {
    DBG_ERR("net_get_ifaddr: ioctl SIOCGIFADDR");
    close(skfd);
    return -1;
  }
  close(skfd);

  saddr = (struct sockaddr_in *) &ifr.ifr_addr;
  return saddr->sin_addr.s_addr;
}

/**
 * @brief	get ip of an interface
 * @param	"int skfd" :
 * @param	"char *ifname" : interface name
 * @retval	ip
 */
struct in_addr net_get_ip(int skfd, const char *ifname) {
  struct ifreq ifr;

  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0) {
    DBG_ERR("net_get_ip: ioctl SIOCGIFADDR");
    return (struct in_addr) {
      -1
    };
  }
  return ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
}

/**
 * @brief	set ip of an interface
 * @param	"char *ifname" : interface name
 * @param	"in_addr_t addr" : ip address
 * @retval	0 : success ; -1 : fail
 */
int net_set_ifaddr(const char *ifname, in_addr_t addr) {
  struct ifreq ifr;
  int skfd;

  if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    DBG_ERR("socket error");
    return -1;
  }
  sa.sin_addr.s_addr = addr;
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  memcpy((char *) &ifr.ifr_addr, (char *) &sa, sizeof(struct sockaddr));
  if (ioctl(skfd, SIOCSIFADDR, &ifr) < 0) {
    DBG_ERR("net_set_ifaddr: ioctl SIOCSIFADDR");
    close(skfd);
    return -1;
  }
  close(skfd);
  return 0;
}

/**
 * @brief	get address of an interface
 * @param	"char *ifname" : interface name
 * @retval	address
 */
in_addr_t net_get_netmask(const char *ifname) {
  struct ifreq ifr;
  int skfd;
  struct sockaddr_in *saddr;

  if ( (skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    DBG_ERR("socket error");
    return -1;
  }

  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  if (ioctl(skfd, SIOCGIFNETMASK, &ifr) < 0) {
    DBG_ERR("net_get_netmask: ioctl SIOCGIFNETMASK");
    close(skfd);
    return -1;
  }
  close(skfd);

  saddr = (struct sockaddr_in *) &ifr.ifr_addr;
  return saddr->sin_addr.s_addr;
}

/**
 * @brief	get netmask of an interface
 * @param	"char *ifname" : [IN]interface name
 * @param	"in_addr_t addr" : [OUT]netmask
 * @retval	0 : success ; -1 : fail
 */
int net_set_netmask(const char *ifname, in_addr_t addr) {
  struct ifreq ifr;
  int skfd;

  if ( (skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    DBG_ERR("socket error");
    return -1;
  }
  sa.sin_addr.s_addr = addr;
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  memcpy((char *) &ifr.ifr_addr, (char *) &sa, sizeof(struct sockaddr));
  if (ioctl(skfd, SIOCSIFNETMASK, &ifr) < 0) {
    DBG_ERR("net_set_netmask: ioctl SIOCSIFNETMASK");
    close(skfd);
    return -1;
  }
  close(skfd);
  return 0;
}

/**
 * @brief	get mac address of an interface
 * @param	"char *ifname" : [IN]interface name
 * @param	"unsigned char *mac" : [OUT]mac address
 * @retval	0 : success ; -1 : fail
 */
int net_get_hwaddr(const char *ifname, unsigned char *mac) {
  struct ifreq ifr;
  int skfd;

  if ( (skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    DBG_ERR("socket error");
    return -1;
  }

  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  if (ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0) {
    DBG_ERR("net_get_hwaddr: ioctl SIOCGIFHWADDR");
    close(skfd);
    return -1;
  }
  close(skfd);

  memcpy(mac, ifr.ifr_ifru.ifru_hwaddr.sa_data, IFHWADDRLEN);
  return 0;
}

/**
 * @brief	add a gateway
 * @param	"in_addr_t addr" : [IN]address of gateway
 * @retval	0 : success ; -1 : fail
 */
int net_add_gateway(in_addr_t addr) {
  struct rtentry rt;
  int skfd;

  /* Clean out the RTREQ structure. */
  memset((char *) &rt, 0, sizeof(struct rtentry));

  /* Fill in the other fields. */
  rt.rt_flags = (RTF_UP | RTF_GATEWAY);

  rt.rt_dst.sa_family = PF_INET;
  rt.rt_genmask.sa_family = PF_INET;

  sa.sin_addr.s_addr = addr;
  memcpy((char *) &rt.rt_gateway, (char *) &sa, sizeof(struct sockaddr));

  /* Create a socket to the INET kernel. */
  if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    DBG_ERR("socket error");
    return -1;
  }
  /* Tell the kernel to accept this route. */
  if (ioctl(skfd, SIOCADDRT, &rt) < 0) {
    DBG_ERR("net_add_gateway: ioctl SIOCADDRT");
    close(skfd);
    return -1;
  }
  /* Close the socket. */
  close(skfd);
  return (0);
}

/**
 * @brief	delete a gateway
 * @param	"in_addr_t addr" : [IN]address of gateway
 * @retval	0 : success ; -1 : fail
 */
int net_del_gateway(in_addr_t addr) {
  struct rtentry rt;
  int skfd;

  /* Clean out the RTREQ structure. */
  memset((char *) &rt, 0, sizeof(struct rtentry));

  /* Fill in the other fields. */
  rt.rt_flags = (RTF_UP | RTF_GATEWAY);

  rt.rt_dst.sa_family = PF_INET;
  rt.rt_genmask.sa_family = PF_INET;

  sa.sin_addr.s_addr = addr;
  memcpy((char *) &rt.rt_gateway, (char *) &sa, sizeof(struct sockaddr));

  /* Create a socket to the INET kernel. */
  if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    DBG_ERR("socket error");
    return -1;
  }
  /* Tell the kernel to accept this route. */
  if (ioctl(skfd, SIOCDELRT, &rt) < 0) {
    DBG_ERR("net_del_gateway: ioctl SIOCDELRT");
    close(skfd);
    return -1;
  }
  /* Close the socket. */
  close(skfd);
  return (0);
}

/**
 * @brief	set domain name server.
 * @param	"char *dnsname" : [IN]dns name
 * @retval	0 : success ; -1 : fail
 */
int net_set_dns(const char *dnsname) {
  FILE *fp;

  fp = fopen(RESOLV_CONF, "w");
  if ( fp ) {
    fprintf(fp, "nameserver %s\n", dnsname);
    fclose(fp);
    DBG_NET("dns=%s\n", dnsname);
    return 0;
  }
  DBG("net_set_dns: file \"%s\" opened for writing error!\n", RESOLV_CONF);
  return -1;
}

/**
 * @brief	get domain name server.
 * @param	none
 * @retval	dns address
 */
in_addr_t net_get_dns(void) {
  FILE *fp;
  char dnsname[80];

  fp = fopen(RESOLV_CONF, "r");
  if ( fp ) {
    if (fscanf(fp, "nameserver %s\n", dnsname) != EOF) {
      fclose(fp);
      DBG_NET("dns=%s\n", dnsname);
      return inet_addr(dnsname);
    } else {
      fclose(fp);
      net_set_dns("0.0.0.0");
      return inet_addr("0.0.0.0");
    }
  }
  DBG("net_get_dns: file \"%s\" opened for reading error!\n", RESOLV_CONF);
  return INADDR_ANY;
}

/**
 * @brief	search gateway
 * @param	"char *buf" : [IN]buffer
 * @param	"in_addr_t *gate_addr" : [OUT]gateway address
 * @return	0 : success ; -1 : fail
 */
int net_search_gateway(char *buf, in_addr_t *gate_addr) {
  char iface[16];
  unsigned long dest, gate;
  int iflags;

  sscanf(buf, "%s\t%08lX\t%08lX\t%8X\t", iface, &dest, &gate, &iflags);
  DBG_NET("net_search_gateway:%s, %lX, %lX, %X\n", iface, dest, gate, iflags);
  if ( (iflags & (RTF_UP | RTF_GATEWAY)) == (RTF_UP | RTF_GATEWAY) ) {
    *gate_addr = gate;
    return 0;
  }
  return -1;
}

/**
 * @brief	set gateway
 * @param	"in_addr_t addr" : [IN]gateway address
 * @return	0 : success ; -1 : fail
 */
int net_set_gateway(in_addr_t addr) {
  in_addr_t gate_addr;
  char buff[132];
  FILE *fp = fopen(PROCNET_ROUTE_PATH, "r");

  if (!fp) {
    DBG_ERR(PROCNET_ROUTE_PATH);
    DBG("INET (IPv4) not configured in this system.\n");
    return -1;
  }
  fgets(buff, 130, fp);
  while (fgets(buff, 130, fp) != NULL) {
    if (net_search_gateway(buff, &gate_addr) == 0) {
      net_del_gateway(gate_addr);
    }
  }
  fclose(fp);

  return net_add_gateway(addr);
}

/**
 * @brief	clean gateway
 * @param	none
 * @return	0 : success ; -1 : fail
 */
int net_clean_gateway(void) {
  in_addr_t gate_addr;
  char buff[132];
  FILE *fp = fopen(PROCNET_ROUTE_PATH, "r");

  if (!fp) {
    DBG_ERR(PROCNET_ROUTE_PATH);
    DBG("INET (IPv4) not configured in this system.\n");
    return -1;
  }
  fgets(buff, 130, fp);
  while (fgets(buff, 130, fp) != NULL) {
    if (net_search_gateway(buff, &gate_addr) == 0) {
      net_del_gateway(gate_addr);
    }
  }
  fclose(fp);

  return 0;
}

/**
 * @brief	get gateway
 * @param	none
 * @return	gatewat address
 */
in_addr_t net_get_gateway(void) {
  in_addr_t gate_addr;
  char buff[132];
  FILE *fp = fopen(PROCNET_ROUTE_PATH, "r");

  if (!fp) {
    DBG_ERR(PROCNET_ROUTE_PATH);
    DBG("INET (IPv4) not configured in this system.\n");
    return (INADDR_ANY);
  }
  fgets(buff, 130, fp);
  while (fgets(buff, 130, fp) != NULL) {
    if (net_search_gateway(buff, &gate_addr) == 0) {
      fclose(fp);
      return gate_addr;
    }
  }
  fclose(fp);
  return (INADDR_ANY);
}
#ifdef WIFI_LAN

int net_search_allgateway(char *buf, in_addr_t *gate_addr,in_addr_t *dest_addr,in_addr_t *mask_addr) {
  char iface[16];
  unsigned long dest, gate,mask;
  int iflags,refcnt,use,metric;

  sscanf(buf, "%s\t%08lX\t%08lX\t%8X\t%8X\t%8X\t%8X\t%08lX\t", iface, &dest, &gate, &iflags,
         &refcnt,&use,&metric,&mask);
  DBG_NET("net_search_allgateway:%s, %lX, %lX, %X, %lX,\n", iface, dest, gate, iflags,mask);
  if (iflags & RTF_UP == RTF_UP ) {
    *gate_addr = gate;
    *dest_addr = dest;
    *mask_addr = mask;
    return 0;
  }
  return -1;
}

int net_del_gateway_byname(char *ifname,in_addr_t gate_addr,in_addr_t dest_addr,in_addr_t mask_addr) {
  struct rtentry rt;
  int skfd;
  DBG_NET("net_del_gateway_byname,ifname=%s,addr=%08lX\n",ifname,gate_addr);
  /* Clean out the RTREQ structure. */
  memset((char *) &rt, 0, sizeof(struct rtentry));

  /* Fill in the other fields. */
  rt.rt_flags = (RTF_UP | RTF_GATEWAY);
  rt.rt_dev = ifname;
  rt.rt_dst.sa_family = PF_INET;
  rt.rt_genmask.sa_family = PF_INET;


  sa.sin_addr.s_addr = dest_addr;
  memcpy((char *) &rt.rt_dst, (char *) &sa, sizeof(struct sockaddr));
  sa.sin_addr.s_addr = mask_addr;
  memcpy((char *) &rt.rt_genmask, (char *) &sa, sizeof(struct sockaddr));

  sa.sin_addr.s_addr = gate_addr;
  memcpy((char *) &rt.rt_gateway, (char *) &sa, sizeof(struct sockaddr));

  /* Create a socket to the INET kernel. */
  if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    DBG_ERR("socket error");
    return -1;
  }
  /* Tell the kernel to accept this route. */
  if (ioctl(skfd, SIOCDELRT, &rt) < 0) {
    DBG_ERR("net_del_gateway: ioctl SIOCDELRT");
    close(skfd);
    return -1;
  }
  /* Close the socket. */
  close(skfd);
  return (0);
}

int net_clean_gateway_byname(char *ifname) {
  in_addr_t gate_addr;
  in_addr_t dest_addr;
  in_addr_t mask_addr;
  int findflag = 0;
  char buff[132];
  FILE *fp = fopen(PROCNET_ROUTE_PATH, "r");
  if (!fp) {
    DBG_ERR(PROCNET_ROUTE_PATH);
    DBG("INET (IPv4) not configured in this system.\n");
    return -1;
  }
  fgets(buff, 130, fp);
  while (fgets(buff, 130, fp) != NULL) {
    if(ifname && (strstr(buff,ifname) != NULL)) {
      if (net_search_allgateway(buff, &gate_addr,&dest_addr,&mask_addr) == 0) {
        net_del_gateway_byname(ifname,gate_addr,dest_addr,mask_addr);
      }
    }
  }

  fclose(fp);
  return 0;
}

int net_add_route_byname(char *ifname,in_addr_t gate_addr,in_addr_t dest_addr,in_addr_t mask_addr,int flag) {
  struct rtentry rt;
  int skfd;
  DBG_NET("net_add_route_byname,ifname=%s,gate=%08lX,dest=%08lX,mask=%08lX,flag=%d,\n",ifname,gate_addr,dest_addr,mask_addr,flag);
  /* Clean out the RTREQ structure. */
  memset((char *) &rt, 0, sizeof(struct rtentry));

  /* Fill in the other fields. */
  rt.rt_flags = flag;
  rt.rt_dev = ifname;
  rt.rt_dst.sa_family = PF_INET;
  rt.rt_genmask.sa_family = PF_INET;

  sa.sin_addr.s_addr = dest_addr;
  memcpy((char *) &rt.rt_dst, (char *) &sa, sizeof(struct sockaddr));
  sa.sin_addr.s_addr = mask_addr;
  memcpy((char *) &rt.rt_genmask, (char *) &sa, sizeof(struct sockaddr));

  sa.sin_addr.s_addr = gate_addr;
  memcpy((char *) &rt.rt_gateway, (char *) &sa, sizeof(struct sockaddr));

  /* Create a socket to the INET kernel. */
  if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    DBG_ERR("socket error");
    return -1;
  }
  /* Tell the kernel to accept this route. */
  if (ioctl(skfd, SIOCADDRT, &rt) < 0) {
    DBG_ERR("net_add_route_byname: ioctl SIOCADDRT");
    close(skfd);
    return -1;
  }
  /* Close the socket. */
  close(skfd);
  return (0);
}

int net_set_route_byname(char *ifname,in_addr_t gateaddr,in_addr_t ipaddr,in_addr_t maskaddr) {
  in_addr_t gate_addr;
  in_addr_t dest_addr;
  in_addr_t mask_addr;
  int ret = 0;
  char buff[132];
  FILE *fp = fopen(PROCNET_ROUTE_PATH, "r");

  if (!fp) {
    DBG_ERR(PROCNET_ROUTE_PATH);
    DBG("INET (IPv4) not configured in this system.\n");
    return -1;
  }
  fgets(buff, 130, fp);
  while (fgets(buff, 130, fp) != NULL) {
    if(ifname && (strstr(buff,ifname) != NULL)) {
      if (net_search_allgateway(buff, &gate_addr,&dest_addr,&mask_addr) == 0) {
        net_del_gateway_byname(ifname,gate_addr,dest_addr,mask_addr);
      }
    }
  }
  fclose(fp);


  ret = net_add_route_byname(ifname,0,(ipaddr&maskaddr),maskaddr,RTF_UP);
  if(ret == 0) {
    ret = net_add_route_byname(ifname,gateaddr,0,0,(RTF_UP | RTF_GATEWAY));
  }
  return ret;
}


#endif
int net_add_gateway_byname(char *ifname,in_addr_t addr) {
  struct rtentry rt;
  int skfd;

  /* Clean out the RTREQ structure. */
  memset((char *) &rt, 0, sizeof(struct rtentry));

  /* Fill in the other fields. */
  rt.rt_flags = (RTF_UP | RTF_GATEWAY);
  rt.rt_dev = ifname;
  rt.rt_dst.sa_family = PF_INET;
  rt.rt_genmask.sa_family = PF_INET;

  sa.sin_addr.s_addr = addr;
  memcpy((char *) &rt.rt_gateway, (char *) &sa, sizeof(struct sockaddr));

  /* Create a socket to the INET kernel. */
  if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    DBG_ERR("socket error");
    return -1;
  }
  /* Tell the kernel to accept this route. */
  if (ioctl(skfd, SIOCADDRT, &rt) < 0) {
    DBG_ERR("net_add_gateway: ioctl SIOCADDRT");
    close(skfd);
    return -1;
  }
  /* Close the socket. */
  close(skfd);
  return (0);
}


int net_set_gateway_byname(char *ifname,in_addr_t addr) {
  in_addr_t gate_addr;
  char buff[132];
  FILE *fp = fopen(PROCNET_ROUTE_PATH, "r");

  if (!fp) {
    DBG_ERR(PROCNET_ROUTE_PATH);
    DBG("INET (IPv4) not configured in this system.\n");
    return -1;
  }
  fgets(buff, 130, fp);
  while (fgets(buff, 130, fp) != NULL) {
    if(ifname && (strstr(buff,ifname) != NULL)) {
      if (net_search_gateway(buff, &gate_addr) == 0) {
        net_del_gateway(gate_addr);
      }
    }
  }
  fclose(fp);

  return net_add_gateway_byname(ifname,addr);
}


in_addr_t net_get_gateway_byname(char *ifname) {
  in_addr_t gate_addr;
  char buff[132];
  FILE *fp = fopen(PROCNET_ROUTE_PATH, "r");

  if (!fp) {
    DBG_ERR(PROCNET_ROUTE_PATH);
    DBG("INET (IPv4) not configured in this system.\n");
    return (INADDR_ANY);
  }
  fgets(buff, 130, fp);
  while (fgets(buff, 130, fp) != NULL) {
    if(ifname && (strstr(buff,ifname) != NULL)) {
      if (net_search_gateway(buff, &gate_addr) == 0) {
        fclose(fp);
        return gate_addr;
      }
    }
  }
  fclose(fp);
  return (INADDR_ANY);
}


in_addr_t net_search_gateway_byname(char *ifname) {
  in_addr_t gate_addr;
  int findflag = 0;
  char buff[132];
  FILE *fp = fopen(PROCNET_ROUTE_PATH, "r");
  if (!fp) {
    DBG_ERR(PROCNET_ROUTE_PATH);
    DBG("INET (IPv4) not configured in this system.\n");
    return (INADDR_ANY);
  }
  fgets(buff, 130, fp);
  while (fgets(buff, 130, fp) != NULL) {
    if(ifname && (strstr(buff,ifname) != NULL)) {
      if (net_search_gateway(buff, &gate_addr) == 0) {
        fclose(fp);
        return gate_addr;
      }
    }
  }
  fclose(fp);
  return (INADDR_ANY);
}

/**
 * @brief	get net info
 * @param	"char *ifname" : [IN]interface name
 * @param	"struct NET_CONFIG *netcfg" : [OUT]net config
 * @return	0 : success ; -1 : fail
 */
int net_get_info(const char *ifname, struct NET_CONFIG *netcfg) {
  struct ifreq ifr;
  int skfd;
  struct sockaddr_in *saddr;

  if ( (skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
    DBG_ERR("socket error");
    return -1;
  }

  saddr = (struct sockaddr_in *) &ifr.ifr_addr;
  strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
  if (ioctl(skfd, SIOCGIFADDR, &ifr) < 0) {
    DBG_ERR("net_get_info: ioctl SIOCGIFADDR");
    close(skfd);
    return -1;
  }
  netcfg->ifaddr = saddr->sin_addr.s_addr;
  DBG_NET("ifaddr=0x%x\n", netcfg->ifaddr);

  if (ioctl(skfd, SIOCGIFNETMASK, &ifr) < 0) {
    DBG_ERR("net_get_info: ioctl SIOCGIFNETMASK");
    close(skfd);
    return -1;
  }
  netcfg->netmask = saddr->sin_addr.s_addr;
  DBG_NET("netmask=0x%x\n", netcfg->netmask);

  if (ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0) {
    DBG_ERR("net_get_info: ioctl SIOCGIFHWADDR");
    close(skfd);
    return -1;
  }
  memcpy(netcfg->mac, ifr.ifr_ifru.ifru_hwaddr.sa_data, IFHWADDRLEN);
  DBG_NET("hwaddr=%02x:%02x:%02x:%02x:%02x:%02x\n", netcfg->mac[0],netcfg->mac[1],
          netcfg->mac[2],netcfg->mac[3],netcfg->mac[4],netcfg->mac[5]);

  close(skfd);
  netcfg->gateway = net_get_gateway();
  DBG_NET("gateway=0x%x\n", netcfg->gateway);
  netcfg->dns = net_get_dns();
  return 0;
}

/**
 * @brief	main entry
 * @param	command line argument
 * @return	error code
 */
//#define NET_TEST
#ifdef NET_TEST
int main(int argc, char **argv) {
  struct NET_CONFIG netcfg;
  int i;

  if (argc < 3) {
    net_get_info("eth0", &netcfg);
    return 0;
  }

  for (i=1; i<argc; i++)
    printf("arg[%d]=%s\n", i, argv[i]);

  if (!strcmp(argv[2], "up"))
    net_set_flag(argv[1], (IFF_UP | IFF_RUNNING));
  else if (!strcmp(argv[2], "down"))
    net_clr_flag(argv[1], IFF_UP);
  else if (!strcmp(argv[2], "ip")) {
    net_set_ifaddr(argv[1], inet_addr(argv[3]));
  } else if (!strcmp(argv[2], "netmask")) {
    net_set_netmask(argv[1], inet_addr(argv[3]));
  } else if (!strcmp(argv[2], "gateway")) {
    if (!strcmp(argv[1], "add"))
      net_add_gateway(inet_addr(argv[3]));
    else
      net_del_gateway(inet_addr(argv[3]));
  } else if (!strcmp(argv[2], "dhcpcd")) {
    if (!strcmp(argv[3], "enable"))
      net_enable_dhcpcd(argv[1]);
    else
      net_disable_dhcpcd(argv[1]);
  } else if (!strcmp(argv[1], "dns"))
    net_set_dns(argv[2]);
  else
    printf("unknown argument!\n");
  return 0;
}
#endif  // NET_TEST

#endif  // _LINUX
