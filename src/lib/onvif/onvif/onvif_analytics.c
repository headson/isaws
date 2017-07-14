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
#include "onvif/onvif/onvif.h"
#include "onvif/onvif/onvif_cm.h"
#include "onvif/onvif/onvif_analytics.h"

#ifdef VIDEO_ANALYTICS

/***************************************************************************************/

/**
 The possible return value
 	ONVIF_ERR_NO_PROFILE
 	ONVIF_ERR_NO_CONFIG
*/
ONVIF_RET onvif_AddVideoAnalyticsConfiguration(AddVideoAnalyticsConfiguration_REQ * p_req)
{
	// todo : add video analytics configuration code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_AddVideoAnalyticsConfiguration(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}

	
	return ONVIF_OK;
}

/**
 The possible return value
 	ONVIF_ERR_NO_PROFILE
*/
ONVIF_RET onvif_RemoveVideoAnalyticsConfiguration(RemoveVideoAnalyticsConfiguration_REQ * p_req)
{
	// todo : remove video analytics configuration code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_RemoveVideoAnalyticsConfiguration(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	return ONVIF_OK;
}

/**
 The possible return value
 	ONVIF_ERR_NO_CONFIG
 	ONVIF_ERR_CONFIG_MODIFY
 	ONVIF_ERR_CONFIGURATION_CONFLICT
*/
ONVIF_RET onvif_SetVideoAnalyticsConfiguration(SetVideoAnalyticsConfiguration_REQ * p_req)
{
	// todo : set video analytics configuration code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetVideoAnalyticsConfiguration(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return value
 	ONVIF_ERR_NO_CONFIG
*/
ONVIF_RET onvif_GetSupportedRules(GetSupportedRules_REQ * p_req, GetSupportedRules_RES * p_res)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_GetSupportedRules_ex(p_req,p_res);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return value
 	ONVIF_ERR_NO_CONFIG
 	ONVIF_ERR_INVALID_RULE,
	ONVIF_ERR_RULE_ALREADY_EXIST
	ONVIF_ERR_TOO_MANY_RULES
	ONVIF_ERR_CONFIGURATION_CONFLICT
*/
ONVIF_RET onvif_CreateRules(CreateRules_REQ * p_req)
{
	ONVIF_VACFG * p_va_cfg;
	ONVIF_CONFIG * p_config;

	if (NULL == p_req->Rule)
	{
		return ONVIF_ERR_INVALID_RULE;
	}

	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_CreateRules_ex(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return value
 	ONVIF_ERR_NO_CONFIG
	ONVIF_ERR_RULE_NOT_EXIST
	ONVIF_ERR_CONFIGURATION_CONFLICT
*/
ONVIF_RET onvif_DeleteRules(DeleteRules_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_DeleteRules_ex(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return value
 	ONVIF_ERR_NO_CONFIG
*/
ONVIF_RET onvif_GetRules(GetRules_REQ * p_req, GetRules_RES * p_res)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_GetRules_ex(p_req,p_res);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return value
 	ONVIF_ERR_NO_CONFIG
 	ONVIF_ERR_INVALID_RULE,
	ONVIF_ERR_RULE_NOT_EXIST
	ONVIF_ERR_TOO_MANY_RULES
	ONVIF_ERR_CONFIGURATION_CONFLICT
*/
ONVIF_RET onvif_ModifyRules(ModifyRules_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_ModifyRules_ex(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return value
 	ONVIF_ERR_NO_CONFIG
 	ONVIF_ERR_NAME_ALREADY_EXIST
	ONVIF_ERR_TOO_MANY_MODULES
	ONVIF_ERR_INVALID_MODULE
	ONVIF_ERR_CONFIGURATION_CONFLICT
*/
ONVIF_RET onvif_CreateAnalyticsModules(CreateAnalyticsModules_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_CreateAnalyticsModules(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return value
 	ONVIF_ERR_NO_CONFIG
 	ONVIF_ERR_NAME_NOT_EXIST
	ONVIF_ERR_CONFIGURATION_CONFLICT
*/
ONVIF_RET onvif_DeleteAnalyticsModules(DeleteAnalyticsModules_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_DeleteAnalyticsModules(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return value
 	ONVIF_ERR_NO_CONFIG
*/
ONVIF_RET onvif_GetAnalyticsModules(GetAnalyticsModules_REQ * p_req, GetAnalyticsModules_RES * p_res)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_GetAnalyticsModules_ex(p_req,p_res);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return value
 	ONVIF_ERR_NO_CONFIG
 	ONVIF_ERR_NAME_NOT_EXIST
	ONVIF_ERR_TOO_MANY_MODULES
	ONVIF_ERR_INVALID_MODULE
	ONVIF_ERR_CONFIGURATION_CONFLICT
*/
ONVIF_RET onvif_ModifyAnalyticsModules(ModifyAnalyticsModules_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_ModifyAnalyticsModules_ex(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}


#endif	// end of VIDEO_ANALYTICS


