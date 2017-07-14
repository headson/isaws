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

#include "onvif/onvif/onvif_device.h"
#include "onvif/bm/sys_inc.h"
#include "onvif/onvif/onvif.h"
#include "onvif/onvif/xml_node.h"
#include "onvif/onvif/onvif_util.h"


/***************************************************************************************/
extern ONVIF_CLS g_onvif_cls;

/***************************************************************************************/

ONVIF_RET onvif_SetSystemDateAndTime(SetSystemDateAndTime_REQ * p_req)
{
	// check datetime
	if (p_req->SystemDateTime.DateTimeType == SetDateTimeType_Manual)
	{
		if (p_req->UTCDateTime.Date.Month < 1 || p_req->UTCDateTime.Date.Month > 12 ||
			p_req->UTCDateTime.Date.Day < 1 || p_req->UTCDateTime.Date.Day > 31 ||
			p_req->UTCDateTime.Time.Hour < 0 || p_req->UTCDateTime.Time.Hour > 23 ||
			p_req->UTCDateTime.Time.Minute < 0 || p_req->UTCDateTime.Time.Minute > 59 ||
			p_req->UTCDateTime.Time.Second < 0 || p_req->UTCDateTime.Time.Second > 61)
		{
			return ONVIF_ERR_INVALID_DATETIME;
		}
	}

	// check timezone
	if (p_req->SystemDateTime.TimeZoneFlag && 
		p_req->SystemDateTime.TimeZone.TZ[0] != '\0' && 
		onvif_is_valid_timezone(p_req->SystemDateTime.TimeZone.TZ) == FALSE)
	{
		return ONVIF_ERR_INVALID_TIMEZONE;
	}

	// todo : add set system date time code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetSystemDateAndTime(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_SystemReboot()
{
	// reboot system ...
	return my_onvif_SystemReboot();
}

ONVIF_RET onvif_SetSystemFactoryDefault(int type /* 0:soft, 1:hard */)
{
	// set system factory default
	return my_onvif_SetSystemFactoryDefault(type);
}

ONVIF_RET onvif_SetHostname(const char * name, BOOL fromdhcp)
{
    if (fromdhcp)
    {
    }
    else if (onvif_is_valid_hostname(name) == FALSE)
    {
    	return ONVIF_ERR_INVALID_HOSTNAME;
    }
    
    // set hostname ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetHostname(name);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}

    return ONVIF_OK;
}

ONVIF_RET onvif_SetDNS(SetDNS_REQ * p_req)
{
	// todo : add set DNS code ...
	
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetDNS(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
    
	return ONVIF_OK;
}

ONVIF_RET onvif_SetNTP(SetNTP_REQ * p_req)
{
	// todo : add set NTP code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetNTP(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
    
	return ONVIF_OK;
}


ONVIF_RET onvif_SetNetworkProtocols(SetNetworkProtocols_REQ * p_req)
{
	// todo : add set network protocols code ...	
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetNetworkProtocols(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_SetNetworkDefaultGateway(SetNetworkDefaultGateway_REQ * p_req)
{
	// todo : add set network default gateway code ...
	
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetNetworkDefaultGateway(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}

	return ONVIF_OK;
}

ONVIF_RET onvif_SetNetworkInterfaces(SetNetworkInterfaces_REQ * p_req)
{
    if (p_req->NetworkInterface.InfoFlag && 
    	p_req->NetworkInterface.Info.MTUFlag && 
    	(p_req->NetworkInterface.Info.MTU < 0 || p_req->NetworkInterface.Info.MTU > 1530))
    {
        return ONVIF_ERR_INVALID_MTU_VALUE;
    }

    if (p_req->NetworkInterface.Enabled && 
        p_req->NetworkInterface.IPv4Flag && 
        p_req->NetworkInterface.IPv4.Enabled && 
        p_req->NetworkInterface.IPv4.Config.DHCP == FALSE)
    {
        if (is_ip_address(p_req->NetworkInterface.IPv4.Config.Address) == FALSE)
        {
            return ONVIF_ERR_INVALID_IPV4_ADDR;
        }
    }

    // todo : add set network interfaces code ...
	
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetNetworkInterfaces(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
    
	return ONVIF_OK;
}

ONVIF_RET onvif_SetDiscoveryMode(SetDiscoveryMode_REQ * p_req)
{
	return my_onvif_SetDiscoveryMode(p_req);
}

BOOL onvif_StartFirmwareUpgrade(StartFirmwareUpgrade_RES * p_res)
{
	// todo : do some file upload prepare ...

	
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_StartFirmwareUpgrade(p_res);
	if(SdkResult != ONVIF_OK)
	{
		return FALSE;
	}

	sprintf(p_res->UploadUri, "http://%s/cgi-bin/update.cgi", g_onvif_cls.local_ipstr);
	
	p_res->UploadDelay = 5;				// 5 seconds
	p_res->ExpectedDownTime = 5 * 60; 	// 5 minutes

	return TRUE;
}

/***
  * do some check before the upgrade
  *
  * buff : pointer the upload content
  * len  : the upload content length
  **/
BOOL onvif_FirmwareUpgradeCheck(const char * buff, int len)
{
	// todo : add the check code ...
	
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_FirmwareUpgradeCheck(buff, len);
	if(SdkResult != ONVIF_OK)
	{
		return FALSE;
	}
	return TRUE;
}

/***
  * begin firmware upgrade
  *
  * buff : pointer the upload content
  * len  : the upload content length
  **/
BOOL onvif_FirmwareUpgrade(const char * buff, int len)
{
	// todo : add the upgrade code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_FirmwareUpgrade(buff, len);
	if(SdkResult != ONVIF_OK)
	{
		return FALSE;
	}
	
	return TRUE;
}

/***
  * After the upgrade is complete do some works, such as reboot device ...
  *  
  **/
void onvif_FirmwareUpgradePost()
{
	my_onvif_SystemReboot();
}





