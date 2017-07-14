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

#include "onvif/bm/sys_inc.h"
#include "onvif/onvif/hxml.h"
#include "onvif/onvif/xml_node.h"
#include "onvif/onvif/onvif_local.h"
#include "onvif/onvif/onvif.h"
#include "onvif/onvif/http.h"
#include "onvif/onvif/onvif_device.h"
#include "onvif/onvif/onvif_pkt.h"
#include "onvif/onvif/soap_parser.h"
#include "onvif/onvif/onvif_event.h"
#include "onvif/bm/sha1.h"
#include "onvif/onvif/onvif_ptz.h"
#include "onvif/onvif/onvif_err.h"
#include "onvif/onvif/onvif_image.h"
#include "onvif/onvif/http_auth.h"

#ifdef PROFILE_G_SUPPORT
#include "onvif/onvif/onvif_recording.h"
#endif
#include "onvif/cfgpro/onvif_cfgfile_mng.h"

/***************************************************************************************/
HD_AUTH_INFO g_auth_info;
char g_send_buf[1024*2048]={0};
char *g_reply_buf = &g_send_buf[1024];
char *g_probe_buf = &g_send_buf[2038*1024];

/***************************************************************************************/
int soap_http_rly(HTTPCLN * p_user, HTTPMSG * rx_msg, const char * p_xml, int len)
{
  int tlen;
	char * p_bufs = g_send_buf;

	//p_bufs = (char *)malloc(len + 1024);
	if (NULL == p_bufs)
	{
		return -1;
	}
	
	tlen = sprintf(p_bufs,	"HTTP/1.1 200 OK\r\n"
							"Server: hsoap/2.8\r\n"
							"Content-Type: %s\r\n"
							"Content-Length: %d\r\n"
							"Connection: close\r\n\r\n",
							get_http_headline(rx_msg, "Content-Type"), len);

	memmove(p_bufs+tlen, p_xml, len);
	tlen += len;

	p_bufs[tlen] = '\0';
	log_print("TX >> %s\r\n\r\n", p_bufs);
	
	send(p_user->cfd, p_bufs, tlen, 0);
	//free(p_bufs);
	
	return tlen;
}

int soap_http_err_rly(HTTPCLN * p_user, HTTPMSG * rx_msg, int err_code, const char * err_str, const char * p_xml, int len)
{
    int tlen;
	char * p_bufs = g_send_buf;
	char auth[256] = {'\0'};
	//p_bufs = (char *)malloc(1024 * 16);
	if (NULL == p_bufs)
	{
		return -1;
	}
	Onvif_Info* pOnvifInfo = GetOnvifInfo();

    if (pOnvifInfo->need_auth)
    {
        sprintf(g_auth_info.auth_nonce,"%08X%08X",rand(),rand());
		strcpy(g_auth_info.auth_qop,"auth");
		strcpy(g_auth_info.auth_realm, "happytimesoft");
		
        sprintf(auth, "WWW-Authenticate: Digest realm=\"%s\", qop=\"%s\", nonce=\"%s\"\r\n", 
            g_auth_info.auth_realm, g_auth_info.auth_qop, g_auth_info.auth_nonce);
    }
	
	tlen = sprintf(p_bufs,	"HTTP/1.1 %d %s\r\n"
							"Server: hsoap/2.8\r\n"
							"Content-Type: %s\r\n"
							"Content-Length: %d\r\n"
							"%s"
							"Connection: close\r\n\r\n",
							err_code, err_str,
							get_http_headline(rx_msg, "Content-Type"), len, auth);

	memmove(p_bufs+tlen, p_xml, len);
	tlen += len;

	p_bufs[tlen] = '\0';
	log_print("TX >> %s\r\n\r\n", p_bufs);
	
	send(p_user->cfd, p_bufs, tlen, 0);
	//free(p_bufs);
	
	return tlen;
}

int soap_err_rly
(
HTTPCLN * p_user, 
HTTPMSG * rx_msg, 
const char * code, 
const char * subcode, 
const char * subcode_ex,
const char * reason,
int http_err_code, 
const char * http_err_str
)
{
    int ret = -1, mlen = 1024*16, xlen;
    char * p_xml =g_reply_buf;
    
	printf("soap_err_rly\r\n");
	
	//p_xml = (char *)malloc(mlen);
	if (NULL == p_xml)
	{
		goto soap_rly_err;
	}
	
	xlen = build_err_rly_xml(p_xml, mlen, code, subcode, subcode_ex, reason);
	if (xlen < 0 || xlen >= mlen)
	{
		goto soap_rly_err;
	}
	
	ret = soap_http_err_rly(p_user, rx_msg, http_err_code, http_err_str, p_xml, xlen);
	
soap_rly_err:

	if (p_xml)
	{
		//free(p_xml);
	}
	
	return ret;
}

int soap_err_def_rly(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	return soap_err_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, NULL, "Action Not Implemented", 400, "Bad Request");
}

int soap_err_def2_rly(HTTPCLN * p_user, HTTPMSG * rx_msg, const char * code, const char * subcode, const char * subcode_ex, const char * reason)
{
	return soap_err_rly(p_user, rx_msg, code, subcode, subcode_ex, reason, 400, "Bad Request");
}

int soap_security_rly(HTTPCLN * p_user, HTTPMSG * rx_msg, int errcode)
{
	printf("soap_security_rly.\r\n");

    return soap_err_rly(p_user, rx_msg, ERR_SENDER, ERR_NOTAUTHORIZED, NULL, "Sender not Authorized", errcode, "Not Authorized");
}


int soap_build_err_rly(HTTPCLN * p_user, HTTPMSG * rx_msg, ONVIF_RET err)
{
	int ret = 0;
	
	switch (err)
	{
	case ONVIF_ERR_INVALID_IPV4_ADDR:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidIPv4Address", "Invalid IPv4 Address");
		break;

	case ONVIF_ERR_INVALID_IPV6_ADDR:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidIPv6Address", "Invalid IPv6 Address");	
		break;

	case ONVIF_ERR_INVALID_DNS_NAME:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter::InvalidDnsName", "Invalid DNS Name");	
		break;	

	case ONVIF_ERR_SERVICE_NOT_SUPPORT:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ServiceNotSupported", "Service Not Supported");	
		break;

	case ONVIF_ERR_PORT_ALREADY_INUSE:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:PortAlreadyInUse", "Port Already In Use");	
		break;	

	case ONVIF_ERR_INVALID_GATEWAY_ADDR:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidGatewayAddress", "Invalid Gateway Address");	
		break;	

	case ONVIF_ERR_INVALID_HOSTNAME:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidHostname", "Invalid Hostname");	
		break;	

	case ONVIF_ERR_MISSINGATTR:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attribute");	
		break;	

	case ONVIF_ERR_INVALID_DATETIME:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidDateTime", "Invalid Datetime");	
		break;		

	case ONVIF_ERR_INVALID_TIMEZONE:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidTimeZone", "Invalid Timezone");	
		break;	

	case ONVIF_ERR_PROFILE_EXISTS:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ProfileExists", "Profile Exist");	
		break;	

	case ONVIF_ERR_MAX_NVT_PROFILES:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:MaxNVTProfiles", "Max Profiles");
		break;

	case ONVIF_ERR_NO_PROFILE:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoProfile", "Profile Not Exist");
		break;

	case ONVIF_ERR_DEL_FIX_PROFILE:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_ACTION, "ter:DeletionOfFixedProfile", "Deleting Fixed Profile");
		break;

	case ONVIF_ERR_NO_CONFIG:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoConfig", "Config Not Exist");
		break;

	case ONVIF_ERR_NO_PTZ_PROFILE:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoPTZProfile", "PTZ Profile Not Exist");
		break;	

	case ONVIF_ERR_NO_HOME_POSITION:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_ACTION, "ter:NoHomePosition", "No Home Position");
		break;	

	case ONVIF_ERR_NO_TOKEN:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_ACTION, "ter:NoToken", "The requested token does not exist.");
		break;	

	case ONVIF_ERR_PRESET_EXIST:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_ACTION, "ter:PresetExist", "The requested name already exist for another preset.");
		break;

	case ONVIF_ERR_TOO_MANY_PRESETS:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_ACTION, "ter:TooManyPresets", "Maximum number of Presets reached.");
		break;	

	case ONVIF_ERR_MOVING_PTZ:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_ACTION, "ter:MovingPTZ", "Preset cannot be set while PTZ unit is moving.");
		break;

	case ONVIF_ERR_NO_ENTITY:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoEntity", "No such PTZ Node on the device");
		break;	

    case ONVIF_ERR_INVALID_NETWORK_INTERFACE:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidNetworkInterface", "The supplied network interface token does not exist");
		break;	

    case ONVIF_ERR_INVALID_MTU_VALUE:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidMtuValue", "The MTU value is invalid");
		break;	

    case ONVIF_ERR_CONFIG_MODIFY:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ConfigModify", "The configuration parameters are not possible to set");
		break;

	case ONVIF_ERR_CONFIGURATION_CONFLICT:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ConfigurationConflict", "The new settings conflicts with other uses of the configuration");
		break;

	case ONVIF_ERR_INVALID_POSIION:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidPosition", "Invalid Postion");
		break;	

	case ONVIF_ERR_TOO_MANY_SCOPES:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:TooManyScopes", "The requested scope list exceeds the supported number of scopes");
		break;

	case ONVIF_ERR_FIXED_SCOPE:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:FixedScope", "Trying to Remove fixed scope parameter, command rejected");
		break;

	case ONVIF_ERR_NO_SCOPE:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoScope", "Trying to Remove scope which does not exist");
		break;

	case ONVIF_ERR_SCOPE_OVERWRITE:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:ScopeOverwrite", "Scope Overwrite");
		break;

    case ONVIF_ERR_RESOURCE_UNKNOWN_FAULT:
        ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "wsrf-rw:ResourceUnknownFault", "ResourceUnknownFault");
        break;
        
	case ONVIF_ERR_NO_SOURCE:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter::NoSource", "The requested VideoSource does not exist");
		break;

	case ONVIF_ERR_CANNOT_OVERWRITE_HOME:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:CannotOverwriteHome", "The home position is fixed and cannot be overwritten");
		break;

	case ONVIF_ERR_SETTINGS_INVALID:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "The requested settings are incorrect");
		break;

	case ONVIF_ERR_NO_IMAGEING_FOR_SOURCE:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, "ter:NoImagingForSource", "The requested VideoSource does not support imaging settings");
		break;

	case ONVIF_ERR_USERNAME_EXIST:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:UsernameClash", "Username already exists");
		break;
		
	case ONVIF_ERR_PASSWORD_TOO_LONG:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:PasswordTooLong", "The password is too long");
		break;
		
	case ONVIF_ERR_USERNAME_TOO_LONG:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:UsernameTooLong", "The username is too long");
		break;
		
	case ONVIF_ERR_PASSWORD:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:Password", "Too weak password");
		break;
		
	case ONVIF_ERR_TOO_MANY_USERS:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_ACTION, "ter:TooManyUsers", "Maximum number of supported users exceeded");
		break;
		
	case ONVIF_ERR_ANONYMOUS_NOT_ALLOWED:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:AnonymousNotAllowed", "User level anonymous is not allowed");
		break;
		
	case ONVIF_ERR_USERNAME_TOO_SHORT:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:UsernameTooShort", "The username is too short");
		break;
		
	case ONVIF_ERR_USERNAME_MISSING:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:UsernameMissing", "Username not recognized");
		break;
		
	case ONVIF_ERR_FIXED_USER:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:FixedUser", "Username may not be deleted");
		break;

	case ONVIF_ERR_MAX_OSDS:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:MaxOSDs", "The maximum number of supported OSDs has been reached");
		break;

	case ONVIF_ERR_INVALID_STREAMSETUP:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidStreamSetup", "Specification of Stream Type or Transport part in StreamSetup is not supported");
		break;

	case ONVIF_ERR_BAD_CONFIGURATION:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:BadConfiguration", "The configuration is invalid");
		break;
		
	case ONVIF_ERR_MAX_RECORDING:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:MaxRecordings", "Max recordings");
		break;
		
	case ONVIF_ERR_NO_RECORDING:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoRecording", "The RecordingToken does not reference an existing recording");
		break;
		
	case ONVIF_ERR_CANNOT_DELETE:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:CannotDelete", "Can not delete");
		break;
		
	case ONVIF_ERR_MAX_TRACKS:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:MaxTracks", "Max tracks");
		break;
		
	case ONVIF_ERR_NO_TRACK:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoTrack", "The TrackToken does not reference an existing track of the recording");
		break;
		
	case ONVIF_ERR_MAX_RECORDING_JOBS:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:MaxRecordingJobs", "The maximum number of recording jobs that the device can handle has been reached");
		break;
		
	case ONVIF_ERR_MAX_RECEIVERS:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:MaxReceivers", "Max receivers");
		break;
		
	case ONVIF_ERR_NO_RECORDINGJOB:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoRecordingJob", "The JobToken does not reference an existing job");
		break;
		
	case ONVIF_ERR_BAD_MODE:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:BadMode", "The Mode is invalid");
		break;
		
	case ONVIF_ERR_INVALID_TOKEN:
		ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidToken", "The Token is not valid");
		break;

    case ONVIF_ERR_INVALID_RULE:
        break;
        
	case ONVIF_ERR_RULE_ALREADY_EXIST:
	    break;
        
	case ONVIF_ERR_TOO_MANY_RULES:
	    break;
        
	case ONVIF_ERR_RULE_NOT_EXIST:
	    break;
        
	case ONVIF_ERR_NAME_ALREADY_EXIST:
	    break;
        
	case ONVIF_ERR_TOO_MANY_MODULES:
	    break;
        
	case ONVIF_ERR_INVALID_MODULE:
	    break;
        
	case ONVIF_ERR_NAME_NOT_EXIST:
        break;
        
	case ONVIF_ERR_INVALID_FILTER_FAULT:
	    ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "wsntw:InvalidFilterFault", "InvalidFilterFault");
	    break;
        
	case ONVIF_ERR_INVALID_TOPIC_EXPRESSION_FAULT:
	    ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "wsntw:InvalidTopicExpressionFault", "InvalidTopicExpressionFault");
	    break;
        
	case ONVIF_ERR_TOPIC_NOT_SUPPORTED_FAULT:
	    ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "wsntw:TopicNotSupportedFault", "TopicNotSupportedFault");
	    break;
        
	case ONVIF_ERR_INVALID_MESSAGE_CONTENT_EXPRESSION_FAULT:
	    ret = soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "wsntw:InvalidMessageContentExpressionFault", "InvalidMessageContentExpressionFault");
	    break;
        
	default:
		ret = soap_err_def_rly(p_user, rx_msg);
		break;
	}

	return ret;
}


typedef int (*soap_build_xml)(char * p_buf, int mlen, const char * argv);

int soap_build_send_rly(HTTPCLN * p_user, HTTPMSG * rx_msg, soap_build_xml build_xml, const char * argv)
{
	int ret = -1, mlen = 1024*32, xlen;
	
	char * p_xml = g_reply_buf;//(char *)malloc(mlen);
	if (NULL == p_xml)
	{
		return -1;
	}
	
	xlen = build_xml(p_xml, mlen, argv);
	if (xlen < 0)
	{
		ret = soap_build_err_rly(p_user, rx_msg, (ONVIF_RET)xlen);
	}
	else
	{
		ret = soap_http_rly(p_user, rx_msg, p_xml, xlen);
	}	

	//free(p_xml);
	
	return ret;
}

int soap_GetDeviceInformation(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	printf("soap_GetDeviceInformation\r\n");
	GetDeviceInformation_RES res;
	ONVIF_RET ret;
	memset(&res, 0, sizeof(GetDeviceInformation_RES));
	ret = my_onvif_GetDeviceInformation(&res);
	if (ONVIF_OK == ret)
	{
		return soap_build_send_rly(p_user, rx_msg, build_GetDeviceInformation_rly_xml, (char *)&res); 
	}		 

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    XMLN * p_GetCapabilities;
    XMLN * p_Category;
	GetCapabilities_RES res;
	ONVIF_RET result;
	memset(&res, 0, sizeof(GetCapabilities_RES));
    printf("soap_GetCapabilities\r\n");

    p_GetCapabilities = xml_node_soap_get(p_body, "GetCapabilities");
    assert(p_GetCapabilities);
    
    p_Category = xml_node_soap_get(p_GetCapabilities, "Category");
	onvif_CapabilityCategory category = CapabilityCategory_All;
    if (p_Category && p_Category->data)
    {
    	category = onvif_StringToCapabilityCategory(p_Category->data);
	    if (CapabilityCategory_Invalid == category) 
	    {
	    	return soap_err_def2_rly(p_user, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, "ter:NoSuchService", "No Such Service");
	    }
    }

	res.Category = category;
	result = my_onvif_GetCapabilities(&res);
	if (ONVIF_OK == result)
	{
		return soap_build_send_rly(p_user, rx_msg, build_GetCapabilities_rly_xml, (char *)&res); 
	}
	
	return soap_err_def_rly(p_user, rx_msg);
}


int soap_GetProfiles(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	printf("soap_GetProfiles.\r\n");
	GetProfiles_RES res;
	int result = 0;
	ONVIF_RET ret;
	memset(&res, 0, sizeof(GetProfiles_RES));
	ret = my_onvif_GetProfiles(&res);
	if (ONVIF_OK == ret)
	{
		result = soap_build_send_rly(p_user, rx_msg, build_GetProfiles_rly_xml, (char *)&res); 
	}
	else
		result = soap_build_err_rly(p_user, rx_msg, ret);

	onvif_free_profiles_sev(&res.profiles);

	return result;
}


int soap_GetProfile(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    XMLN * p_GetProfile;
    XMLN * p_ProfileToken;
    
	printf("soap_GetProfile\r\n");

    p_GetProfile = xml_node_soap_get(p_body, "GetProfile");
    assert(p_GetProfile);

    p_ProfileToken = xml_node_soap_get(p_GetProfile, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		GetProfile_RES res;
		ONVIF_RET ret;
		int result = 0;
		memset(&res, 0, sizeof(GetProfile_RES));
		res.token = (char *)p_ProfileToken->data;
		ret = my_onvif_GetProfile(&res);
		if (ONVIF_OK == ret)
		{
			result = soap_build_send_rly(p_user, rx_msg, build_GetProfile_rly_xml, (char *)&res); 
		}
		else 
			result = soap_build_err_rly(p_user, rx_msg, ret);
		
		onvif_free_profile_sev(&res.profile);
		
		return result;
	}
	else
    {
    	return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attribute");
    }
}

