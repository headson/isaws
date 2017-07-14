#ifndef __JSON_ONVIFINFO_H__
#define __JSON_ONVIFINFO_H__

#ifdef _WIN32

#else
#include <arpa/inet.h>
#endif
#include "onvif_env.h"

typedef struct
{
	struct in_addr	ip; ///< IP address in static IP mode
	struct in_addr	netmask; ///< netmask in static IP mode
	struct in_addr	gateway; ///< gateway in static IP mode
	struct in_addr	dns; ///< DNS IP in static IP mode
	int			dhcp_enable;
} NetworkInfo;

typedef struct
{
	unsigned short		http_port; ///< HTTP port in web site.
	unsigned short		rtsp_port; ///< HTTP port in web site.
} NetworkPort;

#if defined (__cplusplus)
extern "C" {
#endif

int ParserNetInterface(const char *value,int value_size,NetworkInfo *net_interface);

int ParserNetPort(const char *value,int value_size,NetworkPort *net_port);
int JsonRead_OnvifInfo(const char* file_path,Onvif_Info *pOnvifInfo);


#if defined (__cplusplus)
}
#endif

#endif //__JSON_ONVIFINFO_H__
