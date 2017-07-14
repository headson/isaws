/***************************************************************************************
 *
 *  IMPORTANT: READ BEFORE DOWNLOADING, COPYING, INSTALLING OR USING.
 *
 *  By downloading, copying, installing or using the software you agree to this license.
 *  If you do not agree to this license, do not download, install, 
 *  copy or use the software.
 *
 *  Copyright (C) 2010-2014, Happytimesoft Corporation, all rights reserved.
 *
 *  Redistribution and use in binary forms, with or without modification, are permitted.
 *
 *  Unless required by applicable law or agreed to in writing, software distributed 
 *  under the License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
 *  CONDITIONS OF ANY KIND, either express or implied. See the License for the specific
 *  language governing permissions and limitations under the License.
 *
****************************************************************************************/

#ifndef _DEVICE_H_
#define _DEVICE_H_

#include "onvif/bm/sys_inc.h"
#include "onvif/onvif/onvif.h"

/***************************************************************************************/
typedef struct 
{
	onvif_DNSInformation	DNSInformation;			// required, 
} SetDNS_REQ;

typedef struct 
{
	onvif_NTPInformation	NTPInformation;			// required, 
} SetNTP_REQ;

typedef struct
{
	onvif_NetworkProtocol	NetworkProtocol;		// required,  
} SetNetworkProtocols_REQ;

typedef struct
{
	char 	IPv4Address[MAX_GATEWAY][32];			// optional, Sets IPv4 gateway address used as default setting
} SetNetworkDefaultGateway_REQ;

typedef struct
{	
	uint32	UTCDateTimeFlag	: 1;					// Indicates whether the field UTCDateTime is valid
	uint32 	Reserved		: 31;
	
    onvif_SystemDateTime	SystemDateTime;			// required,     
    onvif_DateTime 			UTCDateTime;			// optional, Date and time in UTC. If time is obtained via NTP, UTCDateTime has no meaning
} SetSystemDateAndTime_REQ;

typedef struct
{
	onvif_NetworkInterface	NetworkInterface;		// required,  
} SetNetworkInterfaces_REQ;

typedef struct
{
	onvif_DiscoveryMode	DiscoveryMode;				// required, Indicator of discovery mode: Discoverable, NonDiscoverable
} SetDiscoveryMode_REQ;

typedef struct
{	
	char	UploadUri[256];							// required, A URL to which the firmware file may be uploaded
	int		UploadDelay;							// required, An optional delay; the client shall wait for this amount of time before initiating the firmware upload, unit is second
	int		ExpectedDownTime;						// required, A duration that indicates how long the device expects to be unavailable after the firmware upload is complete, unit is second
} StartFirmwareUpgrade_RES;



#ifdef __cplusplus
extern "C" {
#endif

ONVIF_RET onvif_SetSystemDateAndTime(SetSystemDateAndTime_REQ * p_req);
ONVIF_RET onvif_SetHostname(const char * name, BOOL fromdhcp);
ONVIF_RET onvif_SetDNS(SetDNS_REQ * p_req);
ONVIF_RET onvif_SetNTP(SetNTP_REQ * p_req);
ONVIF_RET onvif_SetNetworkProtocols(SetNetworkProtocols_REQ * p_req);
ONVIF_RET onvif_SetNetworkDefaultGateway(SetNetworkDefaultGateway_REQ * p_req);
ONVIF_RET onvif_SystemReboot();
ONVIF_RET onvif_SetSystemFactoryDefault(int type /* 0:soft, 1:hard */);
ONVIF_RET onvif_SetNetworkInterfaces(SetNetworkInterfaces_REQ * p_req);
ONVIF_RET onvif_SetDiscoveryMode(SetDiscoveryMode_REQ * p_req);

BOOL	  onvif_StartFirmwareUpgrade(StartFirmwareUpgrade_RES * p_res);	
BOOL 	  onvif_FirmwareUpgradeCheck(const char * buff, int len);
BOOL 	  onvif_FirmwareUpgrade(const char * buff, int len);
void 	  onvif_FirmwareUpgradePost();



#ifdef __cplusplus
}
#endif

#endif // _DEVICE_H_