int soap_CreateProfile(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    ONVIF_RET ret;
    XMLN * p_CreateProfile;
    CreateProfile_REQ req;
    
	printf("soap_CreateProfile\r\n");

	p_CreateProfile = xml_node_soap_get(p_body, "CreateProfile");
    assert(p_CreateProfile);
    
    memset(&req, 0, sizeof(req));

    ret = parse_CreateProfile(p_CreateProfile, &req);
    if (ONVIF_OK == ret)
    {
		CreateProfile_RES res;
		ONVIF_RET ret;
		int result =0;
		memset(&res, 0, sizeof(CreateProfile_RES));
		ret = my_onvif_CreateProfile(&req,&res);
		if (ONVIF_OK == ret)
		{
			result = soap_build_send_rly(p_user, rx_msg, build_CreateProfile_rly_xml, (char *)&res); 
		}
		else 
			result = soap_build_err_rly(p_user, rx_msg, ret);

		onvif_free_profile_sev(&res.struprofile);
	
		return result;
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_DeleteProfile(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    XMLN * p_DeleteProfile;
    XMLN * p_ProfileToken;
    ONVIF_RET ret = ONVIF_ERR_MISSINGATTR;
    
	printf("soap_DeleteProfile\r\n");

	p_DeleteProfile = xml_node_soap_get(p_body, "DeleteProfile");
    assert(p_DeleteProfile);
	
    p_ProfileToken = xml_node_soap_get(p_DeleteProfile, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
    	ret = onvif_DeleteProfile(p_ProfileToken->data);
    	if (ONVIF_OK == ret)
    	{
    		return soap_build_send_rly(p_user, rx_msg, build_DeleteProfile_rly_xml, NULL);
    	}    	
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_AddVideoSourceConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    XMLN * p_AddVideoSourceConfiguration;
    ONVIF_RET ret;
    AddVideoSourceConfiguration_REQ req;
    
	printf("soap_AddVideoSourceConfiguration\r\n");

	p_AddVideoSourceConfiguration = xml_node_soap_get(p_body, "AddVideoSourceConfiguration");
    assert(p_AddVideoSourceConfiguration);
	
	memset(&req, 0, sizeof(req));
	
    ret = parse_AddVideoSourceConfiguration(p_AddVideoSourceConfiguration, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_AddVideoSourceConfiguration(&req);
    	if (ONVIF_OK == ret)
    	{
    		return soap_build_send_rly(p_user, rx_msg, build_AddVideoSourceConfiguration_rly_xml, NULL);
    	} 	
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_RemoveVideoSourceConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    XMLN * p_RemoveVideoSourceConfiguration;
    XMLN * p_ProfileToken;
    ONVIF_RET ret = ONVIF_ERR_MISSINGATTR;
    
	printf("soap_RemoveVideoSourceConfiguration\r\n");
	
	p_RemoveVideoSourceConfiguration = xml_node_soap_get(p_body, "RemoveVideoSourceConfiguration");
	assert(p_RemoveVideoSourceConfiguration);
	
	p_ProfileToken = xml_node_soap_get(p_RemoveVideoSourceConfiguration, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
    	ret = onvif_RemoveVideoSourceConfiguration(p_ProfileToken->data);
    	if (ONVIF_OK == ret)
    	{
    		return soap_build_send_rly(p_user, rx_msg, build_RemoveVideoSourceConfiguration_rly_xml, NULL);
    	}
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_AddAudioSourceConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    ONVIF_RET ret;
    XMLN * p_AddAudioSourceConfiguration;
    AddAudioSourceConfiguration_REQ req;;
    
	printf("soap_AddAudioSourceConfiguration\r\n");

	p_AddAudioSourceConfiguration = xml_node_soap_get(p_body, "AddAudioSourceConfiguration");
    assert(p_AddAudioSourceConfiguration);

	memset(&req, 0, sizeof(req));
	
    ret = parse_AddAudioSourceConfiguration(p_AddAudioSourceConfiguration, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_AddAudioSourceConfiguration(&req);
    	if (ONVIF_OK == ret)
    	{
    		return soap_build_send_rly(p_user, rx_msg, build_AddAudioSourceConfiguration_rly_xml, NULL);
    	} 	
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_RemoveAudioSourceConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    XMLN * p_RemoveAudioSourceConfiguration;
    XMLN * p_ProfileToken;
    ONVIF_RET ret = ONVIF_ERR_MISSINGATTR;
    
	printf("soap_RemoveAudioSourceConfiguration\r\n");
	
	p_RemoveAudioSourceConfiguration = xml_node_soap_get(p_body, "RemoveAudioSourceConfiguration");
	assert(p_RemoveAudioSourceConfiguration);	
	
	p_ProfileToken = xml_node_soap_get(p_RemoveAudioSourceConfiguration, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
    	ret = onvif_RemoveAudioSourceConfiguration(p_ProfileToken->data);
    	if (ONVIF_OK == ret)
    	{
    		return soap_build_send_rly(p_user, rx_msg, build_RemoveAudioSourceConfiguration_rly_xml, NULL);
    	}
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}


int soap_AddVideoEncoderConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    ONVIF_RET ret;
    XMLN * p_AddVideoEncoderConfiguration;
    AddVideoEncoderConfiguration_REQ req;
    
	printf("soap_AddVideoEncoderConfiguration\r\n");

	p_AddVideoEncoderConfiguration = xml_node_soap_get(p_body, "AddVideoEncoderConfiguration");
	assert(p_AddVideoEncoderConfiguration);
	
	memset(&req, 0, sizeof(req));
	
	ret = parse_AddVideoEncoderConfiguration(p_AddVideoEncoderConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_AddVideoEncoderConfiguration(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_AddVideoEncoderConfiguration_rly_xml, NULL);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_RemoveVideoEncoderConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{	
    XMLN * p_RemoveVideoEncoderConfiguration;
    XMLN * p_ProfileToken;
    ONVIF_RET ret = ONVIF_ERR_MISSINGATTR;
    
	printf("soap_RemoveVideoEncoderConfiguration\r\n");
	
	p_RemoveVideoEncoderConfiguration = xml_node_soap_get(p_body, "RemoveVideoEncoderConfiguration");
	assert(p_RemoveVideoEncoderConfiguration);	
	
	p_ProfileToken = xml_node_soap_get(p_RemoveVideoEncoderConfiguration, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
    	ret = onvif_RemoveVideoEncoderConfiguration(p_ProfileToken->data);
    	if (ONVIF_OK == ret)
    	{
    		return soap_build_send_rly(p_user, rx_msg, build_RemoveVideoEncoderConfiguration_rly_xml, NULL);
    	}
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_AddAudioEncoderConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    ONVIF_RET ret;
    XMLN * p_AddAudioEncoderConfiguration;
    AddAudioEncoderConfiguration_REQ req;
    
	printf("soap_AddAudioEncoderConfiguration\r\n");

	p_AddAudioEncoderConfiguration = xml_node_soap_get(p_body, "AddAudioEncoderConfiguration");
	assert(p_AddAudioEncoderConfiguration);
	
	memset(&req, 0, sizeof(req));
	
	ret = parse_AddAudioEncoderConfiguration(p_AddAudioEncoderConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_AddAudioEncoderConfiguration(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_AddAudioEncoderConfiguration_rly_xml, NULL);
		}
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_RemoveAudioEncoderConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{	
    XMLN * p_RemoveAudioEncoderConfiguration;
    XMLN * p_ProfileToken;
    ONVIF_RET ret = ONVIF_ERR_MISSINGATTR;
    
	printf("soap_RemoveAudioEncoderConfiguration\r\n");
	
	p_RemoveAudioEncoderConfiguration = xml_node_soap_get(p_body, "RemoveAudioEncoderConfiguration");
	assert(p_RemoveAudioEncoderConfiguration);
	
	p_ProfileToken = xml_node_soap_get(p_RemoveAudioEncoderConfiguration, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
    	ret = onvif_RemoveAudioEncoderConfiguration(p_ProfileToken->data);
    	if (ONVIF_OK == ret)
    	{
    		return soap_build_send_rly(p_user, rx_msg, build_RemoveAudioEncoderConfiguration_rly_xml, NULL);
    	}
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}


int soap_GetSystemDateAndTime(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	printf("soap_GetSystemDateAndTime\r\n");

	GetSystemDateAndTime_RES res;
	ONVIF_RET ret;
	memset(&res, 0, sizeof(GetSystemDateAndTime_RES));
	ret = my_onvif_GetSystemDateAndTime(&res);
    if (ONVIF_OK == ret)
    {
    	return soap_build_send_rly(p_user, rx_msg, build_GetSystemDateAndTime_rly_xml, (char *)&res); 
    }        

    return soap_build_err_rly(p_user, rx_msg, ret);
}


int soap_GetStreamUri(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    ONVIF_RET ret;
    XMLN * p_GetStreamUri;
    GetStreamUri_REQ req;
    
	printf("soap_GetStreamUri\r\n");

    p_GetStreamUri = xml_node_soap_get(p_body, "GetStreamUri");
    assert(p_GetStreamUri);
	
	memset(&req, 0, sizeof(req));

	ret = parse_GetStreamUri(p_GetStreamUri, &req);
	if (ONVIF_OK == ret)
	{
		GetStreamUri_RES res;
		memset(&res, 0, sizeof(GetStreamUri_RES));
		ret = my_onvif_GetStreamUri(&req,&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetStreamUri_rly_xml, (char *)&res); 
		}		 
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}


int soap_GetSnapshotUri(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    XMLN * p_GetSnapshotUri;
    XMLN * p_ProfileToken;
    
    printf("soap_GetSnapshotUri\r\n");

    p_GetSnapshotUri = xml_node_soap_get(p_body, "GetSnapshotUri");
    assert(p_GetSnapshotUri);

    p_ProfileToken = xml_node_soap_get(p_GetSnapshotUri, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		GetSnapshotUri_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(GetSnapshotUri_RES));
		res.token = (char *)p_ProfileToken->data;
		ret = my_onvif_GetSnapshotUri(&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetSnapshotUri_rly_xml, (char *)&res); 
		}		 
	
		return soap_build_err_rly(p_user, rx_msg, ret);
	}
	
	return soap_err_def_rly(p_user, rx_msg);
}


int soap_GetNetworkInterfaces(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	printf("soap_GetNetworkInterfaces\r\n");
	GetNetworkInterfaces_RES res;
	ONVIF_RET ret;
	int result = 0;
	memset(&res, 0, sizeof(GetNetworkInterfaces_RES));
	ret = my_onvif_GetNetworkInterfaces(&res);
	if (ONVIF_OK == ret)
	{
		result = soap_build_send_rly(p_user, rx_msg, build_GetNetworkInterfaces_rly_xml, (char *)&res); 
	}
	else
	{
		result = soap_build_err_rly(p_user, rx_msg, ret);
	}
	
	ONVIF_NET_INF *p_free = res.interfaces;
	ONVIF_NET_INF *p_next = NULL;
	while(p_free)
	{
		p_next = NULL;
		if(p_free->next)
		{
			p_next = p_free->next;
		}
    free_one_item(p_free,ONVIF_TYPE_NET_INF);
		//free(p_free);
		p_free = p_next;
	}
	
	return result;
}

int soap_SetNetworkInterfaces(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    ONVIF_RET ret;
    XMLN * p_SetNetworkInterfaces;
    SetNetworkInterfaces_REQ req;
    
	printf("soap_SetNetworkInterfaces\r\n");

	p_SetNetworkInterfaces = xml_node_soap_get(p_body, "SetNetworkInterfaces");
    assert(p_SetNetworkInterfaces);
	
	memset(&req, 0, sizeof(SetNetworkInterfaces_REQ));
	
	ret = parse_SetNetworkInterfaces(p_SetNetworkInterfaces, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_SetNetworkInterfaces(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_SetNetworkInterfaces_rly_xml, NULL);
		}
	}
    
	return soap_build_err_rly(p_user, rx_msg, ret);
}


int soap_GetVideoSources(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	printf("soap_GetVideoSources\r\n");
	GetVideoSources_RES res;
	ONVIF_RET ret;
	int result = 0;
	memset(&res, 0, sizeof(GetVideoSources_RES));
	ret = my_onvif_GetVideoSources(&res);
	if (ONVIF_OK == ret)
	{
		result = soap_build_send_rly(p_user, rx_msg, build_GetVideoSources_rly_xml, (char *)&res); 
	}		 
	else 
		result = soap_build_err_rly(p_user, rx_msg, ret);

	ONVIF_V_SRC *p_free = res.p_v_src;
	ONVIF_V_SRC *p_next = NULL;
	while(p_free)
	{
		p_next = NULL;
		if(p_free->next)
		{
			p_next = p_free->next;
		}
    free_one_item(p_free,ONVIF_TYPE_V_SRC);
		//free(p_free);
		p_free = p_next;
	}

	return result;
}

int soap_GetAudioSources(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	printf("soap_GetAudioSources\r\n");
	GetAudioSources_RES res;
	int result = 0;
	ONVIF_RET ret;
	memset(&res, 0, sizeof(GetAudioSources_RES));
	ret = my_onvif_GetAudioSources(&res);
	if (ONVIF_OK == ret)
	{
		result = soap_build_send_rly(p_user, rx_msg, build_GetAudioSources_rly_xml, (char *)&res); 
	}
	else
		result = soap_build_err_rly(p_user, rx_msg, ret);
	
	ONVIF_A_SRC *p_free = res.a_src;
	ONVIF_A_SRC *p_next = NULL;
	while(p_free)
	{
		p_next = NULL;
		if(p_free->next)
		{
			p_next = p_free->next;
		}
    free_one_item(p_free,ONVIF_TYPE_A_SRC);
		//free(p_free);
		p_free = p_next;
	}
	return result;
}


int soap_GetVideoEncoderConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	printf("soap_GetVideoEncoderConfigurations\r\n");
	Get_v_enc_cfgs_RES res;
	ONVIF_RET ret;
	int result = 0;
	memset(&res, 0, sizeof(Get_v_enc_cfgs_RES));
	ret = my_onvif_GetVideoEncoderConfigurations(&res);
	if (ONVIF_OK == ret)
	{
		result = soap_build_send_rly(p_user, rx_msg, build_GetVideoEncoderConfigurations_rly_xml, (char *)&res); 
	}
	else
		result = soap_build_err_rly(p_user, rx_msg, ret);

	ONVIF_V_ENC_CFG *p_free = res.p_v_enc_cfg;
	ONVIF_V_ENC_CFG *p_next = NULL;
	while(p_free)
	{
		p_next = NULL;
		if(p_free->next)
		{
			p_next = p_free->next;
		}
    free_one_item(p_free,ONVIF_TYPE_V_ENC);
		//free(p_free);
		p_free = p_next;
	}

	return result;
}

int soap_GetCompatibleVideoEncoderConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    XMLN * p_GetCompatibleVideoEncoderConfigurations;
    XMLN * p_ProfileToken;
    
	printf("soap_GetCompatibleVideoEncoderConfigurations\r\n");

	p_GetCompatibleVideoEncoderConfigurations = xml_node_soap_get(p_body, "GetCompatibleVideoEncoderConfigurations");
    assert(p_GetCompatibleVideoEncoderConfigurations);

    p_ProfileToken = xml_node_soap_get(p_GetCompatibleVideoEncoderConfigurations, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
		GetCompatiblev_enc_cfgs_RES res;
		ONVIF_RET ret;
		int result = 0;
		memset(&res, 0, sizeof(GetCompatiblev_enc_cfgs_RES));
		res.token = (char *)p_ProfileToken->data;
		ret = my_onvif_GetCompatibleVideoEncoderConfigurations(&res);
		if (ONVIF_OK == ret)
		{
			result = soap_build_send_rly(p_user, rx_msg, build_GetCompatibleVideoEncoderConfigurations_rly_xml, (char *)&res); 
		}
		else 
			result = soap_build_err_rly(p_user, rx_msg, ret);
		ONVIF_V_ENC_CFG *p_free = res.p_v_enc_cfg;
		ONVIF_V_ENC_CFG *p_next = NULL;
		while(p_free)
		{
			p_next = NULL;
			if(p_free->next)
			{
				p_next = p_free->next;
			}
      free_one_item(p_free,ONVIF_TYPE_V_ENC);
			//free(p_free);
			p_free = p_next;
		}
		
		return result;
    }
	
    return soap_build_err_rly(p_user, rx_msg, ONVIF_ERR_MISSINGATTR);
}


int soap_GetAudioEncoderConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	printf("soap_GetAudioEncoderConfigurations\r\n");
	GetAudioEncoderCfgs_RES res;
	int result = 0;
	ONVIF_RET ret;
	memset(&res, 0, sizeof(GetAudioEncoderCfgs_RES));
	ret = my_onvif_GetAudioEncoderConfigurations(&res);
	if (ONVIF_OK == ret)
	{
		return result = soap_build_send_rly(p_user, rx_msg, build_GetAudioEncoderConfigurations_rly_xml, (char *)&res); 
	}
	else
		result = soap_build_err_rly(p_user, rx_msg, ret);
	ONVIF_A_ENC_CFG *p_free = res.a_enc_cfg;
	ONVIF_A_ENC_CFG *p_next = NULL;
	while(p_free)
	{
		p_next = NULL;
		if(p_free->next)
		{
			p_next = p_free->next;
		}
    free_one_item(p_free,ONVIF_TYPE_A_ENC);
		//free(p_free);
		p_free = p_next;
	}
	return result;
}

int soap_GetCompatibleAudioEncoderConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    XMLN * p_GetCompatibleAudioEncoderConfigurations;
    XMLN * p_ProfileToken;
    
	printf("soap_GetCompatibleAudioEncoderConfigurations\r\n");

	p_GetCompatibleAudioEncoderConfigurations = xml_node_soap_get(p_body, "GetCompatibleAudioEncoderConfigurations");
    assert(p_GetCompatibleAudioEncoderConfigurations);

    p_ProfileToken = xml_node_soap_get(p_GetCompatibleAudioEncoderConfigurations, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
		GetCompatibleA_Enc_Cfgs_RES res;
		ONVIF_RET ret;
		int result = 0;
		memset(&res, 0, sizeof(GetCompatibleA_Enc_Cfgs_RES));
		res.ProfileToken = (char *)p_ProfileToken->data;
		ret = my_onvif_GetCompatibleAudioEncoderConfigurations(&res);
		if (ONVIF_OK == ret)
		{
			result = soap_build_send_rly(p_user, rx_msg, build_GetCompatibleAudioEncoderConfigurations_rly_xml, (char *)&res); 
		}
		else 
			result = soap_build_err_rly(p_user, rx_msg, ret);
		ONVIF_A_ENC_CFG *p_free = res.a_enc_cfg;
		ONVIF_A_ENC_CFG *p_next = NULL;
		while(p_free)
		{
			p_next = NULL;
			if(p_free->next)
			{
				p_next = p_free->next;
			}
      free_one_item(p_free,ONVIF_TYPE_A_ENC);
			//free(p_free);
			p_free = p_next;
		}
		return result;
    }
	
    return soap_build_err_rly(p_user, rx_msg, ONVIF_ERR_MISSINGATTR);
}

int soap_GetVideoEncoderConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    XMLN * p_GetVideoEncoderConfiguration;
    XMLN * p_ConfigurationToken;
    
	printf("soap_GetVideoEncoderConfiguration\r\n");

    p_GetVideoEncoderConfiguration = xml_node_soap_get(p_body, "GetVideoEncoderConfiguration");
    assert(p_GetVideoEncoderConfiguration);

	p_ConfigurationToken = xml_node_soap_get(p_GetVideoEncoderConfiguration, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		Get_v_enc_cfg_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(Get_v_enc_cfg_RES));
		res.ConfigurationToken = (char *)p_ConfigurationToken->data;
		ret = my_onvif_GetVideoEncoderConfiguration(&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetVideoEncoderConfiguration_rly_xml, (char *)&res); 
		}		 
	
		return soap_build_err_rly(p_user, rx_msg, ret);
	}
	else 
	{
		return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attribute");
	}
}

int soap_GetAudioEncoderConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    XMLN * p_GetAudioEncoderConfiguration;
    XMLN * p_ConfigurationToken;
    
	printf("soap_GetAudioEncoderConfiguration\r\n");

    p_GetAudioEncoderConfiguration = xml_node_soap_get(p_body, "GetAudioEncoderConfiguration");
    assert(p_GetAudioEncoderConfiguration);

	p_ConfigurationToken = xml_node_soap_get(p_GetAudioEncoderConfiguration, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		GetAudioEncoderCfg_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(GetAudioEncoderCfg_RES));
		res.ConfigurationToken =(char *)p_ConfigurationToken->data;
		ret = my_onvif_GetAudioEncoderConfiguration(&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetAudioEncoderConfiguration_rly_xml, (char *)&res); 
		}		 
	
		return soap_build_err_rly(p_user, rx_msg, ret);
	}
	else 
	{
		return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attribute");
	}
}

int soap_SetAudioEncoderConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    ONVIF_RET ret;
    XMLN * p_SetAudioEncoderConfiguration;
    SetAudioEncoderConfiguration_REQ req;
    
	printf("soap_SetAudioEncoderConfiguration\r\n");

    p_SetAudioEncoderConfiguration = xml_node_soap_get(p_body, "SetAudioEncoderConfiguration");
    assert(p_SetAudioEncoderConfiguration);
	
	memset(&req, 0, sizeof(req));

	ret = parse_SetAudioEncoderConfiguration(p_SetAudioEncoderConfiguration, &req);
    if (ONVIF_OK == ret)
    {
        ret = onvif_SetAudioEncoderConfiguration(&req);
        if (ONVIF_OK == ret)
        {
        	return soap_build_send_rly(p_user, rx_msg, build_SetAudioEncoderConfiguration_rly_xml, NULL);
        }
    }
    
    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetVideoSourceConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	printf("soap_GetVideoSourceConfigurations.\r\n");
	Get_v_src_cfgs_RES res;
	ONVIF_RET ret;
	int result = 0;
	memset(&res, 0, sizeof(Get_v_src_cfgs_RES));
	ret = my_onvif_GetVideoSourceConfigurations(&res);
	if (ONVIF_OK == ret)
	{
		result = soap_build_send_rly(p_user, rx_msg, build_GetVideoSourceConfigurations_rly_xml, (char *)&res); 
	}
	else 
		result = soap_build_err_rly(p_user, rx_msg, ret);

	ONVIF_V_SRC_CFG *p_free = res.p_v_src_cfgs;
	ONVIF_V_SRC_CFG *p_next = NULL;
	while(p_free)
	{
		p_next = NULL;
		if(p_free->next)
		{
			p_next = p_free->next;
		}
    free_one_item(p_free,ONVIF_TYPE_V_SRC);
		//free(p_free);
		p_free = p_next;
	}

	return result;
}

int soap_GetVideoSourceConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    XMLN * p_GetVideoSourceConfiguration;
    XMLN * p_ConfigurationToken;
    
    printf("soap_GetVideoSourceConfiguration\r\n");

    p_GetVideoSourceConfiguration = xml_node_soap_get(p_body, "GetVideoSourceConfiguration");
    assert(p_GetVideoSourceConfiguration);

	p_ConfigurationToken = xml_node_soap_get(p_GetVideoSourceConfiguration, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		Get_v_src_cfg_RES res;
		ONVIF_RET ret;
		int result = 0;
		memset(&res, 0, sizeof(Get_v_src_cfg_RES));
		res.ConfigurationToken = (char *)p_ConfigurationToken->data;
		ret = my_onvif_GetVideoSourceConfiguration(&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetVideoSourceConfiguration_rly_xml, (char *)&res); 
		}
		
		return soap_build_err_rly(p_user, rx_msg, ret);
	}
	else 
	{
		return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attribute");
	}
}

int soap_SetVideoSourceConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    ONVIF_RET ret;
    XMLN * p_SetVideoSourceConfiguration;
    SetVideoSourceConfiguration_REQ req;
    
	printf("soap_SetVideoSourceConfiguration\r\n");

    p_SetVideoSourceConfiguration = xml_node_soap_get(p_body, "SetVideoSourceConfiguration");
    assert(p_SetVideoSourceConfiguration);
    
    memset(&req, 0, sizeof(req));

    ret = parse_SetVideoSourceConfiguration(p_SetVideoSourceConfiguration, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_SetVideoSourceConfiguration(&req);
    	if (ONVIF_OK == ret)
    	{
    		return soap_build_send_rly(p_user, rx_msg, build_SetVideoSourceConfiguration_rly_xml, NULL);
    	}
    }    

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetVideoSourceConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
    ONVIF_RET ret;
    XMLN * p_GetVideoSourceConfigurationOptions;
    GetVideoSourceConfigurationOptions_REQ req;
    
    printf("soap_GetVideoSourceConfigurationOptions\r\n");

    p_GetVideoSourceConfigurationOptions = xml_node_soap_get(p_body, "GetVideoSourceConfigurationOptions");
    assert(p_GetVideoSourceConfigurationOptions);
	
	memset(&req, 0, sizeof(req));
	
	ret = parse_GetVideoSourceConfigurationOptions(p_GetVideoSourceConfigurationOptions, &req);
	if (ONVIF_OK == ret)
	{
		Get_v_src_options_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(Get_v_src_options_RES));
		ret = my_onvif_GetVideoSourceConfigurationOptions(&req,&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetVideoSourceConfigurationOptions_rly_xml, (char *)&res); 
		}		 
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetAudioSourceConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	printf("soap_GetAudioSourceConfigurations\r\n");
	GetAudioSourceCfgs_RES res;
	ONVIF_RET ret;
	int result = 0;
	memset(&res, 0, sizeof(GetAudioSourceCfgs_RES));
	ret = my_onvif_GetAudioSourceConfigurations(&res);
	if (ONVIF_OK == ret)
	{
		result = soap_build_send_rly(p_user, rx_msg, build_GetAudioSourceConfigurations_rly_xml, (char *)&res); 
	}
	else 
		result = soap_build_err_rly(p_user, rx_msg, ret);
	
	ONVIF_A_SRC_CFG *p_free = res.a_src_cfg;
	ONVIF_A_SRC_CFG *p_next = NULL;
	while(p_free)
	{
		p_next = NULL;
		if(p_free->next)
		{
			p_next = p_free->next;
		}
    free_one_item(p_free,ONVIF_TYPE_A_SRC);
		//free(p_free);
		p_free = p_next;
	}
	
	return result;
}

int soap_GetCompatibleAudioSourceConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetCompatibleAudioSourceConfigurations;
	XMLN * p_ProfileToken;
		
	printf("soap_GetCompatibleAudioSourceConfigurations\r\n");

	p_GetCompatibleAudioSourceConfigurations = xml_node_soap_get(p_body, "GetCompatibleAudioSourceConfigurations");
    assert(p_GetCompatibleAudioSourceConfigurations);

    p_ProfileToken = xml_node_soap_get(p_GetCompatibleAudioSourceConfigurations, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		GetCompatibleA_Src_Cfgs_RES res;
		ONVIF_RET ret;
		int result = 0;
		memset(&res, 0, sizeof(GetCompatibleA_Src_Cfgs_RES));
		res.ProfileToken =(char *)p_ProfileToken->data;
		ret = my_onvif_GetCompatibleAudioSourceConfigurations(&res);
		if (ONVIF_OK == ret)
		{
			result = soap_build_send_rly(p_user, rx_msg, build_GetCompatibleAudioSourceConfigurations_rly_xml, (char *)&res); 
		}	
		else 
			result = soap_build_err_rly(p_user, rx_msg, ret);
		ONVIF_A_SRC_CFG *p_free = res.a_src_cfg;
		ONVIF_A_SRC_CFG *p_next = NULL;
		while(p_free)
		{
			p_next = NULL;
			if(p_free->next)
			{
				p_next = p_free->next;
			}
      free_one_item(p_free,ONVIF_TYPE_A_SRC);
			//free(p_free);
			p_free = p_next;
		}
	
		return result;
	}
	else 
	{
		return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attribute");
	}
}

int soap_GetAudioSourceConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_GetAudioSourceConfigurationOptions;
	GetAudioSourceConfigurationOptions_REQ req;
	
	printf("soap_GetAudioSourceConfigurationOptions\r\n");

	p_GetAudioSourceConfigurationOptions = xml_node_soap_get(p_body, "GetAudioSourceConfigurationOptions");
    assert(p_GetAudioSourceConfigurationOptions);
    
	memset(&req, 0, sizeof(req));
	
	ret = parse_GetAudioSourceConfigurationOptions(p_GetAudioSourceConfigurationOptions, &req);
	if (ONVIF_OK == ret)
	{
		GetA_Src_Options_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(GetA_Src_Options_RES));
		ret = my_onvif_GetAudioSourceConfigurationOptions(&req,&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetAudioSourceConfigurationOptions_rly_xml, (char *)&res); 
		}		 
	
		return soap_build_err_rly(p_user, rx_msg, ret);
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetAudioSourceConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetAudioSourceConfiguration;
	XMLN * p_ConfigurationToken;
	
    printf("soap_GetAudioSourceConfiguration\r\n");

    p_GetAudioSourceConfiguration = xml_node_soap_get(p_body, "GetAudioSourceConfiguration");
    assert(p_GetAudioSourceConfiguration);

	p_ConfigurationToken = xml_node_soap_get(p_GetAudioSourceConfiguration, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		GetAudioSourceCfg_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(GetAudioSourceCfg_RES));
		res.ConfigurationToken =(char *)p_ConfigurationToken->data;
		ret = my_onvif_GetAudioSourceConfiguration(&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetAudioSourceConfiguration_rly_xml, (char *)&res); 
		}		 
	
		return soap_build_err_rly(p_user, rx_msg, ret);
	}
	else 
	{
		return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attribute");
	}
}

int soap_SetAudioSourceConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetAudioSourceConfiguration;
	SetAudioSourceConfiguration_REQ req;
	
	printf("soap_SetAudioSourceConfiguration\r\n");

    p_SetAudioSourceConfiguration = xml_node_soap_get(p_body, "SetAudioSourceConfiguration");
    assert(p_SetAudioSourceConfiguration);
    
    memset(&req, 0, sizeof(req));

    ret = parse_SetAudioSourceConfiguration(p_SetAudioSourceConfiguration, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_SetAudioSourceConfiguration(&req);
    	if (ONVIF_OK == ret)
    	{
    		return soap_build_send_rly(p_user, rx_msg, build_SetAudioSourceConfiguration_rly_xml, NULL);
    	}
    }    

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetVideoEncoderConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_GetVideoEncoderConfigurationOptions;
	GetVideoEncoderConfigurationOptions_REQ req;
	
    printf("soap_GetVideoEncoderConfigurationOptions\r\n");

	p_GetVideoEncoderConfigurationOptions = xml_node_soap_get(p_body, "GetVideoEncoderConfigurationOptions");
    assert(p_GetVideoEncoderConfigurationOptions);
    
    memset(&req, 0, sizeof(req));

	ret = parse_GetVideoEncoderConfigurationOptions(p_GetVideoEncoderConfigurationOptions, &req);
	if (ONVIF_OK == ret)
	{
		Get_v_enc_options_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(Get_v_enc_options_RES));
		ret = my_onvif_GetVideoEncoderConfigurationOptions(&req,&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetVideoEncoderConfigurationOptions_rly_xml, (char *)&res); 
		}		 
	}

    return soap_build_err_rly(p_user, rx_msg, ret);    
}

int soap_GetAudioEncoderConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_GetAudioEncoderConfigurationOptions;
	GetAudioEncoderConfigurationOptions_REQ req;
	
    printf("soap_GetAudioEncoderConfigurationOptions\r\n");

	p_GetAudioEncoderConfigurationOptions = xml_node_soap_get(p_body, "GetAudioEncoderConfigurationOptions");
    assert(p_GetAudioEncoderConfigurationOptions);    
	
    memset(&req, 0, sizeof(req));

	ret = parse_GetAudioEncoderConfigurationOptions(p_GetAudioEncoderConfigurationOptions, &req);
	if (ONVIF_OK == ret)
	{
		GetA_Enc_Options_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(GetA_Enc_Options_RES));
		ret = my_onvif_GetAudioEncoderConfigurationOptions(&req,&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetAudioEncoderConfigurationOptions_rly_xml, (char *)&res); 
		}		 
	
		return soap_build_err_rly(p_user, rx_msg, ret);
	}

    return soap_build_err_rly(p_user, rx_msg, ret);  
}

int soap_GetCompatibleVideoSourceConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetCompatibleVideoSourceConfigurations;
	XMLN * p_ProfileToken;
	
	printf("soap_GetCompatibleVideoSourceConfigurations\r\n");

    p_GetCompatibleVideoSourceConfigurations = xml_node_soap_get(p_body, "GetCompatibleVideoSourceConfigurations");
    assert(p_GetCompatibleVideoSourceConfigurations);

	p_ProfileToken = xml_node_soap_get(p_GetCompatibleVideoSourceConfigurations, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		GetCompatiblev_src_cfgs_RES res;
		ONVIF_RET ret;
		int result = 0;
		memset(&res, 0, sizeof(GetCompatiblev_src_cfgs_RES));
		res.token = (char *)p_ProfileToken->data;
		ret = my_onvif_GetCompatibleVideoSourceConfigurations(&res);
		if (ONVIF_OK == ret)
		{
			result = soap_build_send_rly(p_user, rx_msg, build_GetCompatibleVideoSourceConfigurations_rly_xml, (char *)&res); 
		}		 
		else
			result = soap_build_err_rly(p_user, rx_msg, ret);
		ONVIF_V_SRC_CFG *p_free = res.p_v_src_cfgs;
		ONVIF_V_SRC_CFG *p_next = NULL;
		while(p_free)
		{
			p_next = NULL;
			if(p_free->next)
			{
				p_next = p_free->next;
			}
      free_one_item(p_free,ONVIF_TYPE_V_SRC);
			//free(p_free);
			p_free = p_next;
		}
		
		return result;
	}
	else 
	{
		return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attribute");
	}
}

int soap_SetVideoEncoderConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetVideoEncoderConfiguration;
	SetVideoEncoderConfiguration_REQ req;
	
    printf("soap_SetVideoEncoderConfiguration\r\n");

    p_SetVideoEncoderConfiguration = xml_node_soap_get(p_body, "SetVideoEncoderConfiguration");
    assert(p_SetVideoEncoderConfiguration);
	
	memset(&req, 0, sizeof(req));

	ret = parse_SetVideoEncoderConfiguration(p_SetVideoEncoderConfiguration, &req);
    if (ONVIF_OK == ret)
    {
        ret = onvif_SetVideoEncoderConfiguration(&req);
        if (ONVIF_OK == ret)
        {
        	return soap_build_send_rly(p_user, rx_msg, build_SetVideoEncoderConfiguration_rly_xml, NULL);
        }
    }
    
    return soap_build_err_rly(p_user, rx_msg, ret);
}


int soap_SystemReboot(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	int ret;
	
    printf("soap_SystemReboot\r\n");

	ret = soap_build_send_rly(p_user, rx_msg, build_SystemReboot_rly_xml, NULL);
	
	onvif_SystemReboot();

    return ret;
}

int soap_SetSystemFactoryDefault(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	int ret;
	int type = 0;
	XMLN * p_SetSystemFactoryDefault;
	XMLN * p_FactoryDefault;
	
	printf("soap_SetSystemFactoryDefault\r\n");

	p_SetSystemFactoryDefault = xml_node_soap_get(p_body, "SetSystemFactoryDefault");
    assert(p_SetSystemFactoryDefault);	
	
    p_FactoryDefault = xml_node_soap_get(p_SetSystemFactoryDefault, "FactoryDefault");
    if (p_FactoryDefault && p_FactoryDefault->data)
    {
    	if (strcasecmp(p_FactoryDefault->data, "Hard") == 0)
    	{
    		type = 1;
    	}
    }

    ret = soap_build_send_rly(p_user, rx_msg, build_SetSystemFactoryDefault_rly_xml, NULL);
	
	onvif_SetSystemFactoryDefault(type);

    return ret;
}

int soap_GetSystemLog(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	int type = 0;
	XMLN * p_GetSystemLog;
	XMLN * p_LogType;
	
	printf("soap_GetSystemLog\r\n");

	p_GetSystemLog = xml_node_soap_get(p_body, "GetSystemLog");
    assert(p_GetSystemLog);    
	
    p_LogType = xml_node_soap_get(p_GetSystemLog, "LogType");
    if (p_LogType && p_LogType->data)
    {
    	if (strcasecmp(p_LogType->data, "Access") == 0)
    	{
    		type = 1;
    	}
    }

    return soap_build_send_rly(p_user, rx_msg, build_GetSystemLog_rly_xml, (char *)(long)type);
}

int soap_SetSystemDateAndTime(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetSystemDateAndTime;
	SetSystemDateAndTime_REQ req;
	
    printf("soap_SetSystemDateAndTime\r\n");

    p_SetSystemDateAndTime = xml_node_soap_get(p_body, "SetSystemDateAndTime");
    assert(p_SetSystemDateAndTime);
	
	memset(&req, 0, sizeof(SetSystemDateAndTime_REQ));

	ret = parse_SetSystemDateAndTime(p_SetSystemDateAndTime, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_SetSystemDateAndTime(&req);
        if (ONVIF_OK == ret)
        {
        	return soap_build_send_rly(p_user, rx_msg, build_SetSystemDateAndTime_rly_xml, NULL); 
        }        
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}


int soap_GetServices(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	BOOL bIncludeCapability = FALSE;
	XMLN * p_GetServices;
	XMLN * p_IncludeCapability;
	GetServices_RES res;
	ONVIF_RET ret;
	memset(&res, 0, sizeof(GetServices_RES));
    printf("soap_GetServices\r\n");

    p_GetServices = xml_node_soap_get(p_body, "tds:GetServices");
    assert(p_GetServices);
    
    p_IncludeCapability = xml_node_soap_get(p_GetServices, "IncludeCapability");
    if (p_IncludeCapability && p_IncludeCapability->data)
    {
        bIncludeCapability = parse_Bool(p_IncludeCapability->data);
    }
	res.bIncludeCapability = bIncludeCapability;
	ret = my_onvif_GetServices(&res);
	if (ONVIF_OK == ret)
	{
		return soap_build_send_rly(p_user, rx_msg, build_GetServices_rly_xml, (char *)&res); 
	}		 

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_Subscribe(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_Subscribe;
	Subscribe_REQ req;
	
    printf("soap_Subscribe\r\n");

    p_Subscribe = xml_node_soap_get(p_body, "Subscribe");
    assert(p_Subscribe);
	
	memset(&req, 0, sizeof(req));

	ret = parse_Subscribe(p_Subscribe, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_Subscribe(&req);
	    if (ONVIF_OK == ret)
	    {
			return soap_build_send_rly(p_user, rx_msg, build_Subscribe_rly_xml, (char *)req.p_eua); 
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_Unsubscribe(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	XMLN * p_Unsubscribe;
	XMLN * p_To;
	
    printf("soap_Unsubscribe\r\n");

    p_Unsubscribe = xml_node_soap_get(p_body, "Unsubscribe");
    assert(p_Unsubscribe);

    p_To = xml_node_soap_get(p_header, "To");
	if (p_To && p_To->data)
	{
		ONVIF_RET ret = onvif_Unsubscribe(p_To->data);		
	    if (ONVIF_OK == ret)
	    {
	        return soap_build_send_rly(p_user, rx_msg, build_Unsubscribe_rly_xml, NULL);
	    }
	    else
	    {
	    	return soap_build_err_rly(p_user, rx_msg, ret);
	    }
	}
	
	return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attibute");
}

int soap_Renew(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_Renew;
	XMLN * p_To;
	Renew_REQ req;
	
	printf("soap_Renew\r\n");

	p_Renew = xml_node_soap_get(p_body, "Renew");
	assert(p_Renew);
	
	memset(&req, 0, sizeof(req));

	p_To = xml_node_soap_get(p_header, "To");
	if (p_To && p_To->data)
	{
		strncpy(req.ProducterReference, p_To->data, sizeof(req.ProducterReference)-1);
	}
	
	ret = parse_Renew(p_Renew, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_Renew(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_Renew_rly_xml, NULL);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_CreatePullPointSubscription(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_CreatePullPointSubscription;
	CreatePullPointSubscription_REQ req;
	
	printf("soap_CreatePullPointSubscription\r\n");

	p_CreatePullPointSubscription = xml_node_soap_get(p_body, "CreatePullPointSubscription");
	assert(p_CreatePullPointSubscription);

	memset(&req, 0, sizeof(req));

	ret = parse_CreatePullPointSubscription(p_CreatePullPointSubscription, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_CreatePullPointSubscription(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_CreatePullPointSubscription_rly_xml, (char *)req.p_eua);
		}
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_PullMessages(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body, XMLN * p_header)
{
	ONVIF_RET ret;
	XMLN * p_PullMessages;
	PullMessages_REQ req;
	
	printf("soap_PullMessages\r\n");

	p_PullMessages = xml_node_soap_get(p_body, "PullMessages");
	assert(p_PullMessages);

	memset(&req, 0, sizeof(req));

	sscanf(rx_msg->first_line.value_string, "/event_service/%u", &req.eua_idx);
	
	ret = parse_PullMessages(p_PullMessages, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_PullMessages(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_PullMessages_rly_xml, (char *)&req);
		}
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_SetSynchronizationPoint(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	printf("soap_SetSynchronizationPoint\r\n");
	onvif_SetSynchronizationPoint(1);
    return soap_build_send_rly(p_user, rx_msg, build_SetSynchronizationPoint_rly_xml, NULL);
}

int soap_GetScopes(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
    printf("soap_GetScopes\r\n");
	GetScopes_RES res;
	ONVIF_RET ret;
	memset(&res, 0, sizeof(GetScopes_RES));
	ret = my_onvif_GetScopes(&res);
	if (ONVIF_OK == ret)
	{
		return soap_build_send_rly(p_user, rx_msg, build_GetScopes_rly_xml, (char *)&res); 
	}		 

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_AddScopes(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_AddScopes;
	ONVIF_SCOPE scopes[MAX_SCOPE_NUMS];
	
    printf("soap_AddScopes\r\n");

    p_AddScopes = xml_node_soap_get(p_body, "AddScopes");
    assert(p_AddScopes);
    
    memset(scopes, 0, sizeof(ONVIF_SCOPE) * MAX_SCOPE_NUMS);
    
    ret = parse_AddScopes(p_AddScopes, scopes, MAX_SCOPE_NUMS);
    if (ONVIF_OK == ret)
    {
        ret = onvif_add_scopes(scopes, MAX_SCOPE_NUMS);
        if (ONVIF_OK == ret)
        {
    		return soap_build_send_rly(p_user, rx_msg, build_AddScopes_rly_xml, NULL);
    	}
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_SetScopes(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetScopes;
	ONVIF_SCOPE scopes[MAX_SCOPE_NUMS];
	
	printf("soap_SetScopes\r\n");

    p_SetScopes = xml_node_soap_get(p_body, "SetScopes");
    assert(p_SetScopes);
    
    memset(scopes, 0, sizeof(ONVIF_SCOPE) * MAX_SCOPE_NUMS);
    
    ret = parse_SetScopes(p_SetScopes, scopes, MAX_SCOPE_NUMS);
    if (ONVIF_OK == ret)
    {
        ret = onvif_set_scopes(scopes, MAX_SCOPE_NUMS);
        if (ONVIF_OK == ret)
        {
    		return soap_build_send_rly(p_user, rx_msg, build_SetScopes_rly_xml, NULL);
    	}
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_RemoveScopes(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_RemoveScopes;
	ONVIF_SCOPE scopes[MAX_SCOPE_NUMS];
	
	printf("soap_RemoveScopes\r\n");

    p_RemoveScopes = xml_node_soap_get(p_body, "RemoveScopes");
    assert(p_RemoveScopes);
	
    memset(scopes, 0, sizeof(ONVIF_SCOPE) * MAX_SCOPE_NUMS);
    
    ret = parse_AddScopes(p_RemoveScopes, scopes, MAX_SCOPE_NUMS);
    if (ONVIF_OK == ret)
    {
        ret = onvif_remove_scopes(scopes, MAX_SCOPE_NUMS);
        if (ONVIF_OK == ret)
        {
    		return soap_build_send_rly(p_user, rx_msg, build_RemoveScopes_rly_xml, (char *)scopes);
    	}
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetHostname(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
    printf("soap_GetHostname\r\n");
{

	GetHostname_RES res;
	ONVIF_RET ret;
	memset(&res, 0, sizeof(GetHostname_RES));
	ret = my_onvif_GetHostname(&res);
	if (ONVIF_OK == ret)
	{
		return soap_build_send_rly(p_user, rx_msg, build_GetHostname_rly_xml, (char *)&res); 
	}		 

	return soap_build_err_rly(p_user, rx_msg, ret);
}

    return soap_build_send_rly(p_user, rx_msg, build_GetHostname_rly_xml, NULL); 
}

int soap_SetHostname(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_SetHostname;
	XMLN * p_Name;
	
    printf("soap_SetHostname\r\n");

    p_SetHostname = xml_node_soap_get(p_body, "SetHostname");
    assert(p_SetHostname);

    p_Name = xml_node_soap_get(p_SetHostname, "Name");
	if (p_Name)
	{
	    ONVIF_RET ret;
	    
	    if (p_Name->data)
	    {
		    ret = onvif_SetHostname(p_Name->data, FALSE);
	    }
	    else
	    {
	        ret = onvif_SetHostname(NULL, TRUE);
	    }

	    if (ONVIF_OK == ret)
	    {
	        return soap_build_send_rly(p_user, rx_msg, build_SetHostname_rly_xml, NULL);
	    }
	    else if (ONVIF_ERR_INVALID_HOSTNAME == ret)
	    {
	    	return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidHostname", "Invalid Hostname");
	    }
	}
	
    return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidHostname", "Invalid ArgVal"); 
}

int soap_SetHostnameFromDHCP(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_SetHostname;
	XMLN * p_FromDHCP;
	
    printf("soap_SetHostnameFromDHCP\r\n");

    p_SetHostname = xml_node_soap_get(p_body, "SetHostnameFromDHCP");
    assert(p_SetHostname);

    p_FromDHCP = xml_node_soap_get(p_SetHostname, "FromDHCP");
	if (p_FromDHCP && p_FromDHCP->data)
	{
	    ONVIF_RET ret;
		Onvif_Info* pOnvifInfo = GetOnvifInfo();
	    if (strcasecmp(p_FromDHCP->data, "true") == 0)
	    {
	    	ret = onvif_SetHostname(NULL, TRUE);
	    }
	    else
	    {
	        ret = onvif_SetHostname(pOnvifInfo->hostname.name, FALSE);
	    }

	    if (ONVIF_OK == ret)
	    {
	        return soap_build_send_rly(p_user, rx_msg, build_SetHostnameFromDHCP_rly_xml,  (char *)&pOnvifInfo->hostname.RebootNeeded);
	    }
	    else if (ONVIF_ERR_INVALID_HOSTNAME == ret)
	    {
	    	return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidHostname", "Invalid Hostname");
	    }
	}
	
    return soap_err_def_rly(p_user, rx_msg); 
}


int soap_GetNetworkProtocols(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
    printf("soap_GetNetworkProtocols\r\n");
	GetNetworkProtocols_RES res;
	ONVIF_RET ret;
	memset(&res, 0, sizeof(GetNetworkProtocols_RES));
	ret = my_onvif_GetNetworkProtocols(&res);
	if (ONVIF_OK == ret)
	{
		return soap_build_send_rly(p_user, rx_msg, build_GetNetworkProtocols_rly_xml, (char *)&res); 
	}		 

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_SetNetworkProtocols(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetNetworkProtocols;
	SetNetworkProtocols_REQ req;
	
    printf("soap_SetNetworkProtocols\r\n");

    p_SetNetworkProtocols = xml_node_soap_get(p_body, "SetNetworkProtocols");
    assert(p_SetNetworkProtocols);
	
	memset(&req, 0, sizeof(req));

	ret = parse_SetNetworkProtocols(p_SetNetworkProtocols, &req);
    if (ONVIF_OK == ret)
    {    
    	ret = onvif_SetNetworkProtocols(&req);
    	if (ONVIF_OK == ret)
    	{
    		return soap_build_send_rly(p_user, rx_msg, build_SetNetworkProtocols_rly_xml, NULL); 
    	}
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetNetworkDefaultGateway(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
    printf("soap_GetNetworkDefaultGateway\r\n");
	GetNetworkDefaultGateway_RES res;
	ONVIF_RET ret;
	memset(&res, 0, sizeof(GetNetworkDefaultGateway_RES));
	ret = my_onvif_GetNetworkDefaultGateway(&res);
	if (ONVIF_OK == ret)
	{
		return soap_build_send_rly(p_user, rx_msg, build_GetNetworkDefaultGateway_rly_xml, (char *)&res); 
	}		 

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_SetNetworkDefaultGateway(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetNetworkDefaultGateway;
	SetNetworkDefaultGateway_REQ req;
	
    printf("soap_SetNetworkDefaultGateway\r\n");

	p_SetNetworkDefaultGateway = xml_node_soap_get(p_body, "SetNetworkDefaultGateway");
    assert(p_SetNetworkDefaultGateway);
	
	memset(&req, 0, sizeof(SetNetworkDefaultGateway_REQ));

	ret = parse_SetNetworkDefaultGateway(p_SetNetworkDefaultGateway, &req);
    if (ONVIF_OK == ret)
    {    
    	ret = onvif_SetNetworkDefaultGateway(&req);
    	if (ONVIF_OK == ret)
    	{
    		return soap_build_send_rly(p_user, rx_msg, build_SetNetworkDefaultGateway_rly_xml, NULL); 
    	}
    }
    	
    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetDiscoveryMode(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
    printf("soap_GetDiscoveryMode.\r\n");
	GetDiscoveryMode_RES res;
	ONVIF_RET ret;
	memset(&res, 0, sizeof(GetDiscoveryMode_RES));
	ret = my_onvif_GetDiscoveryMode(&res);
	if (ONVIF_OK == ret)
	{
		return soap_build_send_rly(p_user, rx_msg, build_GetDiscoveryMode_rly_xml, (char *)&res); 
	}		 

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_SetDiscoveryMode(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetDiscoveryMode;
	SetDiscoveryMode_REQ req;
	
    printf("soap_SetDiscoveryMode\r\n");

    p_SetDiscoveryMode = xml_node_soap_get(p_body, "SetDiscoveryMode");
    assert(p_SetDiscoveryMode);
	
	memset(&req, 0, sizeof(SetDiscoveryMode_REQ));

	ret = parse_SetDiscoveryMode(p_SetDiscoveryMode, &req);
	if (ONVIF_OK == ret)
	{
	    ret = onvif_SetDiscoveryMode(&req);
	    if (ONVIF_OK == ret)
	    {
			return soap_build_send_rly(p_user, rx_msg, build_SetDiscoveryMode_rly_xml, NULL); 
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetDNS(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	printf("soap_GetDNS\r\n");
	GetDNS_RES res;
	ONVIF_RET ret;
	memset(&res, 0, sizeof(GetDNS_RES));
	ret = my_onvif_GetDNS(&res);
	if (ONVIF_OK == ret)
	{
		return soap_build_send_rly(p_user, rx_msg, build_GetDNS_rly_xml, (char *)&res); 
	}		 

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_SetDNS(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetDNS;
	SetDNS_REQ req;
	
	printf("soap_SetDNS\r\n");

	p_SetDNS = xml_node_soap_get(p_body, "SetDNS");
    assert(p_SetDNS);
    
    memset(&req, 0, sizeof(SetDNS_REQ));

	ret = parse_SetDNS(p_SetDNS, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_SetDNS(&req);
    	if (ONVIF_OK == ret)
    	{
    		return soap_build_send_rly(p_user, rx_msg, build_SetDNS_rly_xml, NULL); 
    	}
    }
    
    return soap_build_err_rly(p_user, rx_msg, ret);
}


int soap_GetNTP(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	printf("soap_GetNTP\r\n");
	GetNTP_RES res;
	ONVIF_RET ret;
	memset(&res, 0, sizeof(GetNTP_RES));
	ret = my_onvif_GetNTP(&res);
	if (ONVIF_OK == ret)
	{
		return soap_build_send_rly(p_user, rx_msg, build_GetNTP_rly_xml, (char *)&res); 
	}		 

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_SetNTP(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetNTP;
	SetNTP_REQ req;
	
	printf("soap_SetNTP\r\n");

	p_SetNTP = xml_node_soap_get(p_body, "SetNTP");
    assert(p_SetNTP);
    
    memset(&req, 0, sizeof(SetNTP_REQ));

	ret = parse_SetNTP(p_SetNTP, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_SetNTP(&req);
    	if (ONVIF_OK == ret)
    	{
    		return soap_build_send_rly(p_user, rx_msg, build_SetNTP_rly_xml, NULL); 
    	}
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}


int soap_GetServiceCapabilities(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	char * post;
	XMLN * p_GetServiceCapabilities;
	onvif_CapabilityCategory category;
	GetCapabilities_RES res;
	ONVIF_RET ret;
	memset(&res, 0, sizeof(GetCapabilities_RES));
	printf("soap_GetServiceCapabilities\r\n");

    p_GetServiceCapabilities = xml_node_soap_get(p_body, "GetServiceCapabilities");
    assert(p_GetServiceCapabilities);    
	
	post = rx_msg->first_line.value_string;
	if (NULL == post)
	{
		category = CapabilityCategory_Device;
	}
	else if (strcasestr(post, "media"))
	{
		category = CapabilityCategory_Media;
	}
	else if (strcasestr(post, "device"))
	{
		category = CapabilityCategory_Device;
	}
	else if (strcasestr(post, "imaging"))
	{
		category = CapabilityCategory_Imaging;
	}
	else if (strcasestr(post, "ptz"))
	{
		category = CapabilityCategory_PTZ;
	}
	else if (strcasestr(post, "event"))
	{
		category = CapabilityCategory_Events;
	}
#ifdef VIDEO_ANALYTICS	
	else if (strcasestr(post, "analytics"))
	{
		category = CapabilityCategory_Analytics;
	}
#endif
#ifdef PROFILE_G_SUPPORT
	else if (strcasestr(post, "recording"))
	{
		category = CapabilityCategory_Recording;
	}
	else if (strcasestr(post, "search"))
	{
		category = CapabilityCategory_Search;
	}
	else if (strcasestr(post, "replay"))
	{
		category = CapabilityCategory_Replay;
	}
#endif
	else
	{
		category = CapabilityCategory_Device;
	}

	res.Category = category;
	ret = my_onvif_GetCapabilities(&res);
	if (ONVIF_OK == ret)
	{
		return soap_build_send_rly(p_user, rx_msg, build_GetServiceCapabilities_rly_xml, (char *)&res); 
	}		 

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetEventProperties(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	printf("soap_GetEventProperties.\r\n");

	return soap_build_send_rly(p_user, rx_msg, build_GetEventProperties_rly_xml, NULL);
}

int soap_GetWsdlUrl(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	printf("soap_GetWsdlUrl.\r\n");

	return soap_build_send_rly(p_user, rx_msg, build_GetWsdlUrl_rly_xml, NULL);
}

int soap_GetNodes(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	printf("soap_GetNodes.\r\n");
	int result = 0;
	GetNodes_RES res;
	ONVIF_RET ret;
	memset(&res, 0, sizeof(GetNodes_RES));
	ret = my_onvif_GetNodes(&res);
	if (ONVIF_OK == ret)
	{
		result = soap_build_send_rly(p_user, rx_msg, build_GetNodes_rly_xml, (char *)&res); 
	}
	else
	{
		result = soap_build_err_rly(p_user, rx_msg, ret);
	}

	PTZ_NODE *p_free = res.p_node;
	PTZ_NODE *p_next = NULL;
	while(p_free)
	{
		p_next = NULL;
		if(p_free->next)
		{
			p_next = p_free->next;
		}
    free_one_item(p_free,ONVIF_TYPE_PTZ_NODE);
		//free(p_free);
		p_free = p_next;
	}

	return result;
}

int soap_GetNode(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetNode;
	XMLN * p_NodeToken;
		
	printf("soap_GetNode.\r\n");

	p_GetNode = xml_node_soap_get(p_body, "GetNode");
	assert(p_GetNode);
	
    p_NodeToken = xml_node_soap_get(p_GetNode, "tptz:NodeToken");
	if (p_NodeToken && p_NodeToken->data)
	{
		GetNode_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(GetNode_RES));
		ret = my_onvif_GetNode(p_NodeToken->data,&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetNode_rly_xml, (char *)&res); 
		}		 
	
		return soap_build_err_rly(p_user, rx_msg, ret);
	}
	
	return soap_err_def_rly(p_user, rx_msg);
}

int soap_GetConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	printf("soap_GetConfigurations.\r\n");
	GetPTZCfgs_RES res;
	ONVIF_RET ret;
	int result = 0;
	memset(&res, 0, sizeof(GetPTZCfgs_RES));
	ret = my_onvif_GetPTZConfigurations(&res);
	if (ONVIF_OK == ret)
	{
		result = soap_build_send_rly(p_user, rx_msg, build_GetConfigurations_rly_xml, (char *)&res); 
	}
	else
	{
		result = soap_build_err_rly(p_user, rx_msg, ret);
	}

	PTZ_CFG *p_free = res.p_PTZCfg;
	PTZ_CFG *p_next = NULL;
	while(p_free)
	{
		p_next = NULL;
		if(p_free->next)
		{
			p_next = p_free->next;
		}
    free_one_item(p_free,ONVIF_TYPE_PTZ);
		//free(p_free);
		p_free = p_next;
	}

	return result;
}

int soap_GetConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetConfiguration;
	XMLN * p_PTZConfigurationToken;
	
	printf("soap_GetConfiguration.\r\n");

	p_GetConfiguration = xml_node_soap_get(p_body, "GetConfiguration");
	assert(p_GetConfiguration);
	
	p_PTZConfigurationToken = xml_node_soap_get(p_GetConfiguration, "tptz:PTZConfigurationToken");
	if (p_PTZConfigurationToken && p_PTZConfigurationToken->data)
	{
		GetPTZCfg_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(GetPTZCfg_RES));
		ret = my_onvif_GetPTZConfiguration(p_PTZConfigurationToken->data,&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetConfiguration_rly_xml, (char *)&res); 
		}		 
	
		return soap_build_err_rly(p_user, rx_msg, ret);
	}
	
	return soap_err_def_rly(p_user, rx_msg);
}

int soap_AddPTZConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_AddPTZConfiguration;
	AddPTZConfiguration_REQ req;
	
	printf("soap_AddPTZConfiguration.\r\n");

	p_AddPTZConfiguration = xml_node_soap_get(p_body, "AddPTZConfiguration");
	assert(p_AddPTZConfiguration);	
	
	memset(&req, 0, sizeof(req));

	ret = parse_AddPTZConfiguration(p_AddPTZConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_AddPTZConfiguration(&req);
		if (ONVIF_OK == ret)
    	{
    		return soap_build_send_rly(p_user, rx_msg, build_AddPTZConfiguration_rly_xml, NULL);
    	}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_RemovePTZConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_RemovePTZConfiguration;
	XMLN * p_ProfileToken;
	
	printf("soap_RemovePTZConfiguration.\r\n");

	p_RemovePTZConfiguration = xml_node_soap_get(p_body, "RemovePTZConfiguration");
	assert(p_RemovePTZConfiguration);

	p_ProfileToken = xml_node_soap_get(p_RemovePTZConfiguration, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
    	ONVIF_RET ret = onvif_RemovePTZConfiguration(p_ProfileToken->data);
    	if (ONVIF_OK == ret)
    	{
    		return soap_build_send_rly(p_user, rx_msg, build_RemovePTZConfiguration_rly_xml, NULL);
    	}
    	else if (ONVIF_ERR_NO_PROFILE == ret)
    	{
    		return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoProfile", "Profile Not Exist");
    	}
    }
    else
    {
    	return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attribute");
    }

    return soap_err_def_rly(p_user, rx_msg);
}


int soap_GetConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetConfigurationOptions;
	XMLN * p_ConfigurationToken;
	
	printf("soap_GetConfigurationOptions.\r\n");

	p_GetConfigurationOptions = xml_node_soap_get(p_body, "GetConfigurationOptions");
	assert(p_GetConfigurationOptions);
	
	p_ConfigurationToken = xml_node_soap_get(p_GetConfigurationOptions, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		GetPTZCfgOptions_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(GetPTZCfgOptions_RES));
		ret = my_onvif_GetPTZConfigurationOptions(p_ConfigurationToken->data,&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetConfigurationOptions_rly_xml, (char *)&res); 
		}		 
	
		return soap_build_err_rly(p_user, rx_msg, ret);
	}
	
	return soap_err_def_rly(p_user, rx_msg);
}

int soap_GetStatus(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	char * post;
	XMLN * p_GetStatus;
	
	printf("soap_GetStatus.\r\n");

	p_GetStatus = xml_node_soap_get(p_body, "GetStatus");
	assert(p_GetStatus);

	post = rx_msg->first_line.value_string;
	if (strcasestr(post, "ptz"))
	{
		XMLN * p_ProfileToken = xml_node_soap_get(p_GetStatus, "ProfileToken");
		if (p_ProfileToken && p_ProfileToken->data)
		{
			onvif_PTZStatus res;
			ONVIF_RET ret;
			memset(&res, 0, sizeof(onvif_PTZStatus));
			ret = my_onvif_PTZGetStatus(p_ProfileToken->data,&res);
			if (ONVIF_OK == ret)
			{
				return soap_build_send_rly(p_user, rx_msg, build_PTZ_GetStatus_rly_xml, (char *)&res); 
			}		 
		
			return soap_build_err_rly(p_user, rx_msg, ret);
		}
	}
	else if (strcasestr(post, "imaging"))
	{
		XMLN * p_VideoSourceToken = xml_node_soap_get(p_GetStatus, "VideoSourceToken");
		if (p_VideoSourceToken && p_VideoSourceToken->data)
		{
			onvif_ImagingStatus res;
			ONVIF_RET ret;
			memset(&res, 0, sizeof(onvif_ImagingStatus));
			ret = my_onvif_ImagingGetStatus(p_VideoSourceToken->data,&res);
			if (ONVIF_OK == ret)
			{
				return soap_build_send_rly(p_user, rx_msg, build_IMG_GetStatus_rly_xml, (char *)&res); 
			}		 
		
			return soap_build_err_rly(p_user, rx_msg, ret);
		}
	}	
	
	return soap_err_def_rly(p_user, rx_msg);
}

int soap_ContinuousMove(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_ContinuousMove;
	ContinuousMove_REQ req;
	
	printf("soap_ContinuousMove.\r\n");
	
	p_ContinuousMove = xml_node_soap_get(p_body, "ContinuousMove");
	assert(p_ContinuousMove);
	
	memset(&req, 0, sizeof(ContinuousMove_REQ));

	ret = parse_ContinuousMove(p_ContinuousMove, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_ContinuousMove(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_ContinuousMove_rly_xml, NULL);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);		
}

int soap_Stop(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	char * post;
	XMLN * p_Stop;
	
	printf("soap_Stop\r\n");

	p_Stop = xml_node_soap_get(p_body, "Stop");
	assert(p_Stop);

	post = rx_msg->first_line.value_string;
	if (strcasestr(post, "ptz"))
	{
		ONVIF_RET ret;
		PTZ_Stop_REQ req;
		
		memset(&req, 0, sizeof(req));		

		ret = parse_PTZ_Stop(p_Stop, &req);
		if (ONVIF_OK == ret)
		{
			ret = onvif_PTZ_Stop(&req);
			if (ONVIF_OK == ret)
			{
				return soap_build_send_rly(p_user, rx_msg, build_PTZ_Stop_rly_xml, NULL);
			}
		}

		return soap_build_err_rly(p_user, rx_msg, ret);
	}
	else if (strcasestr(post, "imaging"))
	{
		XMLN * p_VideoSourceToken = xml_node_soap_get(p_Stop, "VideoSourceToken");
		if (p_VideoSourceToken && p_VideoSourceToken->data)
		{
			ONVIF_RET ret;
			ret = my_onvif_Imaging_Stop((char *)p_VideoSourceToken->data);
			if (ONVIF_OK == ret)
			{
				return soap_build_send_rly(p_user, rx_msg, build_IMG_Stop_rly_xml, NULL); 
			}		 
		
			return soap_build_err_rly(p_user, rx_msg, ret);
		}
	}
	
	return soap_err_def_rly(p_user, rx_msg);
}

int soap_AbsoluteMove(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_AbsoluteMove;
	AbsoluteMove_REQ req;
	
    printf("soap_AbsoluteMove\r\n");

    p_AbsoluteMove = xml_node_soap_get(p_body, "AbsoluteMove");
	assert(p_AbsoluteMove);
	
	memset(&req, 0, sizeof(AbsoluteMove_REQ));
	
    ret = parse_AbsoluteMove(p_AbsoluteMove, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_AbsoluteMove(&req);
	    if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_AbsoluteMove_rly_xml, NULL);
		}
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_RelativeMove(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_RelativeMove;
	RelativeMove_REQ req;
	
    printf("soap_RelativeMove\r\n");

    p_RelativeMove = xml_node_soap_get(p_body, "RelativeMove");
	assert(p_RelativeMove);
    
	memset(&req, 0, sizeof(req));

	ret = parse_RelativeMove(p_RelativeMove, &req);    
	if (ONVIF_OK == ret)
	{
		ret = onvif_RelativeMove(&req);		
	    if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_RelativeMove_rly_xml, NULL);
		}
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_SetPreset(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetPreset;
	SetPreset_REQ req;
	
    printf("soap_SetPreset\r\n");

    p_SetPreset = xml_node_soap_get(p_body, "SetPreset");
	assert(p_SetPreset);
	
	memset(&req, 0, sizeof(SetPreset_REQ));

	ret = parse_SetPreset(p_SetPreset, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_SetPreset(&req);
	    if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_SetPreset_rly_xml, req.PresetToken);
		}
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetPresets(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetPresets;
	XMLN * p_ProfileToken;
	
    printf("soap_GetPresets\r\n");

    p_GetPresets = xml_node_soap_get(p_body, "GetPresets");
	assert(p_GetPresets);

	p_ProfileToken = xml_node_soap_get(p_GetPresets, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		GetPresets_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(GetPresets_RES));
		ret = my_onvif_GetPresets(p_ProfileToken->data,&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetPresets_rly_xml, (char *)&res); 
		}		 
	
		return soap_build_err_rly(p_user, rx_msg, ret);
	}
	else 
	{
		return soap_err_def2_rly(p_user, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attribute");
	}
}

int soap_RemovePreset(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_RemovePreset;
	RemovePreset_REQ req;
	
    printf("soap_RemovePreset\r\n");

    p_RemovePreset = xml_node_soap_get(p_body, "RemovePreset");
	assert(p_RemovePreset);
	
	memset(&req, 0, sizeof(req));
	
	ret = parse_RemovePreset(p_RemovePreset, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_RemovePreset(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_RemovePreset_rly_xml, NULL);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GotoPreset(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_GotoPreset;
	GotoPreset_REQ req;
	
    printf("soap_GotoPreset\r\n");

    p_GotoPreset = xml_node_soap_get(p_body, "GotoPreset");
	assert(p_GotoPreset);
	
	memset(&req, 0, sizeof(req));

	ret = parse_GotoPreset(p_GotoPreset, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_GotoPreset(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GotoPreset_rly_xml, NULL);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GotoHomePosition(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_GotoHomePosition;
	GotoHomePosition_REQ req;
	
    printf("soap_GotoHomePosition\r\n");

    p_GotoHomePosition = xml_node_soap_get(p_body, "GotoHomePosition");
	assert(p_GotoHomePosition);
	
	memset(&req, 0, sizeof(req));

	ret = parse_GotoHomePosition(p_GotoHomePosition, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_GotoHomePosition(&req);
	    if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GotoHomePosition_rly_xml, NULL);
		}
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_SetHomePosition(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_SetHomePosition;
	XMLN * p_ProfileToken;
	ONVIF_RET ret = ONVIF_ERR_MISSINGATTR;
	
	printf("soap_SetHomePosition\r\n");

    p_SetHomePosition = xml_node_soap_get(p_body, "SetHomePosition");
	assert(p_SetHomePosition);	
	
	p_ProfileToken = xml_node_soap_get(p_SetHomePosition, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		ret = onvif_SetHomePosition(p_ProfileToken->data);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_SetHomePosition_rly_xml, NULL);
		}
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetGuaranteedNumberOfVideoEncoderInstances(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetGuaranteedNumberOfVideoEncoderInstances;
	XMLN * p_ConfigurationToken;
	
	printf("soap_GetGuaranteedNumberOfVideoEncoderInstances\r\n");

    p_GetGuaranteedNumberOfVideoEncoderInstances = xml_node_soap_get(p_body, "GetGuaranteedNumberOfVideoEncoderInstances");
	assert(p_GetGuaranteedNumberOfVideoEncoderInstances);

	p_ConfigurationToken = xml_node_soap_get(p_GetGuaranteedNumberOfVideoEncoderInstances, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		return soap_build_send_rly(p_user, rx_msg, build_GetGuaranteedNumberOfVideoEncoderInstances_rly_xml, p_ConfigurationToken->data);
	}

	return soap_build_err_rly(p_user, rx_msg, ONVIF_ERR_MISSINGATTR);
}

int soap_GetImagingSettings(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetImagingSettings;
	XMLN * p_VideoSourceToken;
	
	printf("soap_GetImagingSettings\r\n");

    p_GetImagingSettings = xml_node_soap_get(p_body, "GetImagingSettings");
	assert(p_GetImagingSettings);

	p_VideoSourceToken = xml_node_soap_get(p_GetImagingSettings, "VideoSourceToken");
	if (p_VideoSourceToken && p_VideoSourceToken->data)
	{
		GetImagSettings_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(GetImagSettings_RES));
		res.VideoSourceToken = (char *)p_VideoSourceToken->data;
		ret = my_onvif_GetImagingSettings(&res);
	    if (ONVIF_OK == ret)
	    {
	    	return soap_build_send_rly(p_user, rx_msg, build_GetImagingSettings_rly_xml, (char *)&res); 
	    }        

	    return soap_build_err_rly(p_user, rx_msg, ret);
	}

	return soap_build_err_rly(p_user, rx_msg, ONVIF_ERR_MISSINGATTR);
}

int soap_SetImagingSettings(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetImagingSettings;
	SetImagingSettings_REQ req;
	
	printf("soap_SetImagingSettings\r\n");

    p_SetImagingSettings = xml_node_soap_get(p_body, "SetImagingSettings");
	assert(p_SetImagingSettings);
	
	memset(&req, 0, sizeof(SetImagingSettings_REQ));

	ret = parse_SetImagingSettings(p_SetImagingSettings, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_SetImagingSettings(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_SetImagingSettings_rly_xml, NULL);
		}
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetOptions;
	XMLN * p_VideoSourceToken;
	
	printf("soap_GetOptions\r\n");

    p_GetOptions = xml_node_soap_get(p_body, "GetOptions");
	assert(p_GetOptions);

	p_VideoSourceToken = xml_node_soap_get(p_GetOptions, "VideoSourceToken");
	if (p_VideoSourceToken && p_VideoSourceToken->data)
	{
		GetImagingOptions_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(GetImagingOptions_RES));
		res.VideoSourceToken = (char *)p_VideoSourceToken->data;
		ret = my_onvif_GetImagingOptions(&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetOptions_rly_xml, (char *)&res); 
		}		 
	
		return soap_build_err_rly(p_user, rx_msg, ret);
	}

	return soap_build_err_rly(p_user, rx_msg, ONVIF_ERR_MISSINGATTR);
}

int soap_GetMoveOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetMoveOptions;
	XMLN * p_VideoSourceToken;
	
	printf("soap_GetMoveOptions\r\n");

    p_GetMoveOptions = xml_node_soap_get(p_body, "GetMoveOptions");
	assert(p_GetMoveOptions);

	p_VideoSourceToken = xml_node_soap_get(p_GetMoveOptions, "VideoSourceToken");
	if (p_VideoSourceToken && p_VideoSourceToken->data)
	{
		ONVIF_RET ret;
		ret = my_onvif_GetMoveOptions(p_VideoSourceToken->data);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetMoveOptions_rly_xml,NULL); 
		}		 
	
		return soap_build_err_rly(p_user, rx_msg, ret);
	}

	return soap_build_err_rly(p_user, rx_msg, ONVIF_ERR_MISSINGATTR);
}

int soap_Move(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_Move;
	Move_REQ req;
	
	printf("soap_Move\r\n");

	p_Move = xml_node_soap_get(p_body, "Move");
	assert(p_Move);
	
	memset(&req, 0, sizeof(Move_REQ));

	ret = parse_Move(p_Move, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_Move(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_Move_rly_xml, NULL);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetUsers(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	printf("soap_GetUsers\r\n");
	GetUsers_RES res;
	ONVIF_RET ret;
	memset(&res, 0, sizeof(GetUsers_RES));
	ret = my_onvif_GetUsers(&res);
	if (ONVIF_OK == ret)
	{
		return soap_build_send_rly(p_user, rx_msg, build_GetUsers_rly_xml, (char *)&res); 
	}		 

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_CreateUsers(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_CreateUsers;
	ONVIF_USER users[MAX_USERS];
	
	printf("soap_CreateUsers\r\n");

	p_CreateUsers = xml_node_soap_get(p_body, "CreateUsers");
	assert(p_CreateUsers);
	
    memset(users, 0, sizeof(ONVIF_USER) * MAX_USERS);
    
    ret = parse_CreateUsers(p_CreateUsers, users, MAX_USERS);
    if (ONVIF_OK == ret)
    {
        ret = onvif_add_users(users, MAX_USERS);
        if (ONVIF_OK == ret)
        {
    		return soap_build_send_rly(p_user, rx_msg, build_CreateUsers_rly_xml, NULL);
    	}
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}
	
int soap_DeleteUsers(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_DeleteUsers;
	ONVIF_USER users[MAX_USERS];
	
	printf("soap_DeleteUsers\r\n");

	p_DeleteUsers = xml_node_soap_get(p_body, "DeleteUsers");
	assert(p_DeleteUsers);
	
    memset(users, 0, sizeof(ONVIF_USER) * MAX_USERS);
    
    ret = parse_DeleteUsers(p_DeleteUsers, users, MAX_USERS);
    if (ONVIF_OK == ret)
    {
        ret = onvif_delete_users(users, MAX_USERS);
        if (ONVIF_OK == ret)
        {
    		return soap_build_send_rly(p_user, rx_msg, build_DeleteUsers_rly_xml, NULL);
    	}
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_SetUser(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetUser;
	ONVIF_USER users[MAX_USERS];
	
	printf("soap_SetUser\r\n");

	p_SetUser = xml_node_soap_get(p_body, "SetUser");
	assert(p_SetUser);
	
    memset(users, 0, sizeof(ONVIF_USER) * MAX_USERS);
    
    ret = parse_CreateUsers(p_SetUser, users, MAX_USERS);
    if (ONVIF_OK == ret)
    {
        ret = onvif_set_users(users, MAX_USERS);
        if (ONVIF_OK == ret)
        {
    		return soap_build_send_rly(p_user, rx_msg, build_SetUser_rly_xml, NULL);
    	}
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_UpgradeSystemFirmware(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	printf("soap_UpgradeSystemFirmware\r\n");

	return 0;	
}

int soap_StartFirmwareUpgrade(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	StartFirmwareUpgrade_RES res;
	
	printf("soap_StartFirmwareUpgrade\r\n");

	memset(&res, 0, sizeof(res));
	
	if (onvif_StartFirmwareUpgrade(&res))
	{	
		return soap_build_send_rly(p_user, rx_msg, build_StartFirmwareUpgrade_rly_xml, (char *)&res);
	}
	
	return soap_err_def_rly(p_user, rx_msg);
}

void soap_FirmwareUpgrade(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
	char * p_buff = get_http_ctt(rx_msg);
	
	if (onvif_FirmwareUpgradeCheck(p_buff, rx_msg->ctt_len))
	{
		if (onvif_FirmwareUpgrade(p_buff, rx_msg->ctt_len))
		{
			soap_http_rly(p_user, rx_msg, NULL, 0);

			onvif_FirmwareUpgradePost();
		}
		else
		{
			soap_http_err_rly(p_user, rx_msg, 500, "Internal Server Error", NULL, 0);
		}
	}
	else
	{
		soap_http_err_rly(p_user, rx_msg, 415, "Unsupported Media Type", NULL, 0);
	}	
}

void soap_GetSnapshot(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
    char *buff = g_reply_buf;
    int  rlen = 2047*1024;
    //char buff[200*1024];    // JPEG image buff, max 200K
    //int  rlen = sizeof(buff);
    char profile_token[ONVIF_TOKEN_LEN] = {'\0'};

    // get profile token
    char * post = rx_msg->first_line.value_string;
    char * p1 = strstr(post, "snapshot");
    if (p1)
    {
        char * p2 = strchr(p1+1, '/');
        if (p2)
        {   
            int i = 0;
            
            p2++;
            while (p2 && *p2 != '\0')
            {
                if (*p2 == ' ')
                {
                    break;
                }

                if (i < ONVIF_TOKEN_LEN-1)
                {
                    profile_token[i++] = *p2;  
                } 

                p2++;
            }

            profile_token[i] = '\0';
        }
    }

    if (profile_token[0] == '\0')
    {
        soap_http_err_rly(p_user, rx_msg, 500, "Internal Server Error", NULL, 0);
        return;
    }
    
    if (ONVIF_OK == onvif_GetSnapshot(buff, &rlen, profile_token))
    {
    	int tlen;
        char * p_bufs = g_send_buf;//(char *)malloc(rlen + 1024);
    	/*if (NULL == p_bufs)
    	{
    	    soap_http_err_rly(p_user, rx_msg, 500, "Internal Server Error", NULL, 0);
    		return;
    	}*/
    	
    	tlen = sprintf(p_bufs,	"HTTP/1.1 200 OK\r\n"
								"Server: hsoap/2.8\r\n"
								"Content-Type: image/jpeg\r\n"
								"Content-Length: %d\r\n"
								"Connection: close\r\n\r\n",
								rlen);

    	memmove(p_bufs+tlen, buff, rlen);
    	tlen += rlen;
    	
    	send(p_user->cfd, p_bufs, tlen, 0);
    	//free(p_bufs);
    }
    else
    {
        soap_http_err_rly(p_user, rx_msg, 500, "Internal Server Error", NULL, 0);
    }
}

int soap_GetOSDs(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_GetOSDs;
	GetOSDs_REQ req;
	
	printf("soap_GetOSDs\r\n");

	p_GetOSDs = xml_node_soap_get(p_body, "GetOSDs");
	assert(p_GetOSDs);
	
	memset(&req, 0, sizeof(req));
    
    ret = parse_GetOSDs(p_GetOSDs, &req);
    if (ONVIF_OK == ret)
    {
 		GetOSDs_RES res;
		ONVIF_RET ret;
		int result = 0;
		memset(&res, 0, sizeof(GetOSDs_RES));
		ret = my_onvif_GetOSDs(&req,&res);
		if (ONVIF_OK == ret)
		{
			result = soap_build_send_rly(p_user, rx_msg, build_GetOSDs_rly_xml, (char *)&res); 
		}		 
		else
			result = soap_build_err_rly(p_user, rx_msg, ret);
		
		ONVIF_OSD *p_free = res.OSDs;
		ONVIF_OSD *p_next = NULL;
		while(p_free)
		{
			p_next = NULL;
			if(p_free->next)
			{
				p_next = p_free->next;
			}
      free_one_item(p_free,ONVIF_TYPE_OSD);
			//free(p_free);
			p_free = p_next;
		}
		
		return result;
    }

    return soap_build_err_rly(p_user, rx_msg, ret);	
}

int soap_GetOSD(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_GetOSD;
	GetOSD_REQ req;
	
	printf("soap_GetOSD\r\n");

	p_GetOSD = xml_node_soap_get(p_body, "GetOSD");
	assert(p_GetOSD);
	
	memset(&req, 0, sizeof(req));
    
    ret = parse_GetOSD(p_GetOSD, &req);
    if (ONVIF_OK == ret)
    {
		GetOSD_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(GetOSD_RES));
		ret = my_onvif_GetOSD(&req,&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetOSD_rly_xml, (char *)&res); 
		}		 
	
		return soap_build_err_rly(p_user, rx_msg, ret);
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
} 

int soap_SetOSD(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetOSD;
	SetOSD_REQ req;
		
	printf("soap_SetOSD\r\n");

	p_SetOSD = xml_node_soap_get(p_body, "SetOSD");
	assert(p_SetOSD);
	
	memset(&req, 0, sizeof(req));
    
    ret = parse_SetOSD(p_SetOSD, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_SetOSD(&req);
    	if (ONVIF_OK == ret)
    	{
			return soap_build_send_rly(p_user, rx_msg, build_SetOSD_rly_xml, NULL);
		}
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetOSDOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	printf("soap_GetOSDOptions\r\n");
	GetOSDOptions_RES res;
	ONVIF_RET ret;
	memset(&res, 0, sizeof(GetOSDOptions_RES));
	ret = my_onvif_GetOSDOptions(&res);
	if (ONVIF_OK == ret)
	{
		return soap_build_send_rly(p_user, rx_msg, build_GetOSDOptions_rly_xml, (char *)&res); 
	}		 

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_CreateOSD(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_CreateOSD;
	CreateOSD_REQ req;
	
	printf("soap_CreateOSD\r\n");

	p_CreateOSD = xml_node_soap_get(p_body, "CreateOSD");
	assert(p_CreateOSD);
	
	memset(&req, 0, sizeof(req));
    
    ret = parse_CreateOSD(p_CreateOSD, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_CreateOSD(&req);
    	if (ONVIF_OK == ret)
    	{
			return soap_build_send_rly(p_user, rx_msg, build_CreateOSD_rly_xml, req.OSD.token);
		}
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_DeleteOSD(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_DeleteOSD;
	DeleteOSD_REQ req;
		
	printf("soap_DeleteOSD\r\n");

	p_DeleteOSD = xml_node_soap_get(p_body, "DeleteOSD");
	assert(p_DeleteOSD);
	
	memset(&req, 0, sizeof(req));
    
    ret = parse_DeleteOSD(p_DeleteOSD, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_DeleteOSD(&req);
    	if (ONVIF_OK == ret)
    	{
			return soap_build_send_rly(p_user, rx_msg, build_DeleteOSD_rly_xml, NULL);
		}
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_SetConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetConfiguration;
	SetConfiguration_REQ req;
		
	printf("soap_SetConfiguration\r\n");

	p_SetConfiguration = xml_node_soap_get(p_body, "SetConfiguration");
	assert(p_SetConfiguration);
	
	memset(&req, 0, sizeof(req));
    
    ret = parse_SetConfiguration(p_SetConfiguration, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_SetConfiguration(&req);
    	if (ONVIF_OK == ret)
    	{
			return soap_build_send_rly(p_user, rx_msg, build_SetConfiguration_rly_xml, NULL);
		}
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_StartMulticastStreaming(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_StartMulticastStreaming;
	XMLN * p_ProfileToken;
	ONVIF_RET ret = ONVIF_ERR_MISSINGATTR;
	
	printf("soap_StartMulticastStreaming\r\n");

    p_StartMulticastStreaming = xml_node_soap_get(p_body, "StartMulticastStreaming");
	assert(p_StartMulticastStreaming);	
	
	p_ProfileToken = xml_node_soap_get(p_StartMulticastStreaming, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		ret = onvif_StartMulticastStreaming(p_ProfileToken->data);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_StartMulticastStreaming_rly_xml, NULL);
		}
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_StopMulticastStreaming(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_StopMulticastStreaming;
	XMLN * p_ProfileToken;
	ONVIF_RET ret = ONVIF_ERR_MISSINGATTR;
	
	printf("soap_StopMulticastStreaming\r\n");

    p_StopMulticastStreaming = xml_node_soap_get(p_body, "StopMulticastStreaming");
	assert(p_StopMulticastStreaming);	
	
	p_ProfileToken = xml_node_soap_get(p_StopMulticastStreaming, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		ret = onvif_StopMulticastStreaming(p_ProfileToken->data);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_StopMulticastStreaming_rly_xml, NULL);
		}
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetMetadataConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	printf("soap_GetMetadataConfigurations\r\n");
	GetMetadataConfigurations_RES res;
	ONVIF_RET ret;
	int result = 0;
	memset(&res, 0, sizeof(GetMetadataConfigurations_RES));
	ret = my_onvif_GetMetadataConfigurations(&res);
	if (ONVIF_OK == ret)
	{
		result = soap_build_send_rly(p_user, rx_msg, build_GetMetadataConfigurations_rly_xml, (char *)&res); 
	}
	else 
		result = soap_build_err_rly(p_user, rx_msg, ret);
	ONVIF_METADATA_CFG *p_free = res.metadata_cfg;
	ONVIF_METADATA_CFG *p_next = NULL;
	while(p_free)
	{
		p_next = NULL;
		if(p_free->next)
		{
			p_next = p_free->next;
		}
    free_one_item(p_free,ONVIF_TYPE_METADATA);
		//free(p_free);
		p_free = p_next;
	}

	return result;
}

int soap_GetMetadataConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{	
	XMLN * p_GetMetadataConfiguration;
	XMLN * p_ConfigurationToken;
	ONVIF_RET ret = ONVIF_ERR_MISSINGATTR;
	
	printf("soap_GetMetadataConfiguration\r\n");

    p_GetMetadataConfiguration = xml_node_soap_get(p_body, "GetMetadataConfiguration");
	assert(p_GetMetadataConfiguration);	
	
	p_ConfigurationToken = xml_node_soap_get(p_GetMetadataConfiguration, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		GetMetadataConfiguration_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(GetMetadataConfiguration_RES));
		res.ConfigurationToken =(char *)p_ConfigurationToken->data;
		ret = my_onvif_GetMetadataConfiguration(&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetMetadataConfiguration_rly_xml, (char *)&res); 
		}		 
	
		return soap_build_err_rly(p_user, rx_msg, ret);
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetCompatibleMetadataConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetCompatibleMetadataConfigurations;
	XMLN * p_ProfileToken;
	ONVIF_RET ret = ONVIF_ERR_MISSINGATTR;
	
	printf("soap_GetCompatibleMetadataConfigurations\r\n");

    p_GetCompatibleMetadataConfigurations = xml_node_soap_get(p_body, "GetCompatibleMetadataConfigurations");
	assert(p_GetCompatibleMetadataConfigurations);	
	
	p_ProfileToken = xml_node_soap_get(p_GetCompatibleMetadataConfigurations, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		GetCompatibleM_Cfgs_RES res;
		ONVIF_RET ret;
		int result = 0;
		memset(&res, 0, sizeof(GetCompatibleM_Cfgs_RES));
		res.ProfileToken = (char *)p_ProfileToken->data;
		ret = my_onvif_GetCompatibleMetadataConfigurations(&res);
		if (ONVIF_OK == ret)
		{
			result =soap_build_send_rly(p_user, rx_msg, build_GetCompatibleMetadataConfigurations_rly_xml, (char *)&res); 
		}	
		else
			result =soap_build_err_rly(p_user, rx_msg, ret);

		ONVIF_METADATA_CFG *p_free = res.metadata_cfg;
		ONVIF_METADATA_CFG *p_next = NULL;
		while(p_free)
		{
			p_next = NULL;
			if(p_free->next)
			{
				p_next = p_free->next;
			}
      free_one_item(p_free,ONVIF_TYPE_METADATA);
			//free(p_free);
			p_free = p_next;
		}
	
		return result;
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetMetadataConfigurationOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_GetMetadataConfigurationOptions;
	GetMetadataConfigurationOptions_REQ req;
		
	printf("soap_GetMetadataConfigurationOptions\r\n");

	p_GetMetadataConfigurationOptions = xml_node_soap_get(p_body, "GetMetadataConfigurationOptions");
	assert(p_GetMetadataConfigurationOptions);
	
	memset(&req, 0, sizeof(req));
    
    ret = parse_GetMetadataConfigurationOptions(p_GetMetadataConfigurationOptions, &req);
    if (ONVIF_OK == ret)
    {
		GetMetadataCfgOptions_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(GetMetadataCfgOptions_RES));
		ret = my_onvif_GetMetadataConfigurationOptions(&req,&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetMetadataConfigurationOptions_rly_xml, (char *)&res); 
		}		 
	
		return soap_build_err_rly(p_user, rx_msg, ret);
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_SetMetadataConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetMetadataConfiguration;
	SetMetadataConfiguration_REQ req;
		
	printf("soap_SetMetadataConfiguration\r\n");

	p_SetMetadataConfiguration = xml_node_soap_get(p_body, "SetMetadataConfiguration");
	assert(p_SetMetadataConfiguration);
	
	memset(&req, 0, sizeof(req));
    
    ret = parse_SetMetadataConfiguration(p_SetMetadataConfiguration, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_SetMetadataConfiguration(&req);
    	if (ONVIF_OK == ret)
    	{
			return soap_build_send_rly(p_user, rx_msg, build_SetMetadataConfiguration_rly_xml, NULL);
		}
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_AddMetadataConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_AddMetadataConfiguration;
	AddMetadataConfiguration_REQ req;
		
	printf("soap_AddMetadataConfiguration\r\n");

	p_AddMetadataConfiguration = xml_node_soap_get(p_body, "AddMetadataConfiguration");
	assert(p_AddMetadataConfiguration);
	
	memset(&req, 0, sizeof(req));
    
    ret = parse_AddMetadataConfiguration(p_AddMetadataConfiguration, &req);
    if (ONVIF_OK == ret)
    {
    	ret = onvif_AddMetadataConfiguration(&req);
    	if (ONVIF_OK == ret)
    	{
			return soap_build_send_rly(p_user, rx_msg, build_AddMetadataConfiguration_rly_xml, NULL);
		}
    }

    return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_RemoveMetadataConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_RemoveMetadataConfiguration;
	XMLN * p_ProfileToken;
	ONVIF_RET ret = ONVIF_ERR_MISSINGATTR;
	
	printf("soap_RemoveMetadataConfiguration\r\n");

    p_RemoveMetadataConfiguration = xml_node_soap_get(p_body, "RemoveMetadataConfiguration");
	assert(p_RemoveMetadataConfiguration);	
	
	p_ProfileToken = xml_node_soap_get(p_RemoveMetadataConfiguration, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		ret = onvif_RemoveMetadataConfiguration(p_ProfileToken->data);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_RemoveMetadataConfiguration_rly_xml, NULL);
		}	
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

#ifdef VIDEO_ANALYTICS

int soap_GetVideoAnalyticsConfigurations(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	printf("soap_GetVideoAnalyticsConfigurations\r\n");
	Get_VA_Configurations_RES res;
	ONVIF_RET ret;
	int result = 0;
	memset(&res, 0, sizeof(Get_VA_Configurations_RES));
	ret = my_onvif_GetVideoAnalyticsConfigurations(&res);
	if (ONVIF_OK == ret)
	{
		result = soap_build_send_rly(p_user, rx_msg, build_GetVideoAnalyticsConfigurations_rly_xml, (char *)&res); 
	}
	else 
		result = soap_build_err_rly(p_user, rx_msg, ret);
	ONVIF_VACFG *p_free = res.p_va_cfg;
	ONVIF_VACFG *p_next = NULL;
	while(p_free)
	{
		p_next = NULL;
		if(p_free->next)
		{
			p_next = p_free->next;
		}
		onvif_free_configs(&p_free->rules);
		onvif_free_configs(&p_free->modules);
		onvif_free_configs(&p_free->VideoAnalyticsConfiguration.AnalyticsEngineConfiguration.AnalyticsModule);
		onvif_free_configs(&p_free->VideoAnalyticsConfiguration.RuleEngineConfiguration.Rule);
		onvif_free_cfg_descs(&p_free->SupportedRules.RuleDescription);
    free_one_item(p_free,ONVIF_TYPE_VACFG);
		//free(p_free);
		p_free = p_next;
	}

	return result;
}

int soap_AddVideoAnalyticsConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_AddVideoAnalyticsConfiguration;
	AddVideoAnalyticsConfiguration_REQ req;
	ONVIF_RET ret;
	
	printf("soap_AddVideoAnalyticsConfiguration\r\n");

    p_AddVideoAnalyticsConfiguration = xml_node_soap_get(p_body, "AddVideoAnalyticsConfiguration");
	assert(p_AddVideoAnalyticsConfiguration);	

	memset(&req, 0, sizeof(req));
	
	ret = parse_AddVideoAnalyticsConfiguration(p_AddVideoAnalyticsConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_AddVideoAnalyticsConfiguration(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_AddVideoAnalyticsConfiguration_rly_xml, NULL);
		}	
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetVideoAnalyticsConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetVideoAnalyticsConfiguration;
	GetVideoAnalyticsConfiguration_REQ req;
	ONVIF_RET ret;
	
	printf("soap_GetVideoAnalyticsConfiguration\r\n");

    p_GetVideoAnalyticsConfiguration = xml_node_soap_get(p_body, "GetVideoAnalyticsConfiguration");
	assert(p_GetVideoAnalyticsConfiguration);	

	memset(&req, 0, sizeof(req));
	
	ret = parse_GetVideoAnalyticsConfiguration(p_GetVideoAnalyticsConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		int result = 0;
		Get_VA_Configuration_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(Get_VA_Configuration_RES));
		ret = my_onvif_GetVideoAnalyticsConfiguration(&req,&res);
		if (ONVIF_OK == ret)
		{
			result = soap_build_send_rly(p_user, rx_msg, build_GetVideoAnalyticsConfiguration_rly_xml, (char *)&res); 
		}
		else
			result = soap_build_err_rly(p_user, rx_msg, ret);
		onvif_free_configs(&res.struva_cfg.rules);
		onvif_free_configs(&res.struva_cfg.modules);
		onvif_free_configs(&res.struva_cfg.VideoAnalyticsConfiguration.AnalyticsEngineConfiguration.AnalyticsModule);
		onvif_free_configs(&res.struva_cfg.VideoAnalyticsConfiguration.RuleEngineConfiguration.Rule);
		onvif_free_cfg_descs(&res.struva_cfg.SupportedRules.RuleDescription);
		return result;
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_RemoveVideoAnalyticsConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_RemoveVideoAnalyticsConfiguration;
	RemoveVideoAnalyticsConfiguration_REQ req;
	ONVIF_RET ret;
	
	printf("soap_RemoveVideoAnalyticsConfiguration\r\n");

    p_RemoveVideoAnalyticsConfiguration = xml_node_soap_get(p_body, "RemoveVideoAnalyticsConfiguration");
	assert(p_RemoveVideoAnalyticsConfiguration);	

	memset(&req, 0, sizeof(req));
	
	ret = parse_RemoveVideoAnalyticsConfiguration(p_RemoveVideoAnalyticsConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_RemoveVideoAnalyticsConfiguration(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_RemoveVideoAnalyticsConfiguration_rly_xml, NULL);
		}	
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_SetVideoAnalyticsConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_SetVideoAnalyticsConfiguration;
	SetVideoAnalyticsConfiguration_REQ req;
	ONVIF_RET ret;
	
	printf("soap_SetVideoAnalyticsConfiguration\r\n");

    p_SetVideoAnalyticsConfiguration = xml_node_soap_get(p_body, "SetVideoAnalyticsConfiguration");
	assert(p_SetVideoAnalyticsConfiguration);	

	memset(&req, 0, sizeof(req));
	
	ret = parse_SetVideoAnalyticsConfiguration(p_SetVideoAnalyticsConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_SetVideoAnalyticsConfiguration(&req);
		onvif_free_configs(&req.Configuration.RuleEngineConfiguration.Rule);
		onvif_free_configs(&req.Configuration.AnalyticsEngineConfiguration.AnalyticsModule);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_SetVideoAnalyticsConfiguration_rly_xml, NULL);
		}	
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetSupportedRules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetSupportedRules;
	GetSupportedRules_REQ req;
	ONVIF_RET ret;
	
	printf("soap_GetSupportedRules\r\n");

    p_GetSupportedRules = xml_node_soap_get(p_body, "GetSupportedRules");
	assert(p_GetSupportedRules);	

	memset(&req, 0, sizeof(req));
	
	ret = parse_GetSupportedRules(p_GetSupportedRules, &req);
	if (ONVIF_OK == ret)
	{
		int result =0;
		GetSupportedRules_RES res;
		memset(&res, 0, sizeof(res));
		
		ret = onvif_GetSupportedRules(&req, &res);
		if (ONVIF_OK == ret)
		{
			result =soap_build_send_rly(p_user, rx_msg, build_GetSupportedRules_rly_xml, (char *)&res);
		}
		else
			result =soap_build_err_rly(p_user, rx_msg, ret);

		onvif_free_cfg_descs(&res.SupportedRules.RuleDescription);

		return result;
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_CreateRules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_CreateRules;
	CreateRules_REQ req;
	ONVIF_RET ret;
	
	printf("soap_CreateRules\r\n");

    p_CreateRules = xml_node_soap_get(p_body, "CreateRules");
	assert(p_CreateRules);	

	memset(&req, 0, sizeof(req));
	
	ret = parse_CreateRules(p_CreateRules, &req);
	if (ONVIF_OK == ret)
	{		
		ret = onvif_CreateRules(&req);
		onvif_free_configs(&req.Rule);
		if (ONVIF_OK == ret)
		{			
			return soap_build_send_rly(p_user, rx_msg, build_CreateRules_rly_xml, NULL);
		}	
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_DeleteRules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_DeleteRules;
	DeleteRules_REQ req;
	ONVIF_RET ret;
	
	printf("soap_DeleteRules\r\n");

    p_DeleteRules = xml_node_soap_get(p_body, "DeleteRules");
	assert(p_DeleteRules);	

	memset(&req, 0, sizeof(req));
	
	ret = parse_DeleteRules(p_DeleteRules, &req);
	if (ONVIF_OK == ret)
	{		
		ret = onvif_DeleteRules(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_DeleteRules_rly_xml, NULL);
		}	
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetRules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetRules;
	GetRules_REQ req;
	ONVIF_RET ret;
	
	printf("soap_GetRules\r\n");

    p_GetRules = xml_node_soap_get(p_body, "GetRules");
	assert(p_GetRules);	

	memset(&req, 0, sizeof(req));
	
	ret = parse_GetRules(p_GetRules, &req);
	if (ONVIF_OK == ret)
	{
		int result = 0;
		GetRules_RES res;
		memset(&res, 0, sizeof(res));
		
		ret = onvif_GetRules(&req, &res);
		if (ONVIF_OK == ret)
		{
			result = soap_build_send_rly(p_user, rx_msg, build_GetRules_rly_xml, (char *)&res);
		}
		else
			result = soap_build_err_rly(p_user, rx_msg, ret);
		
		onvif_free_configs(&res.Rule);
		return result;
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_ModifyRules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_ModifyRules;
	ModifyRules_REQ req;
	ONVIF_RET ret;
	
	printf("soap_ModifyRules\r\n");

    p_ModifyRules = xml_node_soap_get(p_body, "ModifyRules");
	assert(p_ModifyRules);	

	memset(&req, 0, sizeof(req));
	
	ret = parse_ModifyRules(p_ModifyRules, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_ModifyRules(&req);
		onvif_free_configs(&req.Rule);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_ModifyRules_rly_xml, NULL);
		}	
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_CreateAnalyticsModules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_CreateAnalyticsModules;
	CreateAnalyticsModules_REQ req;
	ONVIF_RET ret;
	
	printf("soap_CreateAnalyticsModules\r\n");

    p_CreateAnalyticsModules = xml_node_soap_get(p_body, "CreateAnalyticsModules");
	assert(p_CreateAnalyticsModules);	

	memset(&req, 0, sizeof(req));
	
	ret = parse_CreateAnalyticsModules(p_CreateAnalyticsModules, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_CreateAnalyticsModules(&req);
		onvif_free_configs(&req.AnalyticsModule);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_CreateAnalyticsModules_rly_xml, NULL);
		}	
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_DeleteAnalyticsModules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_DeleteAnalyticsModules;
	DeleteAnalyticsModules_REQ req;
	ONVIF_RET ret;
	
	printf("soap_DeleteAnalyticsModules\r\n");

    p_DeleteAnalyticsModules = xml_node_soap_get(p_body, "DeleteAnalyticsModules");
	assert(p_DeleteAnalyticsModules);	

	memset(&req, 0, sizeof(req));
	
	ret = parse_DeleteAnalyticsModules(p_DeleteAnalyticsModules, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_DeleteAnalyticsModules(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_DeleteAnalyticsModules_rly_xml, NULL);
		}	
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetAnalyticsModules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetAnalyticsModules;
	GetAnalyticsModules_REQ req;
	ONVIF_RET ret;
	
	printf("soap_GetAnalyticsModules\r\n");

    p_GetAnalyticsModules = xml_node_soap_get(p_body, "GetAnalyticsModules");
	assert(p_GetAnalyticsModules);	

	memset(&req, 0, sizeof(req));
	
	ret = parse_GetAnalyticsModules(p_GetAnalyticsModules, &req);
	if (ONVIF_OK == ret)
	{
		int result = 0;
		GetAnalyticsModules_RES res;
		memset(&res, 0, sizeof(res));
		
		ret = onvif_GetAnalyticsModules(&req, &res);
		if (ONVIF_OK == ret)
		{
			result = soap_build_send_rly(p_user, rx_msg, build_GetAnalyticsModules_rly_xml, (char *)&res);
		}	
		else
			result = soap_build_err_rly(p_user, rx_msg, ret);
		
		onvif_free_configs(&res.AnalyticsModule);
		
		return result;
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_ModifyAnalyticsModules(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_ModifyAnalyticsModules;
	ModifyAnalyticsModules_REQ req;
	ONVIF_RET ret;
	
	printf("soap_ModifyAnalyticsModules\r\n");

    p_ModifyAnalyticsModules = xml_node_soap_get(p_body, "ModifyAnalyticsModules");
	assert(p_ModifyAnalyticsModules);	

	memset(&req, 0, sizeof(req));
	
	ret = parse_ModifyAnalyticsModules(p_ModifyAnalyticsModules, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_ModifyAnalyticsModules(&req);
		onvif_free_configs(&req.AnalyticsModule);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_ModifyAnalyticsModules_rly_xml, NULL);
		}	
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

#endif // end of VIDEO_ANALYTICS

#ifdef PROFILE_G_SUPPORT

int soap_GetRecordingSummary(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	GetRecordingSummary_RES res;
	
	printf("soap_GetRecordingSummary\r\n");

	memset(&res, 0, sizeof(res));
	
	ret = onvif_GetRecordingSummary(&res);
	if (ONVIF_OK == ret)
	{
		return soap_build_send_rly(p_user, rx_msg, build_GetRecordingSummary_rly_xml, (char *)&res);
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetRecordingInformation(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetRecordingInformation;
	XMLN * p_RecordingToken;	
	ONVIF_RET ret = ONVIF_ERR_MISSINGATTR;
	
	printf("soap_GetRecordingInformation\r\n");

	p_GetRecordingInformation = xml_node_soap_get(p_body, "GetRecordingInformation");
	assert(p_GetRecordingInformation);

	p_RecordingToken = xml_node_soap_get(p_GetRecordingInformation, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		GetRecordingInformation_RES res;
		memset(&res, 0, sizeof(res));
		
		ret = onvif_GetRecordingInformation(p_RecordingToken->data, &res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetRecordingInformation_rly_xml, (char *)&res);
		}
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetMediaAttributes(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetMediaAttributes;
	GetMediaAttributes_REQ req;
	ONVIF_RET ret;
	
	printf("soap_GetMediaAttributes\r\n");

	p_GetMediaAttributes = xml_node_soap_get(p_body, "GetMediaAttributes");
	assert(p_GetMediaAttributes);

	memset(&req, 0, sizeof(req));
	
	ret = parse_GetMediaAttributes(p_GetMediaAttributes, &req);
	if (ONVIF_OK == ret)
	{
		GetMediaAttributes_RES res;
		memset(&res, 0, sizeof(res));
	
		ret = onvif_GetMediaAttributes(&req, &res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetMediaAttributes_rly_xml, (char *)&res);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_FindRecordings(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_FindRecordings;
	FindRecordings_REQ req;
	ONVIF_RET ret;
	
	printf("soap_FindRecordings\r\n");

	p_FindRecordings = xml_node_soap_get(p_body, "FindRecordings");
	assert(p_FindRecordings);

	memset(&req, 0, sizeof(req));
	
	ret = parse_FindRecordings(p_FindRecordings, &req);
	if (ONVIF_OK == ret)
	{
		FindRecordings_RES res;
		memset(&res, 0, sizeof(res));
	
		ret = onvif_FindRecordings(&req, &res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_FindRecordings_rly_xml, (char *)&res);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetRecordingSearchResults(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetRecordingSearchResults;
	GetRecordingSearchResults_REQ req;
	ONVIF_RET ret;
	
	printf("soap_GetRecordingSearchResults\r\n");

	p_GetRecordingSearchResults = xml_node_soap_get(p_body, "GetRecordingSearchResults");
	assert(p_GetRecordingSearchResults);

	memset(&req, 0, sizeof(req));
	
	ret = parse_GetRecordingSearchResults(p_GetRecordingSearchResults, &req);
	if (ONVIF_OK == ret)
	{
		GetRecordingSearchResults_RES res;
		memset(&res, 0, sizeof(res));
	
		ret = onvif_GetRecordingSearchResults(&req, &res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetRecordingSearchResults_rly_xml, (char *)&res);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_FindEvents(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_FindEvents;
	FindEvents_REQ req;
	ONVIF_RET ret;
	
	printf("soap_FindEvents\r\n");

	p_FindEvents = xml_node_soap_get(p_body, "FindEvents");
	assert(p_FindEvents);

	memset(&req, 0, sizeof(req));
	
	ret = parse_FindEvents(p_FindEvents, &req);
	if (ONVIF_OK == ret)
	{
		FindEvents_RES res;
		memset(&res, 0, sizeof(res));
	
		ret = onvif_FindEvents(&req, &res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_FindEvents_rly_xml, (char *)&res);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetEventSearchResults(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetEventSearchResults;
	GetEventSearchResults_REQ req;
	ONVIF_RET ret;
	
	printf("soap_GetEventSearchResults\r\n");

	p_GetEventSearchResults = xml_node_soap_get(p_body, "GetEventSearchResults");
	assert(p_GetEventSearchResults);

	memset(&req, 0, sizeof(req));
	
	ret = parse_GetEventSearchResults(p_GetEventSearchResults, &req);
	if (ONVIF_OK == ret)
	{
		GetEventSearchResults_RES res;
		memset(&res, 0, sizeof(res));
	
		ret = onvif_GetEventSearchResults(&req, &res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetEventSearchResults_rly_xml, (char *)&res);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_EndSearch(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_EndSearch;
	EndSearch_REQ req;
	ONVIF_RET ret;
	
	printf("soap_EndSearch\r\n");

	p_EndSearch = xml_node_soap_get(p_body, "EndSearch");
	assert(p_EndSearch);

	memset(&req, 0, sizeof(req));
	
	ret = parse_EndSearch(p_EndSearch, &req);
	if (ONVIF_OK == ret)
	{
		EndSearch_RES res;
		memset(&res, 0, sizeof(res));
	
		ret = onvif_EndSearch(&req, &res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_EndSearch_rly_xml, (char *)&res);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetSearchState(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetSearchState;
	GetSearchState_REQ req;
	ONVIF_RET ret;
	
	printf("soap_GetSearchState\r\n");

	p_GetSearchState = xml_node_soap_get(p_body, "GetSearchState");
	assert(p_GetSearchState);

	memset(&req, 0, sizeof(req));
	
	ret = parse_GetSearchState(p_GetSearchState, &req);
	if (ONVIF_OK == ret)
	{
		GetSearchState_RES res;
		memset(&res, 0, sizeof(res));
	
		ret = onvif_GetSearchState(&req, &res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetSearchState_rly_xml, (char *)&res);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_CreateRecording(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_CreateRecording;
	CreateRecording_REQ req;
	
	printf("soap_CreateRecording\r\n");

	p_CreateRecording = xml_node_soap_get(p_body, "CreateRecording");
	assert(p_CreateRecording);

	memset(&req, 0, sizeof(req));
	
	ret = parse_CreateRecording(p_CreateRecording, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_CreateRecording(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_CreateRecording_rly_xml, req.RecordingToken);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_DeleteRecording(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_DeleteRecording;
	XMLN * p_RecordingToken;
	ONVIF_RET ret = ONVIF_ERR_NO_RECORDING;
	
	printf("soap_DeleteRecording\r\n");

	p_DeleteRecording = xml_node_soap_get(p_body, "DeleteRecording");
	assert(p_DeleteRecording);

	p_RecordingToken = xml_node_soap_get(p_DeleteRecording, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		ret = onvif_DeleteRecording(p_RecordingToken->data);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_DeleteRecording_rly_xml, NULL);
		}
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetRecordings(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	printf("soap_GetRecordings\r\n");
	GetRecordings_RES res;
	ONVIF_RET ret;
	int result = 0;
	memset(&res, 0, sizeof(GetRecordings_RES));
	ret = my_onvif_GetRecordings(&res);
	if (ONVIF_OK == ret)
	{
		 result = soap_build_send_rly(p_user, rx_msg, build_GetRecordings_rly_xml, (char *)&res); 
	}else
	{
		result = soap_build_err_rly(p_user, rx_msg, ret);
	}
	ONVIF_RECORDING *p_free = res.p_recording;
	ONVIF_RECORDING *p_next = NULL;
	while(p_free)
	{
		p_next = NULL;
		if(p_free->next)
		{
			p_next = p_free->next;
		}

		ONVIF_TRACK *p_free_Track = p_free->Recording.Tracks;
		ONVIF_TRACK *p_next_Track = NULL;
		while(p_free_Track)
		{
			p_next_Track = NULL;
			if(p_free_Track->next)
			{
				p_next_Track = p_free_Track->next;
			}
      free_one_item(p_free_Track,ONVIF_TYPE_TRACK);
			//free(p_free_Track);
			p_free_Track = p_next_Track;
		}
    free_one_item(p_free,ONVIF_TYPE_RECORDING);
		//free(p_free);
		p_free = p_next;
	}

	return result;
}

int soap_SetRecordingConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetRecordingConfiguration;
	SetRecordingConfiguration_REQ req;
	
	printf("soap_SetRecordingConfiguration\r\n");

	p_SetRecordingConfiguration = xml_node_soap_get(p_body, "SetRecordingConfiguration");
	assert(p_SetRecordingConfiguration);

	memset(&req, 0, sizeof(req));
	
	ret = parse_SetRecordingConfiguration(p_SetRecordingConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_SetRecordingConfiguration(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_SetRecordingConfiguration_rly_xml, NULL);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetRecordingConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetRecordingConfiguration;
	XMLN * p_RecordingToken;
	ONVIF_RET ret = ONVIF_ERR_NO_RECORDING;
	
	printf("soap_GetRecordingConfiguration\r\n");

	p_GetRecordingConfiguration = xml_node_soap_get(p_body, "GetRecordingConfiguration");
	assert(p_GetRecordingConfiguration);

	p_RecordingToken = xml_node_soap_get(p_GetRecordingConfiguration, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		GetRecordingConfiguration_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(GetRecordingConfiguration_RES));
		res.RecordingToken = (char *)p_RecordingToken->data;
		ret = my_onvif_GetRecordingConfiguration(&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetRecordingConfiguration_rly_xml, (char *)&res); 
		}		 
	
		return soap_build_err_rly(p_user, rx_msg, ret);
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_CreateTrack(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_CreateTrack;
	CreateTrack_REQ req;
	
	printf("soap_CreateTrack\r\n");

	p_CreateTrack = xml_node_soap_get(p_body, "CreateTrack");
	assert(p_CreateTrack);

	memset(&req, 0, sizeof(req));
	
	ret = parse_CreateTrack(p_CreateTrack, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_CreateTrack(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_CreateTrack_rly_xml, req.TrackToken);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_DeleteTrack(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_DeleteTrack;
	DeleteTrack_REQ req;
	
	printf("soap_DeleteTrack\r\n");

	p_DeleteTrack = xml_node_soap_get(p_body, "DeleteTrack");
	assert(p_DeleteTrack);

	memset(&req, 0, sizeof(req));
	
	ret = parse_DeleteTrack(p_DeleteTrack, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_DeleteTrack(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_DeleteTrack_rly_xml, req.TrackToken);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetTrackConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_GetTrackConfiguration;
	GetTrackConfiguration_REQ req;
	
	printf("soap_GetTrackConfiguration\r\n");

	p_GetTrackConfiguration = xml_node_soap_get(p_body, "GetTrackConfiguration");
	assert(p_GetTrackConfiguration);

	memset(&req, 0, sizeof(req));
	
	ret = parse_GetTrackConfiguration(p_GetTrackConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		GetTrackConfiguration_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(GetTrackConfiguration_RES));
		ret = my_onvif_GetTrackConfiguration(&req,&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetTrackConfiguration_rly_xml, (char *)&res); 
		}		 
	
		return soap_build_err_rly(p_user, rx_msg, ret);
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_SetTrackConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetTrackConfiguration;
	SetTrackConfiguration_REQ req;
	
	printf("soap_SetTrackConfiguration\r\n");

	p_SetTrackConfiguration = xml_node_soap_get(p_body, "SetTrackConfiguration");
	assert(p_SetTrackConfiguration);

	memset(&req, 0, sizeof(req));
	
	ret = parse_SetTrackConfiguration(p_SetTrackConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_SetTrackConfiguration(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_SetTrackConfiguration_rly_xml, NULL);
		}	
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_CreateRecordingJob(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_CreateRecordingJob;
	CreateRecordingJob_REQ req;
	
	printf("soap_CreateRecordingJob\r\n");

	p_CreateRecordingJob = xml_node_soap_get(p_body, "CreateRecordingJob");
	assert(p_CreateRecordingJob);

	memset(&req, 0, sizeof(req));
	
	ret = parse_CreateRecordingJob(p_CreateRecordingJob, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_CreateRecordingJob(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_CreateRecordingJob_rly_xml, (char *)&req);
		}	
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_DeleteRecordingJob(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_DeleteRecordingJob;
	XMLN * p_JobToken;
	ONVIF_RET ret = ONVIF_ERR_NO_RECORDINGJOB;
	
	printf("soap_DeleteRecordingJob\r\n");

	p_DeleteRecordingJob = xml_node_soap_get(p_body, "DeleteRecordingJob");
	assert(p_DeleteRecordingJob);

	p_JobToken = xml_node_soap_get(p_DeleteRecordingJob, "JobToken");
	if (p_JobToken && p_JobToken->data)
	{	
		ret = onvif_DeleteRecordingJob(p_JobToken->data);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_DeleteRecordingJob_rly_xml, NULL);
		}
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetRecordingJobs(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	printf("soap_GetRecordingJobs\r\n");
	GetRecordingJobs_RES res;
	ONVIF_RET ret;
	int result = 0;
	memset(&res, 0, sizeof(GetRecordingJobs_RES));
	ret = my_onvif_GetRecordingJobs(&res);
	if (ONVIF_OK == ret)
	{
		result = soap_build_send_rly(p_user, rx_msg, build_GetRecordingJobs_rly_xml, (char *)&res); 
	}
	else
		result = soap_build_err_rly(p_user, rx_msg, ret);
	ONVIF_RECORDINGJOB *p_free = res.recording_jobs;
	ONVIF_RECORDINGJOB *p_next = NULL;
	while(p_free)
	{
		p_next = NULL;
		if(p_free->next)
		{
			p_next = p_free->next;
		}
    free_one_item(p_free,ONVIF_TYPE_RECORDINGJOB);
		//free(p_free);
		p_free = p_next;
	}
	return result;
}

int soap_SetRecordingJobConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetRecordingJobConfiguration;
	SetRecordingJobConfiguration_REQ req;
	
	printf("soap_SetRecordingJobConfiguration\r\n");

	p_SetRecordingJobConfiguration = xml_node_soap_get(p_body, "SetRecordingJobConfiguration");
	assert(p_SetRecordingJobConfiguration);

	memset(&req, 0, sizeof(req));
	
	ret = parse_SetRecordingJobConfiguration(p_SetRecordingJobConfiguration, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_SetRecordingJobConfiguration(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_SetRecordingJobConfiguration_rly_xml, (char *)&req);
		}	
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetRecordingJobConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetRecordingJobConfiguration;
	XMLN * p_JobToken;
	ONVIF_RET ret = ONVIF_ERR_NO_RECORDINGJOB;
	
	printf("soap_GetRecordingJobConfiguration\r\n");

	p_GetRecordingJobConfiguration = xml_node_soap_get(p_body, "GetRecordingJobConfiguration");
	assert(p_GetRecordingJobConfiguration);

	p_JobToken = xml_node_soap_get(p_GetRecordingJobConfiguration, "JobToken");
	if (p_JobToken && p_JobToken->data)
	{
		GetRecordingJobConfiguration_RES res;
		ONVIF_RET ret;
		memset(&res, 0, sizeof(GetRecordingJobConfiguration_RES));
		res.JobToken =(char *)p_JobToken->data;
		ret = my_onvif_GetRecordingJobConfiguration(&res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetRecordingJobConfiguration_rly_xml, (char *)&res); 
		}		 
	
		return soap_build_err_rly(p_user, rx_msg, ret);
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_SetRecordingJobMode(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	XMLN * p_SetRecordingJobMode;
	SetRecordingJobMode_REQ req;
	
	printf("soap_SetRecordingJobMode\r\n");

	p_SetRecordingJobMode = xml_node_soap_get(p_body, "SetRecordingJobMode");
	assert(p_SetRecordingJobMode);

	memset(&req, 0, sizeof(req));
	
	ret = parse_SetRecordingJobMode(p_SetRecordingJobMode, &req);
	if (ONVIF_OK == ret)
	{
		ret = onvif_SetRecordingJobMode(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_SetRecordingJobMode_rly_xml, NULL);
		}	
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetRecordingJobState(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetRecordingJobState;
	XMLN * p_JobToken;
	ONVIF_RET ret = ONVIF_ERR_NO_RECORDINGJOB;
	
	printf("soap_GetRecordingJobConfiguration\r\n");

	p_GetRecordingJobState = xml_node_soap_get(p_body, "GetRecordingJobState");
	assert(p_GetRecordingJobState);

	p_JobToken = xml_node_soap_get(p_GetRecordingJobState, "JobToken");
	if (p_JobToken && p_JobToken->data)
	{
		onvif_RecordingJobStateInformation state;
		memset(&state, 0, sizeof(state));
		
		ret = onvif_GetRecordingJobState(p_JobToken->data, &state);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetRecordingJobState_rly_xml, (char*)&state);
		}
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetRecordingOptions(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetRecordingOptions;
	XMLN * p_RecordingToken;
	ONVIF_RET ret = ONVIF_ERR_NO_RECORDING;
	
	printf("soap_GetRecordingOptions\r\n");

	p_GetRecordingOptions = xml_node_soap_get(p_body, "GetRecordingOptions");
	assert(p_GetRecordingOptions);

	p_RecordingToken = xml_node_soap_get(p_GetRecordingOptions, "RecordingToken");
	if (p_RecordingToken && p_RecordingToken->data)
	{
		onvif_RecordingOptions options;
		memset(&options, 0, sizeof(options));
		
		ret = onvif_GetRecordingOptions(p_RecordingToken->data, &options);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetRecordingOptions_rly_xml, (char *)&options);
		}
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetReplayUri(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_GetReplayUri;
	GetReplayUri_REQ req;
	ONVIF_RET ret;
	
	printf("soap_GetReplayUri\r\n");

	p_GetReplayUri = xml_node_soap_get(p_body, "GetReplayUri");
	assert(p_GetReplayUri);

	memset(&req, 0, sizeof(req));
	
	ret = parse_GetReplayUri(p_GetReplayUri, &req);
	if (ONVIF_OK == ret)
	{
		GetReplayUri_RES res;
		memset(&res, 0, sizeof(res));
	
		ret = onvif_GetReplayUri(&req, &res);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_GetReplayUri_rly_xml, (char *)&res);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_GetReplayConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	ONVIF_RET ret;
	
	GetReplayConfiguration_RES res;
	memset(&res, 0, sizeof(res));

	ret = onvif_GetReplayConfiguration(&res);
	if (ONVIF_OK == ret)
	{
		return soap_build_send_rly(p_user, rx_msg, build_GetReplayConfiguration_rly_xml, (char *)&res);
	}

	return soap_build_err_rly(p_user, rx_msg, ret);
}

int soap_SetReplayConfiguration(HTTPCLN * p_user, HTTPMSG * rx_msg, XMLN * p_body)
{
	XMLN * p_SetReplayConfiguration;
	SetReplayConfiguration_REQ req;
	ONVIF_RET ret;
	
	printf("soap_SetReplayConfiguration\r\n");

	p_SetReplayConfiguration = xml_node_soap_get(p_body, "SetReplayConfiguration");
	assert(p_SetReplayConfiguration);

	memset(&req, 0, sizeof(req));
	
	ret = parse_SetReplayConfiguration(p_SetReplayConfiguration, &req);
	if (ONVIF_OK == ret)
	{	
		ret = onvif_SetReplayConfiguration(&req);
		if (ONVIF_OK == ret)
		{
			return soap_build_send_rly(p_user, rx_msg, build_SetReplayConfiguration_rly_xml, NULL);
		}
	}
	
	return soap_build_err_rly(p_user, rx_msg, ret);
}

#endif	// end of PROFILE_G_SUPPORT


/*********************************************************/
void soap_calc_digest(const char *created, unsigned char *nonce, int noncelen, const char *password, unsigned char hash[20])
{
	sha1_context ctx;
	
	sha1_starts(&ctx);
	sha1_update(&ctx, (unsigned char *)nonce, noncelen);
	sha1_update(&ctx, (unsigned char *)created, strlen(created));
	sha1_update(&ctx, (unsigned char *)password, strlen(password));
	sha1_finish(&ctx, (unsigned char *)hash);
}

BOOL soap_auth_process(XMLN * p_Security)
{
	int nonce_len;
	XMLN * p_UsernameToken;
	XMLN * p_Username;
	XMLN * p_Password;
	XMLN * p_Nonce;
	XMLN * p_Created;
	char HABase64[100];
	const char * auth_pass;
	unsigned char nonce[200];
	unsigned char HA[20];	
	const char * p_Type;

	p_UsernameToken = xml_node_soap_get(p_Security, "wsse:UsernameToken");
	if (NULL == p_UsernameToken)
	{
		return FALSE;
	}

	p_Username = xml_node_soap_get(p_UsernameToken, "wsse:Username");
	p_Password = xml_node_soap_get(p_UsernameToken, "wsse:Password");
	p_Nonce = xml_node_soap_get(p_UsernameToken, "wsse:Nonce");
	p_Created = xml_node_soap_get(p_UsernameToken, "wsse:Created");

	if (NULL == p_Username || NULL == p_Username->data || 
		NULL == p_Password || NULL == p_Password->data || 
		NULL == p_Nonce || NULL == p_Nonce->data ||
		NULL == p_Created || NULL == p_Created->data)
	{
		return FALSE;
	}
	
    p_Type = xml_attr_get(p_Password, "Type");
    if (NULL == p_Type)
    {
        return FALSE;
    }    

	auth_pass = onvif_get_user_pass(p_Username->data);
	if (NULL == auth_pass)	// user not exist
	{
		return FALSE;
	}
		
	nonce_len = base64_decode_onvif(p_Nonce->data, nonce, sizeof(nonce));	
	
	soap_calc_digest(p_Created->data, nonce, nonce_len, auth_pass, HA);
	base64_encode_onvif(HA, 20, HABase64, sizeof(HABase64));

	if (strcmp(HABase64, p_Password->data) == 0)
	{
		return TRUE;
	}
	
	return FALSE;
}

BOOL soap_digest_auth_process(HD_AUTH_INFO * p_auth)
{
    HASHHEX HA1;
	HASHHEX HA2 = "";
	const char * auth_pass;

	char calc_response[36];

    auth_pass = onvif_get_user_pass(p_auth->auth_name);
	if (NULL == auth_pass)	// user not exist
	{
		return FALSE;
	}
	
	DigestCalcHA1("md5", p_auth->auth_name, g_auth_info.auth_realm, auth_pass, p_auth->auth_nonce, p_auth->auth_cnonce, HA1);
    
	DigestCalcResponse(HA1, g_auth_info.auth_nonce, p_auth->auth_ncstr, p_auth->auth_cnonce,
		p_auth->auth_qop, "POST", p_auth->auth_uri, HA2, calc_response);
		
	if (strcmp(calc_response, p_auth->auth_response) == 0)
		return TRUE;

	return FALSE;
}


/*********************************************************
 *
 * process soap request
 *
 * p_user [in] 	 --- http client
 * rx_msg [in] --- http message
 *
**********************************************************/ 
void soap_process_request(HTTPCLN * p_user, HTTPMSG * rx_msg)
{
    int errcode = 401;
	BOOL auth = FALSE;	
	char * p_xml;
	XMLN * p_node;
	XMLN * p_header;
	XMLN * p_body;

	p_xml = get_http_ctt(rx_msg);
	if (NULL == p_xml)
	{
		printf("soap_process::get_http_ctt ret null!!!\r\n");
		return;
	}

	//printf("soap_process::rx xml:\r\n%s\r\n", p_xml);

	p_node = xxx_hxml_parse(p_xml, strlen(p_xml));
	if (NULL == p_node || NULL == p_node->name)
	{
		printf("soap_process::xxx_hxml_parse ret null!!!\r\n");
		return;
	}
	
	if (soap_strcmp(p_node->name, "Envelope") != 0)
	{
		printf("soap_process::node name[%s] != [s:Envelope]!!!\r\n", p_node->name);
		xml_node_del(p_node);
		return;
	}

	p_header = xml_node_soap_get(p_node, "Header");
	if (p_header)
	{
		XMLN * p_Security = xml_node_soap_get(p_header, "Security");
		if (p_Security)
		{
		    errcode = 400;
			auth = soap_auth_process(p_Security);
		}
	}
	Onvif_Info* pOnvifInfo = GetOnvifInfo();

	if (pOnvifInfo->need_auth && !auth)
	{
	    HD_AUTH_INFO auth_info;
	    
	    if (http_get_auth_digest_info(rx_msg, &auth_info))
	    {
	        auth = soap_digest_auth_process(&auth_info);
	    }

	    if (auth == FALSE)
	    {
    		soap_security_rly(p_user, rx_msg, errcode);
    		xml_node_del(p_node);
    		return;
		}
	}

	p_body = xml_node_soap_get(p_node, "Body");
	if (NULL == p_body)
	{
		printf("soap_process::xml_node_soap_get[s:Body] ret null!!!\r\n");
		xml_node_del(p_node);
		return;
	}

	if (NULL == p_body->f_child)
	{
		printf("soap_process::body first child node is null!!!\r\n");
	}	
	else if (NULL == p_body->f_child->name)
	{
		printf("soap_process::body first child node name is null!!!\r\n");
	}	
	else
	{
		printf("soap_process::body first child node name[%s].\r\n", p_body->f_child->name);
	}

	if (soap_strcmp(p_body->f_child->name, "GetDeviceInformation") == 0)
	{
		soap_GetDeviceInformation(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetCapabilities") == 0)
	{
        soap_GetCapabilities(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetProfiles") == 0)
	{
		soap_GetProfiles(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetProfile") == 0)
	{
		soap_GetProfile(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "CreateProfile") == 0)
	{
		soap_CreateProfile(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "DeleteProfile") == 0)
	{
		soap_DeleteProfile(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "AddVideoSourceConfiguration") == 0)
	{
		soap_AddVideoSourceConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "RemoveVideoSourceConfiguration") == 0)
	{
		soap_RemoveVideoSourceConfiguration(p_user, rx_msg, p_body);
	}	
	else if (soap_strcmp(p_body->f_child->name, "AddAudioSourceConfiguration") == 0)
	{
		soap_AddAudioSourceConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "RemoveAudioSourceConfiguration") == 0)
	{
		soap_RemoveAudioSourceConfiguration(p_user, rx_msg, p_body);
	}	
	else if (soap_strcmp(p_body->f_child->name, "AddVideoEncoderConfiguration") == 0)
	{
		soap_AddVideoEncoderConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "RemoveVideoEncoderConfiguration") == 0)
	{
		soap_RemoveVideoEncoderConfiguration(p_user, rx_msg, p_body);
	}	
	else if (soap_strcmp(p_body->f_child->name, "AddAudioEncoderConfiguration") == 0)
	{
		soap_AddAudioEncoderConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "RemoveAudioEncoderConfiguration") == 0)
	{
		soap_RemoveAudioEncoderConfiguration(p_user, rx_msg, p_body);
	}	
	else if (soap_strcmp(p_body->f_child->name, "GetSystemDateAndTime") == 0)
	{
		soap_GetSystemDateAndTime(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetSystemDateAndTime") == 0)
	{
        soap_SetSystemDateAndTime(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetStreamUri") == 0)
	{
		soap_GetStreamUri(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetNetworkInterfaces") == 0)
	{	
		soap_GetNetworkInterfaces(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetNetworkInterfaces") == 0)
	{
		soap_SetNetworkInterfaces(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetVideoSources") == 0)
	{
	    soap_GetVideoSources(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetAudioSources") == 0)
	{
	    soap_GetAudioSources(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetVideoEncoderConfigurations") == 0)
	{
		soap_GetVideoEncoderConfigurations(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetCompatibleVideoEncoderConfigurations") == 0)
	{
		soap_GetCompatibleVideoEncoderConfigurations(p_user, rx_msg, p_body);
	}	
	else if (soap_strcmp(p_body->f_child->name, "GetAudioEncoderConfigurations") == 0)
	{
		soap_GetAudioEncoderConfigurations(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetCompatibleAudioEncoderConfigurations") == 0)
	{
		soap_GetCompatibleAudioEncoderConfigurations(p_user, rx_msg, p_body);
	}	
	else if (soap_strcmp(p_body->f_child->name, "GetVideoSourceConfigurations") == 0)
	{
		soap_GetVideoSourceConfigurations(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetVideoSourceConfiguration") == 0)
	{
		soap_GetVideoSourceConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetVideoSourceConfigurationOptions") == 0)
	{
		soap_GetVideoSourceConfigurationOptions(p_user, rx_msg, p_body);
	}	
	else if (soap_strcmp(p_body->f_child->name, "SetVideoSourceConfiguration") == 0)
	{
		soap_SetVideoSourceConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetAudioSourceConfigurations") == 0)
	{
		soap_GetAudioSourceConfigurations(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetCompatibleAudioSourceConfigurations") == 0)
	{
		soap_GetCompatibleAudioSourceConfigurations(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetAudioSourceConfigurationOptions") == 0)
	{
		soap_GetAudioSourceConfigurationOptions(p_user, rx_msg, p_body);
	}	
	else if (soap_strcmp(p_body->f_child->name, "GetAudioSourceConfiguration") == 0)
	{
		soap_GetAudioSourceConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetAudioSourceConfiguration") == 0)
	{
		soap_SetAudioSourceConfiguration(p_user, rx_msg, p_body);
	}	
	else if (soap_strcmp(p_body->f_child->name, "GetVideoEncoderConfiguration") == 0)
	{
	    soap_GetVideoEncoderConfiguration(p_user, rx_msg, p_body);	    	
	}
	else if (soap_strcmp(p_body->f_child->name, "SetVideoEncoderConfiguration") == 0)
	{
        soap_SetVideoEncoderConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetAudioEncoderConfiguration") == 0)
	{
		soap_GetAudioEncoderConfiguration(p_user, rx_msg, p_body);
	}	
	else if (soap_strcmp(p_body->f_child->name, "SetAudioEncoderConfiguration") == 0)
	{
		soap_SetAudioEncoderConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetVideoEncoderConfigurationOptions") == 0)
	{
		soap_GetVideoEncoderConfigurationOptions(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetAudioEncoderConfigurationOptions") == 0)
	{
		soap_GetAudioEncoderConfigurationOptions(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetCompatibleVideoSourceConfigurations") == 0)
	{
		soap_GetCompatibleVideoSourceConfigurations(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "SystemReboot") == 0)
	{
		soap_SystemReboot(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetSystemFactoryDefault") == 0)
	{
		soap_SetSystemFactoryDefault(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetSystemLog") == 0)
	{
		soap_GetSystemLog(p_user, rx_msg, p_body);
	}	
	else if (soap_strcmp(p_body->f_child->name, "GetServices") == 0)
	{
		soap_GetServices(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetSnapshotUri") == 0)
	{
		soap_GetSnapshotUri(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetScopes") == 0)
	{
		soap_GetScopes(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "AddScopes") == 0)
	{
		soap_AddScopes(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetScopes") == 0)
	{
		soap_SetScopes(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "RemoveScopes") == 0)
	{
		soap_RemoveScopes(p_user, rx_msg, p_body);
	}	
	else if (soap_strcmp(p_body->f_child->name, "GetHostname") == 0)
	{
		soap_GetHostname(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetHostname") == 0)
	{
		soap_SetHostname(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetHostnameFromDHCP") == 0)
	{
		soap_SetHostnameFromDHCP(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetNetworkProtocols") == 0)
	{
		soap_GetNetworkProtocols(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetNetworkProtocols") == 0)
	{
		soap_SetNetworkProtocols(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetNetworkDefaultGateway") == 0)
	{
		soap_GetNetworkDefaultGateway(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetNetworkDefaultGateway") == 0)
	{
		soap_SetNetworkDefaultGateway(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetDiscoveryMode") == 0)
	{
		soap_GetDiscoveryMode(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetDiscoveryMode") == 0)
	{
		soap_SetDiscoveryMode(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetDNS") == 0)
	{
		soap_GetDNS(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetDNS") == 0)
	{
		soap_SetDNS(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetNTP") == 0)
	{
		soap_GetNTP(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetNTP") == 0)
	{
		soap_SetNTP(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetServiceCapabilities") == 0)
	{			
		soap_GetServiceCapabilities(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetEventProperties") == 0)
	{
		soap_GetEventProperties(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "Subscribe") == 0)
	{
	    soap_Subscribe(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "Unsubscribe") == 0)
	{
		soap_Unsubscribe(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_body->f_child->name, "Renew") == 0)
	{
		soap_Renew(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_body->f_child->name, "CreatePullPointSubscription") == 0)
	{
		soap_CreatePullPointSubscription(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_body->f_child->name, "PullMessages") == 0)
	{
		soap_PullMessages(p_user, rx_msg, p_body, p_header);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetSynchronizationPoint") == 0)
	{
		soap_SetSynchronizationPoint(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetWsdlUrl") == 0)
	{
		soap_GetWsdlUrl(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetNodes") == 0)
	{
		soap_GetNodes(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetNode") == 0)
	{
		soap_GetNode(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetConfigurations") == 0)
	{
		soap_GetConfigurations(p_user, rx_msg);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetConfiguration") == 0)
	{
		soap_GetConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "AddPTZConfiguration") == 0)
	{
		soap_AddPTZConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "RemovePTZConfiguration") == 0)
	{
		soap_RemovePTZConfiguration(p_user, rx_msg, p_body);
	}	
	else if (soap_strcmp(p_body->f_child->name, "GetConfigurationOptions") == 0)
	{	
		soap_GetConfigurationOptions(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetStatus") == 0)
	{
		soap_GetStatus(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "ContinuousMove") == 0)
	{
		soap_ContinuousMove(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "Stop") == 0)
	{
		soap_Stop(p_user, rx_msg, p_body);
	}
    else if (soap_strcmp(p_body->f_child->name, "AbsoluteMove") == 0)
	{
		soap_AbsoluteMove(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "RelativeMove") == 0)
	{
		soap_RelativeMove(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetPreset") == 0)
	{
		soap_SetPreset(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetPresets") == 0)
	{
		soap_GetPresets(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "RemovePreset") == 0)
	{
		soap_RemovePreset(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GotoPreset") == 0)
	{
		soap_GotoPreset(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GotoHomePosition") == 0)
	{
		soap_GotoHomePosition(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetHomePosition") == 0)
	{
		soap_SetHomePosition(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetGuaranteedNumberOfVideoEncoderInstances") == 0)
	{
		soap_GetGuaranteedNumberOfVideoEncoderInstances(p_user, rx_msg, p_body);		
	}
	else if (soap_strcmp(p_body->f_child->name, "GetImagingSettings") == 0)
	{
		soap_GetImagingSettings(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetImagingSettings") == 0)
	{
		soap_SetImagingSettings(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetOptions") == 0)
	{
		soap_GetOptions(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetMoveOptions") == 0)
	{
		soap_GetMoveOptions(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "Move") == 0)
	{
		soap_Move(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetUsers") == 0)
	{
		soap_GetUsers(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "CreateUsers") == 0)
	{
		soap_CreateUsers(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "DeleteUsers") == 0)
	{
		soap_DeleteUsers(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetUser") == 0)
	{
		soap_SetUser(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "UpgradeSystemFirmware") == 0)
	{
		soap_UpgradeSystemFirmware(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "StartFirmwareUpgrade") == 0)
	{
		soap_StartFirmwareUpgrade(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetOSDs") == 0) 
	{
		soap_GetOSDs(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetOSD") == 0) 
	{
		soap_GetOSD(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetOSD") == 0) 
	{
		soap_SetOSD(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetOSDOptions") == 0) 
	{
		soap_GetOSDOptions(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "CreateOSD") == 0) 
	{
		soap_CreateOSD(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "DeleteOSD") == 0) 
	{
		soap_DeleteOSD(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetConfiguration") == 0)
	{
		soap_SetConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "StartMulticastStreaming") == 0)
	{
		soap_StartMulticastStreaming(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "StopMulticastStreaming") == 0)
	{
		soap_StopMulticastStreaming(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetMetadataConfigurations") == 0)
	{
		soap_GetMetadataConfigurations(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetMetadataConfiguration") == 0)
	{
		soap_GetMetadataConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetCompatibleMetadataConfigurations") == 0)
	{
		soap_GetCompatibleMetadataConfigurations(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetMetadataConfigurationOptions") == 0)
	{
		soap_GetMetadataConfigurationOptions(p_user, rx_msg, p_body);
	}	 
	else if (soap_strcmp(p_body->f_child->name, "SetMetadataConfiguration") == 0)
	{
		soap_SetMetadataConfiguration(p_user, rx_msg, p_body);
	}	
	else if (soap_strcmp(p_body->f_child->name, "AddMetadataConfiguration") == 0)
	{
		soap_AddMetadataConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "RemoveMetadataConfiguration") == 0)
	{
		soap_RemoveMetadataConfiguration(p_user, rx_msg, p_body);
	}

#ifdef VIDEO_ANALYTICS	
	else if (soap_strcmp(p_body->f_child->name, "GetVideoAnalyticsConfigurations") == 0)
	{
		soap_GetVideoAnalyticsConfigurations(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "AddVideoAnalyticsConfiguration") == 0)
	{
		soap_AddVideoAnalyticsConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetVideoAnalyticsConfiguration") == 0)
	{
		soap_GetVideoAnalyticsConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "RemoveVideoAnalyticsConfiguration") == 0)
	{
		soap_RemoveVideoAnalyticsConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetVideoAnalyticsConfiguration") == 0)
	{
		soap_SetVideoAnalyticsConfiguration(p_user, rx_msg, p_body);
	}	
	else if (soap_strcmp(p_body->f_child->name, "GetSupportedRules") == 0)
	{
		soap_GetSupportedRules(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "CreateRules") == 0)
	{
		soap_CreateRules(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "DeleteRules") == 0)
	{
		soap_DeleteRules(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetRules") == 0)
	{
		soap_GetRules(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "ModifyRules") == 0)
	{
		soap_ModifyRules(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "CreateAnalyticsModules") == 0)
	{
		soap_CreateAnalyticsModules(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "DeleteAnalyticsModules") == 0)
	{
		soap_DeleteAnalyticsModules(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetAnalyticsModules") == 0)
	{
		soap_GetAnalyticsModules(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "ModifyAnalyticsModules") == 0)
	{
		soap_ModifyAnalyticsModules(p_user, rx_msg, p_body);
	}
#endif	// endif of VIDEO_ANALYTICS

#ifdef PROFILE_G_SUPPORT
	else if (soap_strcmp(p_body->f_child->name, "GetRecordingSummary") == 0)
	{
		soap_GetRecordingSummary(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetRecordingInformation") == 0)
	{
		soap_GetRecordingInformation(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetMediaAttributes") == 0)
	{
		soap_GetMediaAttributes(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "FindRecordings") == 0)
	{
		soap_FindRecordings(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetRecordingSearchResults") == 0)
	{
		soap_GetRecordingSearchResults(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "FindEvents") == 0)
	{
		soap_FindEvents(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetEventSearchResults") == 0)
	{
		soap_GetEventSearchResults(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "EndSearch") == 0)
	{
		soap_EndSearch(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetSearchState") == 0)
	{
		soap_GetSearchState(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "CreateRecording") == 0)
	{
		soap_CreateRecording(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "DeleteRecording") == 0)
	{
		soap_DeleteRecording(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetRecordings") == 0)
	{
		soap_GetRecordings(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetRecordingConfiguration") == 0)
	{
		soap_SetRecordingConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetRecordingConfiguration") == 0)
	{
		soap_GetRecordingConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "CreateTrack") == 0)
	{
		soap_CreateTrack(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "DeleteTrack") == 0)
	{
		soap_DeleteTrack(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetTrackConfiguration") == 0)
	{
		soap_GetTrackConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetTrackConfiguration") == 0)
	{
		soap_SetTrackConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "CreateRecordingJob") == 0)
	{
		soap_CreateRecordingJob(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "DeleteRecordingJob") == 0)
	{
		soap_DeleteRecordingJob(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetRecordingJobs") == 0)
	{
		soap_GetRecordingJobs(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetRecordingJobConfiguration") == 0)
	{
		soap_SetRecordingJobConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetRecordingJobConfiguration") == 0)
	{
		soap_GetRecordingJobConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetRecordingJobMode") == 0)
	{
		soap_SetRecordingJobMode(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetRecordingJobState") == 0)
	{
		soap_GetRecordingJobState(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetRecordingOptions") == 0)
	{
		soap_GetRecordingOptions(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetReplayUri") == 0)
	{
		soap_GetReplayUri(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "GetReplayConfiguration") == 0)
	{
		soap_GetReplayConfiguration(p_user, rx_msg, p_body);
	}
	else if (soap_strcmp(p_body->f_child->name, "SetReplayConfiguration") == 0)
	{
		soap_SetReplayConfiguration(p_user, rx_msg, p_body);
	}
#endif	// end of PROFILE_G_SUPPORT

	else
	{
		soap_err_def_rly(p_user, rx_msg);
	}
	
	xml_node_del(p_node);
}



