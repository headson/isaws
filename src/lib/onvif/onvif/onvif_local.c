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
#include "onvif/onvif/xml_node.h"
#include "onvif/onvif/onvif_util.h"
#include "onvif/localsdk/VzLocalSdk.h"
#include "onvif/onvif/onvif_local.h"

/***************************************************************************************/
extern ONVIF_CLS g_onvif_cls;

/***************************************************************************************/

ONVIF_RET LocalSDK_ErrHandle(void)
{
	LocalApi_Err_Type ErrType = (LocalApi_Err_Type)LocalSDK_GetErr();
	ONVIF_RET soapErr = -1;
	switch(ErrType)
	{
	case VZ_LOCALAPI_TZ_ERR:
		soapErr = ONVIF_ERR_INVALID_TIMEZONE;
		break;
	case VZ_LOCALAPI_DATETIME_ERR:
		soapErr = ONVIF_ERR_INVALID_DATETIME;
		break;
	case VZ_LOCALAPI_NTP_ERR:
		soapErr = ONVIF_ERR_INVALID_IPV4_ADDR;
		break;
	case VZ_LOCALAPI_ACTION_FAIL:
		soapErr = ONVIF_ERR_CONFIG_MODIFY;
		break;
	case VZ_LOCALAPI_ACTION_UNSUPPORT:
		soapErr = ONVIF_ERR_SERVICE_NOT_SUPPORT;
		break;
	case VZ_LOCALAPI_SCOPE_EXCEED:
		soapErr = ONVIF_ERR_TOO_MANY_SCOPES;
		break;
	case VZ_LOCALAPI_SCOPELIST_EXCEED:
		soapErr = ONVIF_ERR_TOO_MANY_SCOPES;
		break;
	case VZ_LOCALAPI_SCOPE_OVERWRITE:
		soapErr = ONVIF_ERR_SCOPE_OVERWRITE;
		break;
	case VZ_LOCALAPI_SCOPE_NONEXIST:
		soapErr = ONVIF_ERR_NO_SCOPE;
		break;
	case VZ_LOCALAPI_USER_EXCEED:
		soapErr = ONVIF_ERR_TOO_MANY_USERS;
		break;
	case VZ_LOCALAPI_USER_ANONYMOUS:
		soapErr = ONVIF_ERR_ANONYMOUS_NOT_ALLOWED;
		break;
	case VZ_LOCALAPI_USERNAME_LONG:
		soapErr = ONVIF_ERR_USERNAME_TOO_LONG;
		break;
	case VZ_LOCALAPI_USERNAME_SHORT:
		soapErr = ONVIF_ERR_USERNAME_TOO_SHORT;
		break;
	case VZ_LOCALAPI_PASSWORD_LONG:
		soapErr = ONVIF_ERR_PASSWORD_TOO_LONG;
		break;
	case VZ_LOCALAPI_PASSWORD_SHORT:
		soapErr = ONVIF_ERR_PASSWORD;
		break;
	case VZ_LOCALAPI_USERNAME_CLASH:
		soapErr = ONVIF_ERR_USERNAME_EXIST;
		break;
	case VZ_LOCALAPI_USERNAME_MISS:
		soapErr = ONVIF_ERR_USERNAME_MISSING;
		break;
	case VZ_LOCALAPI_USERNAME_FIXED:
		soapErr = ONVIF_ERR_FIXED_USER;
		break;
	case VZ_LOCALAPI_VALUE_INVALID:
		soapErr = ONVIF_ERR_CONFIG_MODIFY;
		break;
	case VZ_LOCALAPI_IMAGEVALUE_INVALID:
		soapErr = ONVIF_ERR_SETTINGS_INVALID;
		break;
	case VZ_LOCALAPI_NETWORK_INTERFACE_INVALID:
		soapErr = ONVIF_ERR_INVALID_NETWORK_INTERFACE;
		break;
	case VZ_LOCALAPI_IPV4_INVALID:
		soapErr = ONVIF_ERR_INVALID_IPV4_ADDR;
		break;
	case VZ_LOCALAPI_ENABLE_TLS_FAIL:
		soapErr = ONVIF_ERR_SERVICE_NOT_SUPPORT;
		break;
	case VZ_LOCALAPI_RULE_EXCEED:
		soapErr = ONVIF_ERR_TOO_MANY_RULES;
		break;
	case VZ_LOCALAPI_RULE_CLASH:
		soapErr = ONVIF_ERR_RULE_ALREADY_EXIST;
		break;
	case VZ_LOCALAPI_RULE_NONEXIST:
		soapErr = ONVIF_ERR_RULE_NOT_EXIST;
		break;
	case VZ_LOCALAPI_RULE_INVALID:
		soapErr = ONVIF_ERR_INVALID_RULE;
		break;
	case VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST:
		soapErr = ONVIF_ERR_NO_CONFIG;
		break;
	case VZ_LOCALAPI_PROFILE_NONEXIST:
		soapErr = ONVIF_ERR_NO_PROFILE;
		break;
	case VZ_LOCALAPI_HOSTNAME_INVALID:
		soapErr = ONVIF_ERR_INVALID_HOSTNAME;
		break;
	case VZ_LOCALAPI_RECORDING_EXCEED:
		soapErr = ONVIF_ERR_MAX_RECORDING;
		break;
	case VZ_LOCALAPI_RECORDINGJOB_EXCEED:
		soapErr = ONVIF_ERR_MAX_RECORDING_JOBS;
		break;
	case VZ_LOCALAPI_SERIALPORT_INVALID:
		soapErr = ONVIF_ERR_INVALID_IPV4_ADDR;
		break;
	case VZ_LOCALAPI_PRESET_EXCEED:
		soapErr = ONVIF_ERR_TOO_MANY_PRESETS;
		break;
	case VZ_LOCALAPI_PRESETNAME_INVALID:
		soapErr = ONVIF_ERR_INVALID_IPV4_ADDR;
		break;
	case VZ_LOCALAPI_VIDEOANALYTICS_CHANNEL_EXCEED:
		soapErr = ONVIF_ERR_INVALID_IPV4_ADDR;
		break;
	case VZ_LOCALAPI_RECEIVER_EXCEED:
		soapErr = ONVIF_ERR_MAX_RECEIVERS;
		break;
	case VZ_LOCALAPI_NO_PTZNODE:
		soapErr = ONVIF_ERR_NO_ENTITY;
		break;
	case VZ_LOCALAPI_NO_VSOURCE:
		soapErr = ONVIF_ERR_NO_SOURCE;
		break;
	case VZ_LOCALAPI_TRACK_EXCEED:
		soapErr = ONVIF_ERR_MAX_TRACKS;
		break;
	case VZ_LOCALAPI_NO_TRACK:
		soapErr = ONVIF_ERR_NO_TRACK;
		break;
	case VZ_LOCALAPI_NO_RECORDING:
		soapErr = ONVIF_ERR_NO_RECORDING;
		break;
	case VZ_LOCALAPI_NO_RECORDINGJOB:
		soapErr = ONVIF_ERR_NO_RECORDINGJOB;
		break;
	case VZ_LOCALAPI_BAD_MODE:
		soapErr = ONVIF_ERR_BAD_MODE;
		break;
	case VZ_LOCALAPI_OSDS_EXCEED:
		soapErr = ONVIF_ERR_MAX_OSDS;
		break;
		
	default:
		break;
	}
		return soapErr;
}

static void iGetOneVideoAnalyticsCfg(ONVIF_VACFG * p_cfg,vz_onvif_VideoAnalyticsCfg *vz_AnalyticsCfg)
{
	p_cfg->rules = NULL;
	p_cfg->modules = NULL;
	memset(&p_cfg->SupportedRules,0,sizeof(onvif_SupportedRules));
	onvif_VideoAnalyticsConfiguration *AnalyCfg = &p_cfg->VideoAnalyticsConfiguration;
	
	strncpy(AnalyCfg->token,vz_AnalyticsCfg->token,sizeof(AnalyCfg->token));
	strncpy(AnalyCfg->Name,vz_AnalyticsCfg->Name,sizeof(AnalyCfg->Name));
	AnalyCfg->UseCount = vz_AnalyticsCfg->UseCount;
	int i =0;
	for(i =0;i<MAX_ANALY_MODULE_NUM;i++)
	{
		if(strlen(vz_AnalyticsCfg->AnalyticsEngineCfg.AnalyticsModule[i].Name))
		{
			ONVIF_CONFIG * AModule = AnalyCfg->AnalyticsEngineConfiguration.AnalyticsModule;
			ONVIF_CONFIG * p_AModule_temp = (ONVIF_CONFIG *)get_one_item(ONVIF_TYPE_CONFIG); //malloc(sizeof(ONVIF_CONFIG));
			if (NULL == p_AModule_temp)
			{
				return ;
			}
			memset(p_AModule_temp,0,sizeof(ONVIF_CONFIG));
			p_AModule_temp->next = NULL;
			vz_onvif_Config *vz_AModule=&vz_AnalyticsCfg->AnalyticsEngineCfg.AnalyticsModule[i];
			strncpy(p_AModule_temp->Config.Name,vz_AModule->Name,sizeof(p_AModule_temp->Config.Name));
			strncpy(p_AModule_temp->Config.Type,vz_AModule->Type,sizeof(p_AModule_temp->Config.Type));
			p_AModule_temp->Config.Parameters.ElementItem = NULL;
			p_AModule_temp->Config.Parameters.SimpleItem = NULL;
			if (NULL == AModule)
			{
				AnalyCfg->AnalyticsEngineConfiguration.AnalyticsModule = p_AModule_temp;
			}
			else
			{
				while (AModule && AModule->next) AModule = AModule->next;
			
				AModule->next = p_AModule_temp;
			}
		}
	}

	for(i =0;i<MAX_ANALY_RULE_NUM;i++)
	{
		if(strlen(vz_AnalyticsCfg->RuleEngineCfg.Rule[i].Name))
		{
			ONVIF_CONFIG * Rule = AnalyCfg->RuleEngineConfiguration.Rule;
			ONVIF_CONFIG * p_Rule_temp = (ONVIF_CONFIG *)get_one_item(ONVIF_TYPE_CONFIG);// malloc(sizeof(ONVIF_CONFIG));
			if (NULL == p_Rule_temp)
			{
				return ;
			}
			memset(p_Rule_temp,0,sizeof(ONVIF_CONFIG));
			p_Rule_temp->next = NULL;
			vz_onvif_Config *vz_Rule=&vz_AnalyticsCfg->RuleEngineCfg.Rule[i];
			strncpy(p_Rule_temp->Config.Name,vz_Rule->Name,sizeof(p_Rule_temp->Config.Name));
			strncpy(p_Rule_temp->Config.Type,vz_Rule->Type,sizeof(p_Rule_temp->Config.Type));
			p_Rule_temp->Config.Parameters.ElementItem = NULL;
			p_Rule_temp->Config.Parameters.SimpleItem = NULL;
			if (NULL == Rule)
			{
				AnalyCfg->RuleEngineConfiguration.Rule = p_Rule_temp;
			}
			else
			{
				while (Rule && Rule->next) Rule = Rule->next;
			
				Rule->next = p_Rule_temp;
			}
		}
	}

}

static void iGetOneOSD(ONVIF_OSD * p_OSDcfg,VZ_ONVIF_OSD * vz_OSD)
{
	onvif_OSDConfiguration *OSDConfig = &p_OSDcfg->OSD;

	OSDConfig->TextStringFlag =vz_OSD->OSDCfg.TextStringFlag;
	OSDConfig->ImageFlag =vz_OSD->OSDCfg.ImageFlag;
	OSDConfig->ImageEnableFlag =vz_OSD->OSDCfg.ImageEnableFlag;
	OSDConfig->Type =vz_OSD->OSDCfg.Type;
	
	strncpy(OSDConfig->token,vz_OSD->OSDCfg.token,sizeof(OSDConfig->token));
	strncpy(OSDConfig->VideoSourceConfigurationToken,vz_OSD->OSDCfg.VideoSourceConfigurationToken,sizeof(OSDConfig->VideoSourceConfigurationToken));

	OSDConfig->Position.Type = (onvif_OSDPosType)vz_OSD->OSDCfg.Position.Type;
	OSDConfig->Position.PosFlag = vz_OSD->OSDCfg.Position.PosFlag;
	if(vz_OSD->OSDCfg.Position.PosFlag)
	{
		OSDConfig->Position.Pos.x = vz_OSD->OSDCfg.Position.Pos.x;
		OSDConfig->Position.Pos.y = vz_OSD->OSDCfg.Position.Pos.y;
	}

	if(vz_OSD->OSDCfg.TextStringFlag)
	{
		OSDConfig->TextString.DateFormatFlag = vz_OSD->OSDCfg.TextString.DateFormatFlag;
		OSDConfig->TextString.TimeFormatFlag = vz_OSD->OSDCfg.TextString.TimeFormatFlag;
		OSDConfig->TextString.FontSizeFlag = vz_OSD->OSDCfg.TextString.FontSizeFlag;
		OSDConfig->TextString.FontColorFlag = vz_OSD->OSDCfg.TextString.FontColorFlag;
		OSDConfig->TextString.BackgroundColorFlag = vz_OSD->OSDCfg.TextString.BackgroundColorFlag;
		OSDConfig->TextString.PlainTextFlag = vz_OSD->OSDCfg.TextString.PlainTextFlag;
		OSDConfig->TextString.DateEnableFlag = vz_OSD->OSDCfg.TextString.DateEnableFlag;
		OSDConfig->TextString.TimeEnableFlag = vz_OSD->OSDCfg.TextString.TimeEnableFlag;
		OSDConfig->TextString.PlainEnableFlag = vz_OSD->OSDCfg.TextString.PlainEnableFlag;

		OSDConfig->TextString.Type = (onvif_OSDTextType)vz_OSD->OSDCfg.TextString.Type;
		if(vz_OSD->OSDCfg.TextString.DateFormatFlag)
		{
			strncpy(OSDConfig->TextString.DateFormat,vz_OSD->OSDCfg.TextString.DateFormat,sizeof(OSDConfig->TextString.DateFormat));
		}
		
		if(OSDConfig->TextString.TimeFormatFlag)
		{
			strncpy(OSDConfig->TextString.TimeFormat,vz_OSD->OSDCfg.TextString.TimeFormat,sizeof(OSDConfig->TextString.TimeFormat));
		}
		
		if(OSDConfig->TextString.PlainTextFlag)
		{
			strncpy(OSDConfig->TextString.PlainText,vz_OSD->OSDCfg.TextString.PlainText,sizeof(OSDConfig->TextString.PlainText));
		}

		if(OSDConfig->TextString.FontSizeFlag)
			OSDConfig->TextString.FontSize = vz_OSD->OSDCfg.TextString.FontSize;
		vz_onvif_OSDTextConfiguration	*vz_Text = &vz_OSD->OSDCfg.TextString;
		if(OSDConfig->TextString.FontColorFlag)
		{
		
			OSDConfig->TextString.FontColor.ColorspaceFlag = vz_Text->FontColor.ColorspaceFlag;
			OSDConfig->TextString.FontColor.TransparentFlag = vz_Text->FontColor.TransparentFlag;
			OSDConfig->TextString.FontColor.X = vz_Text->FontColor.X;
			OSDConfig->TextString.FontColor.Y = vz_Text->FontColor.Y;
			OSDConfig->TextString.FontColor.Z = vz_Text->FontColor.Z;
			if(vz_Text->FontColor.ColorspaceFlag)
				strcpy(OSDConfig->TextString.FontColor.Colorspace,vz_Text->FontColor.Colorspace);
			if(vz_Text->FontColor.TransparentFlag)
				OSDConfig->TextString.FontColor.Transparent=vz_Text->FontColor.Transparent;
		}
		if(OSDConfig->TextString.BackgroundColorFlag)
		{
		
			OSDConfig->TextString.BackgroundColor.ColorspaceFlag = vz_Text->BackgroundColor.ColorspaceFlag;
			OSDConfig->TextString.BackgroundColor.TransparentFlag = vz_Text->BackgroundColor.TransparentFlag;
			OSDConfig->TextString.BackgroundColor.X = vz_Text->BackgroundColor.X;
			OSDConfig->TextString.BackgroundColor.Y = vz_Text->BackgroundColor.Y;
			OSDConfig->TextString.BackgroundColor.Z = vz_Text->BackgroundColor.Z;
			if(vz_Text->BackgroundColor.ColorspaceFlag)
				strcpy(OSDConfig->TextString.BackgroundColor.Colorspace,vz_Text->BackgroundColor.Colorspace);
			if(vz_Text->BackgroundColor.TransparentFlag)
				OSDConfig->TextString.BackgroundColor.Transparent=vz_Text->BackgroundColor.Transparent;
		}
		
	}
	
	if(OSDConfig->ImageFlag)
	{
		strcpy(OSDConfig->Image.ImgPath,vz_OSD->OSDCfg.Image.ImgPath);
	}

}

//use one vz onvif PTZNode to set the PTZ_NODE
static void iGetOneNode(PTZ_NODE * p_temp_node,vz_onvif_PTZNode *vz_PTZNode)
{ 
	onvif_PTZNode *PNodeInfo = &p_temp_node->PTZNode;
	
	strncpy(PNodeInfo->token,vz_PTZNode->token,sizeof(PNodeInfo->token));
	PNodeInfo->MaximumNumberOfPresets = vz_PTZNode->MaximumNumberOfPresets; 		
	PNodeInfo->HomeSupported = vz_PTZNode->HomeSupported;
	if(vz_PTZNode->NameFlag)
	{
		PNodeInfo->NameFlag = 1;
		strncpy(PNodeInfo->Name,vz_PTZNode->Name,sizeof(PNodeInfo->Name));
	}
	else
	{
		PNodeInfo->NameFlag = 0;
	}

	if(vz_PTZNode->FixedHomePositionFlag)
	{
		PNodeInfo->FixedHomePositionFlag = 1;
		PNodeInfo->FixedHomePosition = vz_PTZNode->FixedHomePosition;
	}
	else
	{
		PNodeInfo->FixedHomePositionFlag = 0;
	}

	if(vz_PTZNode->ExtensionFlag)
	{
		PNodeInfo->ExtensionFlag = 1;
		PNodeInfo->Extension.SupportedPresetTourFlag = vz_PTZNode->Extension.SupportedPresetTourFlag;
		if(PNodeInfo->Extension.SupportedPresetTourFlag)
		{
			onvif_PTZPresetTourSupported *PresetTour = &PNodeInfo->Extension.SupportedPresetTour;
			PresetTour->PTZPresetTourOperation_Start = vz_PTZNode->Extension.SupportedPresetTour.PTZPresetTourOperation_Start;
			PresetTour->PTZPresetTourOperation_Stop = vz_PTZNode->Extension.SupportedPresetTour.PTZPresetTourOperation_Stop;
			PresetTour->PTZPresetTourOperation_Pause = vz_PTZNode->Extension.SupportedPresetTour.PTZPresetTourOperation_Pause;
			PresetTour->PTZPresetTourOperation_Extended = vz_PTZNode->Extension.SupportedPresetTour.PTZPresetTourOperation_Extended;
			PresetTour->MaximumNumberOfPresetTours = vz_PTZNode->Extension.SupportedPresetTour.MaximumNumberOfPresetTours;
		}
	}
	else
	{
		PNodeInfo->ExtensionFlag = 0;
	}


	onvif_PTZSpaces *Spaces = &PNodeInfo->SupportedPTZSpaces;
	vz_onvif_PTZSpaces *vz_Spaces = &vz_PTZNode->SupportedPTZSpaces;
	
	if(vz_Spaces->AbsolutePanTiltPositionSpaceFlag)
	{
		Spaces->AbsolutePanTiltPositionSpaceFlag = 1;
		Spaces->AbsolutePanTiltPositionSpace.XRange.Min = vz_Spaces->AbsolutePanTiltPositionSpace.XRange.Min; 
		Spaces->AbsolutePanTiltPositionSpace.XRange.Max = vz_Spaces->AbsolutePanTiltPositionSpace.XRange.Max;
		Spaces->AbsolutePanTiltPositionSpace.YRange.Min = vz_Spaces->AbsolutePanTiltPositionSpace.YRange.Min;
		Spaces->AbsolutePanTiltPositionSpace.YRange.Max = vz_Spaces->AbsolutePanTiltPositionSpace.YRange.Max;
	}
	else
	{
		Spaces->AbsolutePanTiltPositionSpaceFlag = 0;
	}
	
	if(vz_Spaces->AbsoluteZoomPositionSpaceFlag)
	{
		Spaces->AbsoluteZoomPositionSpaceFlag = 1;
		Spaces->AbsoluteZoomPositionSpace.XRange.Min = vz_Spaces->AbsoluteZoomPositionSpace.XRange.Min; 
		Spaces->AbsoluteZoomPositionSpace.XRange.Max = vz_Spaces->AbsoluteZoomPositionSpace.XRange.Max;
	}
	else
	{
		Spaces->AbsoluteZoomPositionSpaceFlag = 0;
	}

	if(vz_Spaces->RelativePanTiltTranslationSpaceFlag)
	{
		Spaces->RelativePanTiltTranslationSpaceFlag = 1;
		Spaces->RelativePanTiltTranslationSpace.XRange.Min = vz_Spaces->RelativePanTiltTranslationSpace.XRange.Min; 
		Spaces->RelativePanTiltTranslationSpace.XRange.Max = vz_Spaces->RelativePanTiltTranslationSpace.XRange.Max;
		Spaces->RelativePanTiltTranslationSpace.YRange.Min = vz_Spaces->RelativePanTiltTranslationSpace.YRange.Min;
		Spaces->RelativePanTiltTranslationSpace.YRange.Max = vz_Spaces->RelativePanTiltTranslationSpace.YRange.Max;
	}
	else
	{
		Spaces->RelativePanTiltTranslationSpaceFlag = 0;
	}

	if(vz_Spaces->RelativeZoomTranslationSpaceFlag)
	{
		Spaces->RelativeZoomTranslationSpaceFlag = 1;
		Spaces->RelativeZoomTranslationSpace.XRange.Min = vz_Spaces->RelativeZoomTranslationSpace.XRange.Min; 
		Spaces->RelativeZoomTranslationSpace.XRange.Max = vz_Spaces->RelativeZoomTranslationSpace.XRange.Max;
	}
	else
	{
		Spaces->RelativeZoomTranslationSpaceFlag = 0;
	}

	if(vz_Spaces->ContinuousPanTiltVelocitySpaceFlag)
	{
		Spaces->ContinuousPanTiltVelocitySpaceFlag = 1;
		Spaces->ContinuousPanTiltVelocitySpace.XRange.Min = vz_Spaces->ContinuousPanTiltVelocitySpace.XRange.Min; 
		Spaces->ContinuousPanTiltVelocitySpace.XRange.Max = vz_Spaces->ContinuousPanTiltVelocitySpace.XRange.Max;
		Spaces->ContinuousPanTiltVelocitySpace.YRange.Min = vz_Spaces->ContinuousPanTiltVelocitySpace.YRange.Min;
		Spaces->ContinuousPanTiltVelocitySpace.YRange.Max = vz_Spaces->ContinuousPanTiltVelocitySpace.YRange.Max;
	}
	else
	{
		Spaces->ContinuousPanTiltVelocitySpaceFlag = 0;
	}

	if(vz_Spaces->ContinuousZoomVelocitySpaceFlag)
	{
		Spaces->ContinuousZoomVelocitySpaceFlag = 1;
		Spaces->ContinuousZoomVelocitySpace.XRange.Min = vz_Spaces->ContinuousZoomVelocitySpace.XRange.Min; 
		Spaces->ContinuousZoomVelocitySpace.XRange.Max = vz_Spaces->ContinuousZoomVelocitySpace.XRange.Max;
	}
	else
	{
		Spaces->ContinuousZoomVelocitySpaceFlag = 0;
	}

	if(vz_Spaces->PanTiltSpeedSpaceFlag)
	{
		Spaces->PanTiltSpeedSpaceFlag = 1;
		Spaces->PanTiltSpeedSpace.XRange.Min = vz_Spaces->PanTiltSpeedSpace.XRange.Min; 
		Spaces->PanTiltSpeedSpace.XRange.Max = vz_Spaces->PanTiltSpeedSpace.XRange.Max;
	}
	else
	{
		Spaces->PanTiltSpeedSpaceFlag = 0;
	}
	
	if(vz_Spaces->ZoomSpeedSpaceFlag)
	{
		Spaces->ZoomSpeedSpaceFlag = 1;
		Spaces->ZoomSpeedSpace.XRange.Min = vz_Spaces->ZoomSpeedSpace.XRange.Min; 
		Spaces->ZoomSpeedSpace.XRange.Max = vz_Spaces->ZoomSpeedSpace.XRange.Max;
	}
	else
	{
		Spaces->ZoomSpeedSpaceFlag = 0;
	}
}



//use one vz onvif PTZcfg to set the PTZ_CFG
static void iGetOnePTZCfg(PTZ_CFG * p_PTZcfg,vz_onvif_PTZConfiguration *vz_PTZCfg)
{ 
	onvif_PTZConfiguration *PCFGInfo = &p_PTZcfg->PTZConfiguration;
	
	strncpy(PCFGInfo->token,vz_PTZCfg->token,sizeof(PCFGInfo->token));
	strncpy(PCFGInfo->Name,vz_PTZCfg->Name,sizeof(PCFGInfo->Name));
	strncpy(PCFGInfo->NodeToken,vz_PTZCfg->NodeToken,sizeof(PCFGInfo->NodeToken));
	PCFGInfo->DefaultPTZSpeedFlag = vz_PTZCfg->DefaultPTZSpeedFlag;
	PCFGInfo->DefaultPTZTimeoutFlag = vz_PTZCfg->DefaultPTZTimeoutFlag;
	PCFGInfo->PanTiltLimitsFlag = vz_PTZCfg->PanTiltLimitsFlag;
	PCFGInfo->ZoomLimitsFlag = vz_PTZCfg->ZoomLimitsFlag;
	PCFGInfo->ExtensionFlag = vz_PTZCfg->ExtensionFlag;
	PCFGInfo->UseCount = vz_PTZCfg->UseCount;
		
	if (vz_PTZCfg->DefaultPTZSpeedFlag)
	{
		PCFGInfo->DefaultPTZSpeed.PanTiltFlag = vz_PTZCfg->DefaultPTZSpeed.PanTiltFlag;
		PCFGInfo->DefaultPTZSpeed.ZoomFlag = vz_PTZCfg->DefaultPTZSpeed.ZoomFlag;
		if (vz_PTZCfg->DefaultPTZSpeed.PanTiltFlag)
		{
			PCFGInfo->DefaultPTZSpeed.PanTilt.x = vz_PTZCfg->DefaultPTZSpeed.PanTilt.x;
			PCFGInfo->DefaultPTZSpeed.PanTilt.y = vz_PTZCfg->DefaultPTZSpeed.PanTilt.y;
		}

		if (vz_PTZCfg->DefaultPTZSpeed.ZoomFlag)
		{
			PCFGInfo->DefaultPTZSpeed.Zoom.x = vz_PTZCfg->DefaultPTZSpeed.Zoom.x;
		}
	}

	if (vz_PTZCfg->DefaultPTZTimeoutFlag)
	{
		PCFGInfo->DefaultPTZTimeout = vz_PTZCfg->DefaultPTZTimeout;
	}

	if (vz_PTZCfg->PanTiltLimitsFlag)
	{
		PCFGInfo->PanTiltLimits.XRange.Min = vz_PTZCfg->PanTiltLimits.XRange.Min;
		PCFGInfo->PanTiltLimits.XRange.Max = vz_PTZCfg->PanTiltLimits.XRange.Max;
		PCFGInfo->PanTiltLimits.YRange.Min = vz_PTZCfg->PanTiltLimits.YRange.Min;
		PCFGInfo->PanTiltLimits.YRange.Max = vz_PTZCfg->PanTiltLimits.YRange.Max;
	}

	if (vz_PTZCfg->ZoomLimitsFlag)
	{
		PCFGInfo->ZoomLimits.XRange.Min = vz_PTZCfg->ZoomLimits.XRange.Min;
		PCFGInfo->ZoomLimits.XRange.Max = vz_PTZCfg->ZoomLimits.XRange.Max;
	}

	if (vz_PTZCfg->ExtensionFlag)
	{
		PCFGInfo->Extension.PTControlDirectionFlag = 1;
		PCFGInfo->Extension.PTControlDirection.EFlipFlag = 1;
		PCFGInfo->Extension.PTControlDirection.ReverseFlag = 1;
		PCFGInfo->Extension.PTControlDirection.EFlip = vz_PTZCfg->Extension.EFlip;
		PCFGInfo->Extension.PTControlDirection.Reverse = vz_PTZCfg->Extension.Reverse;
	}
}

//use one vz onvif Profile to set the Profile
static int iGetOneProfile(ONVIF_PROFILE * pProfile,vz_onvif_Profile *vz_Profile)
{
	int i=0;
	Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();
	strncpy(pProfile->token,vz_Profile->token,sizeof(pProfile->token));
	strncpy(pProfile->name,vz_Profile->Name,sizeof(pProfile->name));
	pProfile->a_src_cfg = NULL;
	pProfile->a_enc_cfg = NULL;
	pProfile->metadata_cfg = NULL;
	if(strlen(vz_Profile->VideoEncoderConfig.token)){
		pProfile->v_enc_cfg = (ONVIF_V_ENC_CFG *)get_one_item(ONVIF_TYPE_V_ENC);// malloc(sizeof(ONVIF_V_ENC_CFG));
		if(pProfile->v_enc_cfg == NULL)
		{
			return -1;
		}
		memset(pProfile->v_enc_cfg,0,sizeof(ONVIF_V_ENC_CFG));
		pProfile->v_enc_cfg->next = NULL;
		onvif_VideoEncoderConfiguration	*Pv_enc_cfg = &pProfile->v_enc_cfg->VideoEncoderConfiguration;
		strncpy(Pv_enc_cfg->Name,vz_Profile->VideoEncoderConfig.Name,sizeof(Pv_enc_cfg->Name));
		strncpy(Pv_enc_cfg->token,vz_Profile->VideoEncoderConfig.token,sizeof(Pv_enc_cfg->token));
		Pv_enc_cfg->UseCount=1;
		Pv_enc_cfg->Encoding=vz_Profile->VideoEncoderConfig.Encoding;
		Pv_enc_cfg->Resolution.Height=vz_Profile->VideoEncoderConfig.Resolution.Height;
		Pv_enc_cfg->Resolution.Width=vz_Profile->VideoEncoderConfig.Resolution.Width;
		Pv_enc_cfg->Quality=vz_Profile->VideoEncoderConfig.Quality;
		Pv_enc_cfg->RateControlFlag = 1;
		Pv_enc_cfg->RateControl.BitrateLimit=vz_Profile->VideoEncoderConfig.RateControl.BitrateLimit;
		Pv_enc_cfg->RateControl.FrameRateLimit=vz_Profile->VideoEncoderConfig.RateControl.FrameRateLimit;
		Pv_enc_cfg->RateControl.EncodingInterval=vz_Profile->VideoEncoderConfig.RateControl.EncodingInterval;
		if(Pv_enc_cfg->Encoding == VideoEncoding_MPEG4)
		{
			Pv_enc_cfg->MPEG4Flag = 1;
			Pv_enc_cfg->MPEG4.GovLength = 30;
			Pv_enc_cfg->MPEG4.Mpeg4Profile= Mpeg4Profile_SP;
		}
		else
			Pv_enc_cfg->MPEG4Flag = 0;

		if(Pv_enc_cfg->Encoding == VideoEncoding_H264)
		{
			Pv_enc_cfg->H264Flag = 1;
			Pv_enc_cfg->H264.GovLength = vz_Profile->VideoEncoderConfig.H264.GovLength;
			Pv_enc_cfg->H264.H264Profile = vz_Profile->VideoEncoderConfig.H264.H264Profile;
		}
		else
			Pv_enc_cfg->H264Flag = 0;

		strcpy(Pv_enc_cfg->Multicast.IPv4Address, "239.0.1.0");
		Pv_enc_cfg->Multicast.Port = 32002;
		Pv_enc_cfg->Multicast.TTL = 2;
		Pv_enc_cfg->Multicast.AutoStart = FALSE;
		Pv_enc_cfg->SessionTimeout= 10;
	}else{
		pProfile->v_enc_cfg=NULL;
	}
	
	if(strlen(vz_Profile->VideoSourceConfig.token)){
		pProfile->v_src_cfg = (ONVIF_V_SRC_CFG *)get_one_item(ONVIF_TYPE_V_SRC); //malloc(sizeof(ONVIF_V_SRC_CFG));
		if(pProfile->v_src_cfg == NULL)
		{
			return -1;
		}
		memset(pProfile->v_src_cfg,0,sizeof(ONVIF_V_SRC_CFG));
		pProfile->v_src_cfg->next = NULL;
		onvif_VideoSourceConfiguration	*Pv_src_cfg = &pProfile->v_src_cfg->VideoSourceConfiguration;
		
		strncpy(Pv_src_cfg->Name,vz_Profile->VideoSourceConfig.Name,sizeof(Pv_src_cfg->Name));
		strncpy(Pv_src_cfg->token,vz_Profile->VideoSourceConfig.token,sizeof(Pv_src_cfg->token));
		Pv_src_cfg->UseCount=vz_Profile->VideoSourceConfig.UserCount;
		strncpy(Pv_src_cfg->SourceToken,vz_Profile->VideoSourceConfig.token,sizeof(Pv_src_cfg->SourceToken));
		
		Pv_src_cfg->Bounds.x=vz_Profile->VideoSourceConfig.Bounds.x;
		Pv_src_cfg->Bounds.y=vz_Profile->VideoSourceConfig.Bounds.y;
		Pv_src_cfg->Bounds.width=vz_Profile->VideoSourceConfig.Bounds.width;
		Pv_src_cfg->Bounds.height=vz_Profile->VideoSourceConfig.Bounds.height;
	}else{
		pProfile->v_src_cfg = NULL;
	}

	if(vz_Profile->VA_ID >= 0){
		pProfile->va_cfg = (ONVIF_VACFG *)get_one_item(ONVIF_TYPE_VACFG);// malloc(sizeof(ONVIF_VACFG));
		if(pProfile->va_cfg == NULL)
		{
			return -1;
		}
		memset(pProfile->va_cfg,0,sizeof(ONVIF_VACFG));
		pProfile->va_cfg->next = NULL;
		pProfile->va_cfg->rules = NULL;
		pProfile->va_cfg->modules = NULL;
		pProfile->va_cfg->SupportedRules.sizeRuleContentSchemaLocation = 0;
		pProfile->va_cfg->SupportedRules.RuleDescription = NULL;
		vz_onvif_VideoAnalyticsCfg *vz_VACfg=&pOnvifInfo->VideoAnalytics[vz_Profile->VA_ID].VACfg;
		onvif_VideoAnalyticsConfiguration *Pva_cfg = &pProfile->va_cfg->VideoAnalyticsConfiguration;
		strncpy(Pva_cfg->Name,vz_VACfg->Name,sizeof(Pva_cfg->Name));
		strncpy(Pva_cfg->token,vz_VACfg->token,sizeof(Pva_cfg->token));
		Pva_cfg->UseCount=vz_VACfg->UseCount;
		Pva_cfg->AnalyticsEngineConfiguration.AnalyticsModule=NULL;
		Pva_cfg->RuleEngineConfiguration.Rule=NULL;
	}else{
		pProfile->va_cfg = NULL;
	}

	if(vz_Profile->PTZCfg_id > 0){
		pProfile->ptz_cfg = (PTZ_CFG *)get_one_item(ONVIF_TYPE_PTZ);// malloc(sizeof(PTZ_CFG));
		if(pProfile->ptz_cfg == NULL)
		{
			return -1;
		}
		memset(pProfile->ptz_cfg,0,sizeof(PTZ_CFG));
		pProfile->ptz_cfg->next = NULL;
		vz_onvif_PTZConfiguration *vz_ptz_cfg = &pOnvifInfo->ptz_cfg[vz_Profile->PTZCfg_id].PTZConfiguration;
		iGetOnePTZCfg(pProfile->ptz_cfg,vz_ptz_cfg);
	}else{
		pProfile->ptz_cfg = NULL;
	}
	
	pProfile->fixed=vz_Profile->fixed;
	pProfile->multicasting = FALSE;

	return 0;
}

//use one vz onvif VideoEncoderCfg to set the VideoEncoderCfg
static int iGetOneVideoEncoderCfg(ONVIF_V_ENC_CFG * p_cfg,vz_onvif_VideoEncoderConfig *vz_cfg)
{
	onvif_VideoEncoderConfiguration *Pv_enc_cfg = &p_cfg->VideoEncoderConfiguration;
	strncpy(Pv_enc_cfg->Name,vz_cfg->Name,sizeof(Pv_enc_cfg->Name));
	strncpy(Pv_enc_cfg->token,vz_cfg->token,sizeof(Pv_enc_cfg->token));
	Pv_enc_cfg->UseCount=1;
	Pv_enc_cfg->Encoding=vz_cfg->Encoding;
	Pv_enc_cfg->Resolution.Height=vz_cfg->Resolution.Height;
	Pv_enc_cfg->Resolution.Width=vz_cfg->Resolution.Width;
	Pv_enc_cfg->Quality=vz_cfg->Quality;
	Pv_enc_cfg->RateControlFlag = 1;
	Pv_enc_cfg->RateControl.BitrateLimit=vz_cfg->RateControl.BitrateLimit;
	Pv_enc_cfg->RateControl.FrameRateLimit=vz_cfg->RateControl.FrameRateLimit;
	Pv_enc_cfg->RateControl.EncodingInterval=vz_cfg->RateControl.EncodingInterval;
	if(Pv_enc_cfg->Encoding == VideoEncoding_MPEG4)
	{
		Pv_enc_cfg->MPEG4Flag = 1;
		Pv_enc_cfg->MPEG4.GovLength = 30;
		Pv_enc_cfg->MPEG4.Mpeg4Profile= Mpeg4Profile_SP;
	}
	else
		Pv_enc_cfg->MPEG4Flag = 0;

	if(Pv_enc_cfg->Encoding == VideoEncoding_H264)
	{
		Pv_enc_cfg->H264Flag = 1;
		Pv_enc_cfg->H264.GovLength = vz_cfg->H264.GovLength;
		Pv_enc_cfg->H264.H264Profile = vz_cfg->H264.H264Profile;
	}
	else
		Pv_enc_cfg->H264Flag = 0;
	strcpy(Pv_enc_cfg->Multicast.IPv4Address, "239.0.1.0");
	Pv_enc_cfg->Multicast.Port = 32002;
	Pv_enc_cfg->Multicast.TTL = 2;
	Pv_enc_cfg->Multicast.AutoStart = FALSE;
	Pv_enc_cfg->SessionTimeout= 10;

	return 0;
}

//use one vz onvif VideoSourceCfg to set the VideoSourceCfg

static int iGetOneVideoSourceCfg(ONVIF_V_SRC_CFG * p_cfg,vz_onvif_VideoSourceConfig *vz_cfg)
{
	onvif_VideoSourceConfiguration	*Pv_src_cfg = &p_cfg->VideoSourceConfiguration;
	strncpy(Pv_src_cfg->Name,vz_cfg->Name,sizeof(Pv_src_cfg->Name));
	strncpy(Pv_src_cfg->token,vz_cfg->token,sizeof(Pv_src_cfg->token));
	Pv_src_cfg->UseCount=vz_cfg->UserCount;
	strncpy(Pv_src_cfg->SourceToken,vz_cfg->token,sizeof(Pv_src_cfg->SourceToken));
	
	Pv_src_cfg->Bounds.x=vz_cfg->Bounds.x;
	Pv_src_cfg->Bounds.y=vz_cfg->Bounds.y;
	Pv_src_cfg->Bounds.width=vz_cfg->Bounds.width;
	Pv_src_cfg->Bounds.height=vz_cfg->Bounds.height;
	
	return 0;
}


ONVIF_RET my_onvif_SetSystemDateAndTime(SetSystemDateAndTime_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK_SystemDateAndTime SystemDateAndTime;
	memset(&SystemDateAndTime,0x0,sizeof(LocalSDK_SystemDateAndTime));
	SystemDateAndTime.DateTimeType = p_req->SystemDateTime.DateTimeType;
	SystemDateAndTime.DaylightSavings = p_req->SystemDateTime.DaylightSavings;
	if(p_req->UTCDateTimeFlag)
	{
		SystemDateAndTime.UTCDateTime.Time.Hour = p_req->UTCDateTime.Time.Hour;
		SystemDateAndTime.UTCDateTime.Time.Minute = p_req->UTCDateTime.Time.Minute;
		SystemDateAndTime.UTCDateTime.Time.Second = p_req->UTCDateTime.Time.Second;
	
		SystemDateAndTime.UTCDateTime.Date.Year = p_req->UTCDateTime.Date.Year;
		SystemDateAndTime.UTCDateTime.Date.Month = p_req->UTCDateTime.Date.Month;
		SystemDateAndTime.UTCDateTime.Date.Day = p_req->UTCDateTime.Date.Day;
	}
	
	if(p_req->SystemDateTime.TimeZoneFlag && 
	p_req->SystemDateTime.TimeZone.TZ[0] != '\0')
	{
		strcpy(SystemDateAndTime.TimeZone,p_req->SystemDateTime.TimeZone.TZ);
	}
	
	SdkResult = LocalSDK_SetSystemDateAndTime(&SystemDateAndTime);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;

}

ONVIF_RET my_onvif_SystemReboot()
{
	// reboot system ...
	LocalSDK__SystemReboot();
	return ONVIF_OK;
}

ONVIF_RET my_onvif_SetSystemFactoryDefault(int type /* 0:soft, 1:hard */)
{
	// set system factory default
	__u8 value;
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	
	value = (type== 1)?0:1;
	SdkResult = LocalSDK__SetSystemFactoryDefault(value);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_SetHostname(const char * name)
{
    // set hostname ...
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	SdkResult = LocalSDK__SetHostname((char *)name);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

    return ONVIF_OK;
}

ONVIF_RET my_onvif_SetDNS(SetDNS_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int i = 0;
	int index = 0;
	char *SearchDomain[8];
	LocalSDK_DNSInfo DNSInfo;
	memset(&DNSInfo,0x0,sizeof(LocalSDK_DNSInfo));
	DNSInfo.FromDHCP =p_req->DNSInformation.FromDHCP;
	DNSInfo.sizeSearchDomain =0;
	if(p_req->DNSInformation.SearchDomainFlag)
	{
		for(i = 0;i < MAX_SEARCHDOMAIN ;i++)
		{
			if(strlen(p_req->DNSInformation.SearchDomain[i]))
			{
				SearchDomain[index++] =(char *)p_req->DNSInformation.SearchDomain[i];
			}
		}
		DNSInfo.sizeSearchDomain =index;
	}
	DNSInfo.SearchDomain =SearchDomain;
	
	index = 0;
	for(i = 0;i < MAX_DNS_SERVER ;i++)
	{
		strcpy(DNSInfo.DNSManual[i].IPv6Address,"");
		if(strlen(p_req->DNSInformation.DNSServer[i]))
		{
			DNSInfo.DNSManual[index].Type = Vz__IPType__IPv4;
			strcpy(DNSInfo.DNSManual[index++].IPv4Address,p_req->DNSInformation.DNSServer[i]);
		}
		else
		{
			strcpy(DNSInfo.DNSManual[index].IPv4Address,"");
		}
	}
	DNSInfo.sizeDNSManual =index;

	SdkResult = LocalSDK__SetDNS(&DNSInfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_SetNTP(SetNTP_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int i = 0;
	int index = 0;
	LocalSDK_SetNTP SetNTP;
	memset(&SetNTP,0x0,sizeof(LocalSDK_SetNTP));
	SetNTP.FromDHCP =p_req->NTPInformation.FromDHCP;
	SetNTP.sizeNTPManual=0;
	for(i = 0;i < MAX_NTP_SERVER ;i++)
	{
		if(strlen(p_req->NTPInformation.NTPServer[i]))
		{
			
			if (is_ip_address(p_req->NTPInformation.NTPServer[i]))
			{
				strcpy(SetNTP.NTPManual[index].IPv4Address,p_req->NTPInformation.NTPServer[i]);
				strcpy(SetNTP.NTPManual[index].DNSname,"");
				SetNTP.NTPManual[index].Type = Vz__NetworkHostType__IPv4;
			}
			else
			{
				strcpy(SetNTP.NTPManual[index].DNSname,p_req->NTPInformation.NTPServer[i]);
				strcpy(SetNTP.NTPManual[index].IPv4Address,"");
				SetNTP.NTPManual[index].Type = Vz__NetworkHostType__DNS;
			}
			strcpy(SetNTP.NTPManual[index].IPv6Address,"");
			index ++;
			SetNTP.sizeNTPManual ++;
		}
	}

	SdkResult = LocalSDK__SetNTP(&SetNTP);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	return ONVIF_OK;
}


ONVIF_RET my_onvif_SetNetworkProtocols(SetNetworkProtocols_REQ * p_req)
{
	int i = 0;
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__NetworkProtocolList NetworkProtocolList;
	memset(&NetworkProtocolList,0x0,sizeof(LocalSDK__NetworkProtocolList));
	if (p_req->NetworkProtocol.HTTPFlag)
	{
		NetworkProtocolList.NetworkProtocols[i].Enabled = p_req->NetworkProtocol.HTTPEnabled;
		NetworkProtocolList.NetworkProtocols[i].sizePort = 1;//MAX_SERVER_PORT;
		NetworkProtocolList.NetworkProtocols[i].Port =  p_req->NetworkProtocol.HTTPPort[0];
		NetworkProtocolList.NetworkProtocols[i].Name = NETWORK_PROTOCOL_TYPE_HTTP;
		i++;
	}

	if (p_req->NetworkProtocol.HTTPSFlag)
	{
		NetworkProtocolList.NetworkProtocols[i].Enabled = p_req->NetworkProtocol.HTTPSEnabled;
		NetworkProtocolList.NetworkProtocols[i].sizePort = 1;//MAX_SERVER_PORT;
		NetworkProtocolList.NetworkProtocols[i].Port =  p_req->NetworkProtocol.HTTPSPort[0];
		NetworkProtocolList.NetworkProtocols[i].Name = NETWORK_PROTOCOL_TYPE_HTTPS;
		i++;
	}

	if (p_req->NetworkProtocol.RTSPFlag)
	{
		NetworkProtocolList.NetworkProtocols[i].Enabled = p_req->NetworkProtocol.RTSPEnabled;
		NetworkProtocolList.NetworkProtocols[i].sizePort = 1;//MAX_SERVER_PORT;
		NetworkProtocolList.NetworkProtocols[i].Port =  p_req->NetworkProtocol.RTSPPort[0];
		NetworkProtocolList.NetworkProtocols[i].Name = NETWORK_PROTOCOL_TYPE_RTSP;
		i++;
	}
	NetworkProtocolList.sizeNetworkProtocols = i;

	for(i=0;i<NetworkProtocolList.sizeNetworkProtocols;i++)
	{
		NetworkProtocolList.NetworkProtocols[i].Extension.__size = 0;
		NetworkProtocolList.NetworkProtocols[i].Extension.__any = NULL;
		NetworkProtocolList.NetworkProtocols[i].anyAttribute = NULL;
	}
	SdkResult = LocalSDK__SetNetworkProtocols(&NetworkProtocolList);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_SetNetworkDefaultGateway(SetNetworkDefaultGateway_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	
	int i = 0;
	int index = 0;
	LocalSDK__NetworkGateway NetworkGateway;
	memset(&NetworkGateway,0x0,sizeof(LocalSDK__NetworkGateway));
	NetworkGateway.sizeIPv6Address = 0;
	for(i=0;i<MAX_GATEWAY;i++)
	{
		if(strlen(p_req->IPv4Address[i]))
		{
			strcpy(NetworkGateway.IPv4Address[index++],p_req->IPv4Address[i]);
		}
	}
	
	NetworkGateway.sizeIPv4Address = index;
	
	SdkResult = LocalSDK__SetNetworkDefaultGateway(&NetworkGateway);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_SetNetworkInterfaces(SetNetworkInterfaces_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int i = 0;
	LocalSDK__NetworkInterface NetworkInterface;
	memset(&NetworkInterface,0x0,sizeof(LocalSDK__NetworkInterface));
	NetworkInterface.Enabled = p_req->NetworkInterface.Enabled;
	NetworkInterface.IPv4.Enabled = 1;
	NetworkInterface.token = p_req->NetworkInterface.token;
	if(strlen(p_req->NetworkInterface.token))
	strcpy(NetworkInterface.Name,p_req->NetworkInterface.token);
	if(p_req->NetworkInterface.InfoFlag)
	{
		if(p_req->NetworkInterface.Info.NameFlag)
		{
			strcpy(NetworkInterface.Name,p_req->NetworkInterface.Info.Name);
		}
		if(p_req->NetworkInterface.Info.MTUFlag)
		{
			NetworkInterface.MTU = p_req->NetworkInterface.Info.MTU;
		}
		strcpy(NetworkInterface.HwAddress,p_req->NetworkInterface.Info.HwAddress);
	}
	if(p_req->NetworkInterface.IPv4Flag)
	{
		NetworkInterface.IPv4.Enabled = p_req->NetworkInterface.IPv4.Enabled;
		NetworkInterface.IPv4.Config.DHCP = p_req->NetworkInterface.IPv4.Config.DHCP;
		strcpy(NetworkInterface.IPv4.Config.Manual[0].Address,(char *)p_req->NetworkInterface.IPv4.Config.Address);
		NetworkInterface.IPv4.Config.Manual[0].PrefixLength = p_req->NetworkInterface.IPv4.Config.PrefixLength;
		NetworkInterface.IPv4.Config.sizeManual = 1;

	}
	
	SdkResult = LocalSDK__SetNetworkInterfaces(&NetworkInterface);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
    
	return ONVIF_OK;
}

ONVIF_RET my_onvif_SetDiscoveryMode(SetDiscoveryMode_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	vz_onvif_DiscoveryMode Mode;
	memset(&Mode,0x0,sizeof(vz_onvif_DiscoveryMode));
	Mode = p_req->DiscoveryMode;
	SdkResult = LocalSDK__SetDiscoveryMode(&Mode);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	return ONVIF_OK;
}

ONVIF_RET my_onvif_StartFirmwareUpgrade(StartFirmwareUpgrade_RES * p_res)
{
	//不支持
	return ONVIF_OK;
}

/***
  * do some check before the upgrade
  *
  * buff : pointer the upload content
  * len  : the upload content length
  **/
ONVIF_RET my_onvif_FirmwareUpgradeCheck(const char * buff, int len)
{
	//不支持
	return ONVIF_OK;
}

/***
  * begin firmware upgrade
  *
  * buff : pointer the upload content
  * len  : the upload content length
  **/
ONVIF_RET my_onvif_FirmwareUpgrade(const char * buff, int len)
{
	//不支持
	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetSystemDateAndTime(GetSystemDateAndTime_RES* p_res)
{
	LocalSDK_SystemDateAndTime SystemDateAndTime;
	memset(&SystemDateAndTime,0x0,sizeof(LocalSDK_SystemDateAndTime));
	LocalSDK__GetSystemDateAndTime(&SystemDateAndTime);

	p_res->SystemDateTime.DaylightSavings = SystemDateAndTime.DaylightSavings;
	p_res->SystemDateTime.DateTimeType = SystemDateAndTime.DateTimeType;
	if(strlen(SystemDateAndTime.TimeZone))
	{
		strcpy(p_res->SystemDateTime.TimeZone.TZ,SystemDateAndTime.TimeZone);
		p_res->SystemDateTime.TimeZoneFlag = 1;
	}
	else
	{
		p_res->SystemDateTime.TimeZoneFlag = 0;
	} 
	
	p_res->UTCDateTime.Date.Year = SystemDateAndTime.UTCDateTime.Date.Year;
	p_res->UTCDateTime.Date.Month = SystemDateAndTime.UTCDateTime.Date.Month;
	p_res->UTCDateTime.Date.Day = SystemDateAndTime.UTCDateTime.Date.Day;
	p_res->UTCDateTime.Time.Hour = SystemDateAndTime.UTCDateTime.Time.Hour;
	p_res->UTCDateTime.Time.Minute = SystemDateAndTime.UTCDateTime.Time.Minute;
	p_res->UTCDateTime.Time.Second = SystemDateAndTime.UTCDateTime.Time.Second;
		

	p_res->LocalDateTime.Date.Year = SystemDateAndTime.LocalDateTime.Date.Year;
	p_res->LocalDateTime.Date.Month = SystemDateAndTime.LocalDateTime.Date.Month;
	p_res->LocalDateTime.Date.Day = SystemDateAndTime.LocalDateTime.Date.Day;
	p_res->LocalDateTime.Time.Hour = SystemDateAndTime.LocalDateTime.Time.Hour;
	p_res->LocalDateTime.Time.Minute = SystemDateAndTime.LocalDateTime.Time.Minute;
	p_res->LocalDateTime.Time.Second = SystemDateAndTime.LocalDateTime.Time.Second;

	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetHostname(GetHostname_RES * p_res)
{
	Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();
	p_res->HostnameInfo.FromDHCP = pOnvifInfo->hostname.FromDHCP;
	strncpy(p_res->HostnameInfo.Name, pOnvifInfo->hostname.name,sizeof(p_res->HostnameInfo.Name));
	p_res->HostnameInfo.NameFlag = 1;

	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetDNS(GetDNS_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int i = 0;
	int index = 0;
	LocalSDK_DNSInfo DNSInfo;
	memset(&DNSInfo,0x0,sizeof(LocalSDK_DNSInfo));
	SdkResult = LocalSDK__GetDNS(&DNSInfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	p_res->DNSInfo.SearchDomainFlag = 0;
	p_res->DNSInfo.FromDHCP = DNSInfo.FromDHCP;
	if(DNSInfo.FromDHCP)
	{
		for(i=0;i<DNSInfo.sizeDNSFromDHCP && index<MAX_DNS_SERVER;i++)
		{
			switch(DNSInfo.DNSFromDHCP[i].Type)
			{
			case Vz__IPType__IPv4:
				strncpy(p_res->DNSInfo.DNSServer[index++],DNSInfo.DNSFromDHCP[i].IPv4Address,sizeof(p_res->DNSInfo.DNSServer[0]));
				break;
			case Vz__IPType__IPv6:
				strncpy(p_res->DNSInfo.DNSServer[index++],DNSInfo.DNSFromDHCP[i].IPv6Address,sizeof(p_res->DNSInfo.DNSServer[0]));
				break;
			default :
				break;
			}
		}
	}
	else
	{
		for(i=0;i<DNSInfo.sizeDNSManual && index<MAX_DNS_SERVER;i++)
		{
			switch(DNSInfo.DNSManual[i].Type)
			{
			case Vz__IPType__IPv4:
				strncpy(p_res->DNSInfo.DNSServer[index++],DNSInfo.DNSManual[i].IPv4Address,sizeof(p_res->DNSInfo.DNSServer[0]));
				break;
			case Vz__IPType__IPv6:
				strncpy(p_res->DNSInfo.DNSServer[index++],DNSInfo.DNSManual[i].IPv6Address,sizeof(p_res->DNSInfo.DNSServer[0]));
				break;
			default :
				break;
			}
		}
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetNTP(GetNTP_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int i = 0;
	int index = 0;
	LocalSDK_NTPInfo GetNTP;
	memset(&GetNTP,0x0,sizeof(LocalSDK_NTPInfo));
	SdkResult = LocalSDK__GetNTP(&GetNTP);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	p_res->NTPInfo.FromDHCP = GetNTP.FromDHCP;

	if(GetNTP.FromDHCP && GetNTP.sizeNTPFromDHCP)
	{
		for(i=0;i<GetNTP.sizeNTPFromDHCP && index<MAX_NTP_SERVER;i++)
		{
			switch(GetNTP.NTPFromDHCP[i].Type)
			{
			case Vz__NetworkHostType__IPv4:
				strncpy(p_res->NTPInfo.NTPServer[index++],GetNTP.NTPFromDHCP[i].IPv4Address,sizeof(p_res->NTPInfo.NTPServer[0]));
				break;
			case Vz__NetworkHostType__IPv6:
				strncpy(p_res->NTPInfo.NTPServer[index++],GetNTP.NTPFromDHCP[i].IPv6Address,sizeof(p_res->NTPInfo.NTPServer[0]));
				break;
			case Vz__NetworkHostType__DNS:
				strncpy(p_res->NTPInfo.NTPServer[index++],GetNTP.NTPFromDHCP[i].DNSname,sizeof(p_res->NTPInfo.NTPServer[0]));
				break;
			default :
				break;
			}
		}
	}
	else if(GetNTP.sizeNTPManual)
	{
		for(i=0;i<GetNTP.sizeNTPManual && index<MAX_NTP_SERVER;i++)
		{
			switch(GetNTP.NTPManual[i].Type)
			{
			case Vz__NetworkHostType__IPv4:
				strncpy(p_res->NTPInfo.NTPServer[index++],GetNTP.NTPManual[i].IPv4Address,sizeof(p_res->NTPInfo.NTPServer[0]));
				break;
			case Vz__NetworkHostType__IPv6:
				strncpy(p_res->NTPInfo.NTPServer[index++],GetNTP.NTPManual[i].IPv6Address,sizeof(p_res->NTPInfo.NTPServer[0]));
				break;
			case Vz__NetworkHostType__DNS:
				strncpy(p_res->NTPInfo.NTPServer[index++],GetNTP.NTPManual[i].DNSname,sizeof(p_res->NTPInfo.NTPServer[0]));
				break;
			default :
				break;
			}
		}
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetNetworkProtocols(GetNetworkProtocols_RES * p_res)
{
	int i = 0;
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	
	LocalSDK__NetworkProtocolList NetworkProtocolList;
	memset(&NetworkProtocolList,0x0,sizeof(LocalSDK__NetworkProtocolList));
	SdkResult = LocalSDK__GetNetworkProtocols(&NetworkProtocolList);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	for(i=0;i<NetworkProtocolList.sizeNetworkProtocols;i++)
	{
		if(NetworkProtocolList.NetworkProtocols[i].Name == NETWORK_PROTOCOL_TYPE_HTTP)
		{
			p_res->NetworkProtocol.HTTPFlag = 1;
			p_res->NetworkProtocol.HTTPEnabled = NetworkProtocolList.NetworkProtocols[i].Enabled;
			p_res->NetworkProtocol.HTTPPort[0] = NetworkProtocolList.NetworkProtocols[i].Port;
		}
		else if(NetworkProtocolList.NetworkProtocols[i].Name == NETWORK_PROTOCOL_TYPE_HTTPS)
		{
			p_res->NetworkProtocol.HTTPSFlag = 1;
			p_res->NetworkProtocol.HTTPSEnabled = NetworkProtocolList.NetworkProtocols[i].Enabled;
			p_res->NetworkProtocol.HTTPSPort[0] = NetworkProtocolList.NetworkProtocols[i].Port;
		}
		else if(NetworkProtocolList.NetworkProtocols[i].Name == NETWORK_PROTOCOL_TYPE_RTSP)
		{
			p_res->NetworkProtocol.RTSPFlag = 1;
			p_res->NetworkProtocol.RTSPEnabled = NetworkProtocolList.NetworkProtocols[i].Enabled;
			p_res->NetworkProtocol.RTSPPort[0] = NetworkProtocolList.NetworkProtocols[i].Port;
		}
	}
	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetNetworkDefaultGateway(GetNetworkDefaultGateway_RES * p_res)
{
	int i = 0;
	int index = 0;
	LocalSDK__NetworkGateway NetworkGateway;
	memset(&NetworkGateway,0x0,sizeof(LocalSDK__NetworkGateway));
	LocalSDK__GetNetworkDefaultGateway(&NetworkGateway);
		
	if(NetworkGateway.sizeIPv4Address != 0)
	{
		for(i=0;i<NetworkGateway.sizeIPv4Address && index < MAX_GATEWAY;i++)
		{
			strncpy(p_res->NetworkGateway.IPv4Address[index++],NetworkGateway.IPv4Address[i],sizeof(p_res->NetworkGateway.IPv4Address[0]));
		}
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetNetworkInterfaces(GetNetworkInterfaces_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int i = 0;
	int j = 0;
	LocalSDK__NetworkInterfaceList NetworkInterfaceList;
	memset(&NetworkInterfaceList,0x0,sizeof(LocalSDK__NetworkInterfaceList));
	SdkResult = LocalSDK__GetNetworkInterfaces(&NetworkInterfaceList);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	for(i=0;i<NetworkInterfaceList.sizeNetworkInterfaces;i++)
	{
		ONVIF_NET_INF * p_net_inf = p_res->interfaces;
		ONVIF_NET_INF * p_tmp = (ONVIF_NET_INF *)get_one_item(ONVIF_TYPE_NET_INF);// malloc(sizeof(ONVIF_NET_INF));
		if (NULL == p_tmp)
		{
			return ONVIF_OK;
		}
		memset(p_tmp, 0, sizeof(ONVIF_NET_INF));

		p_tmp->NetworkInterface.Enabled = NetworkInterfaceList.NetworkInterfaces[i].Enabled;
		if(NetworkInterfaceList.NetworkInterfaces[i].token)
			strncpy(p_tmp->NetworkInterface.token,NetworkInterfaceList.NetworkInterfaces[i].token,sizeof(p_tmp->NetworkInterface.token));
		p_tmp->NetworkInterface.InfoFlag = 1;
		strncpy(p_tmp->NetworkInterface.Info.HwAddress,NetworkInterfaceList.NetworkInterfaces[i].HwAddress,sizeof(p_tmp->NetworkInterface.Info.HwAddress));
		strncpy(p_tmp->NetworkInterface.Info.Name,NetworkInterfaceList.NetworkInterfaces[i].Name,sizeof(p_tmp->NetworkInterface.Info.Name));
		p_tmp->NetworkInterface.Info.NameFlag = 1;
		if(NetworkInterfaceList.NetworkInterfaces[i].MTU)
		{
			p_tmp->NetworkInterface.Info.MTUFlag = 1;
			p_tmp->NetworkInterface.Info.MTU = NetworkInterfaceList.NetworkInterfaces[i].MTU;
		}
		else
		{
			p_tmp->NetworkInterface.Info.MTUFlag = 0;
		}
		
		p_tmp->NetworkInterface.IPv4Flag = 1;
		
		p_tmp->NetworkInterface.IPv4.Enabled = NetworkInterfaceList.NetworkInterfaces[i].IPv4.Enabled;
		p_tmp->NetworkInterface.IPv4.Config.DHCP = NetworkInterfaceList.NetworkInterfaces[i].IPv4.Config.DHCP;

		if(p_tmp->NetworkInterface.IPv4.Config.DHCP)
		{
			strncpy(p_tmp->NetworkInterface.IPv4.Config.Address,NetworkInterfaceList.NetworkInterfaces[i].IPv4.Config.PreIPv4Add[PREFIX_IPV4_ADDR_FROMDHCP].Address,sizeof(p_tmp->NetworkInterface.IPv4.Config.Address)); 
			p_tmp->NetworkInterface.IPv4.Config.PrefixLength = NetworkInterfaceList.NetworkInterfaces[i].IPv4.Config.PreIPv4Add[PREFIX_IPV4_ADDR_FROMDHCP].PrefixLength;
		}
		else
		{
			strncpy(p_tmp->NetworkInterface.IPv4.Config.Address,NetworkInterfaceList.NetworkInterfaces[i].IPv4.Config.Manual[j].Address,sizeof(p_tmp->NetworkInterface.IPv4.Config.Address)); 
			p_tmp->NetworkInterface.IPv4.Config.PrefixLength = NetworkInterfaceList.NetworkInterfaces[i].IPv4.Config.Manual[j].PrefixLength;
		}

		if (NULL == p_net_inf)
		{
			p_res->interfaces = p_tmp;
		}
		else
		{
			while (p_net_inf && p_net_inf->next) p_net_inf = p_net_inf->next;
		
			p_net_inf->next = p_tmp;
		}
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetDiscoveryMode(GetDiscoveryMode_RES * p_res)
{
#ifdef NEW_ONVIF
	Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();

	p_res->DiscoveryMode = pOnvifInfo->DiscoveryMode;
#endif	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetDeviceInformation(GetDeviceInformation_RES * p_res)
{
	Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();
	
	strncpy(p_res->DeviceInfo.Manufacturer,pOnvifInfo->device_information.Manufacturer,sizeof(p_res->DeviceInfo.Manufacturer));
	strncpy(p_res->DeviceInfo.Model,pOnvifInfo->device_information.Model,sizeof(p_res->DeviceInfo.Model));
	strncpy(p_res->DeviceInfo.FirmwareVersion,pOnvifInfo->device_information.FirmwareVersion,sizeof(p_res->DeviceInfo.FirmwareVersion));
	strncpy(p_res->DeviceInfo.SerialNumber,pOnvifInfo->device_information.SerialNumber,sizeof(p_res->DeviceInfo.SerialNumber));
	strncpy(p_res->DeviceInfo.HardwareId,pOnvifInfo->device_information.HardwareId,sizeof(p_res->DeviceInfo.HardwareId));

	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetCapabilities(GetCapabilities_RES * p_res)
{
	int AllCategoryflag=0;
	int haveCategoryAnalytics = 0;
	int haveCategoryDevice = 0; 
	int haveCategoryEvents = 0;
	int haveCategoryImaging = 0;
	int haveCategoryMedia = 0;
	int haveCategoryPTZ = 0;
	int haveCategoryRecording = 0;
	int haveCategorySearch = 0;
	int haveCategoryReplay = 0;
	//如果远程服务器发送的信息过多，删除
	if(p_res->Category == CapabilityCategory_All)
	{
		haveCategoryAnalytics = 1;
		haveCategoryDevice = 1; 		
		haveCategoryEvents = 1;
		haveCategoryImaging = 1;
		haveCategoryMedia = 1;
		haveCategoryPTZ = 1;
		haveCategoryRecording = 1;
		haveCategorySearch = 1;
		haveCategoryReplay = 1;
		AllCategoryflag =1;
	}
	else if(p_res->Category ==CapabilityCategory_Analytics)
	{
		haveCategoryAnalytics = 1;
	}
	else if(p_res->Category ==CapabilityCategory_Device)
	{
		haveCategoryDevice = 1;
	}
	else if(p_res->Category ==CapabilityCategory_Events)
	{
		haveCategoryEvents = 1;
	}
	else if(p_res->Category ==CapabilityCategory_Imaging)
	{
		haveCategoryImaging = 1;
	}
	else if(p_res->Category ==CapabilityCategory_Media)
	{
		haveCategoryMedia = 1;
	}
	else if(p_res->Category ==CapabilityCategory_PTZ)
	{
		haveCategoryPTZ = 1;
	}
	else if(p_res->Category ==CapabilityCategory_Recording)
	{
		haveCategoryRecording = 1;
	}
	else if(p_res->Category ==CapabilityCategory_Search)
	{
		haveCategorySearch = 1;
	}
	else if(p_res->Category ==CapabilityCategory_Replay)
	{
		haveCategoryReplay = 1;
	}
	

	Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();	
	if(haveCategoryDevice){
		onvif_DevicesCapabilities * p_dev = &p_res->Capabilities.device;
		
		p_dev->MaxUsers = MAX_USERS;			
		strncpy(p_dev->AuxiliaryCommands,pOnvifInfo->services.device_service_cap.Misc.AuxiliaryCommands,sizeof(p_dev->AuxiliaryCommands));
		strncpy(p_dev->XAddr,pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_DEVICE_SERVICE].XAddr,sizeof(p_dev->XAddr));
		// system capabilities	
		vz_tds__SystemCapabilities* pvz_tds__SystemCapabilities = &pOnvifInfo->services.device_service_cap.System;
	
		p_dev->DiscoveryResolve = pvz_tds__SystemCapabilities->DiscoveryResolve;
		p_dev->DiscoveryBye = pvz_tds__SystemCapabilities->DiscoveryBye;
		p_dev->RemoteDiscovery = pvz_tds__SystemCapabilities->RemoteDiscovery;
		p_dev->SystemBackup = pvz_tds__SystemCapabilities->SystemBackup;
		p_dev->SystemLogging = pvz_tds__SystemCapabilities->SystemLogging;
		p_dev->FirmwareUpgrade = pvz_tds__SystemCapabilities->FirmwareUpgrade;
		p_dev->HttpFirmwareUpgrade = pvz_tds__SystemCapabilities->HttpFirmwareUpgrade;	
		p_dev->HttpSystemBackup = pvz_tds__SystemCapabilities->HttpSystemBackup;
		p_dev->HttpSystemLogging = pvz_tds__SystemCapabilities->HttpSystemLogging;
		p_dev->HttpSupportInformation = pvz_tds__SystemCapabilities->HttpSupportInformation; 	
		
		// network capabilities
		vz_tds__NetworkCapabilities* pvz_tds__NetworkCapabilities = &pOnvifInfo->services.device_service_cap.Network;

		p_dev->IPFilter = pvz_tds__NetworkCapabilities->IPFilter;
		p_dev->ZeroConfiguration = pvz_tds__NetworkCapabilities->ZeroConfiguration;
		p_dev->IPVersion6 = pvz_tds__NetworkCapabilities->IPVersion6;
		p_dev->DynDNS = pvz_tds__NetworkCapabilities->DynDNS;
		p_dev->Dot11Configuration = pvz_tds__NetworkCapabilities->Dot11Configuration;	 
		p_dev->HostnameFromDHCP = pvz_tds__NetworkCapabilities->HostnameFromDHCP;  
		p_dev->DHCPv6 = pvz_tds__NetworkCapabilities->DHCPv6; 
		p_dev->Dot1XConfigurations = pvz_tds__NetworkCapabilities->Dot1XConfigurations; 
		p_dev->NTP = pvz_tds__NetworkCapabilities->NTP; 								

		// scurity capabilities
 		vz_tds__SecurityCapabilities* pvz_tds__SecurityCapabilities = &pOnvifInfo->services.device_service_cap.Security;
 		
		p_dev->TLS10 = pvz_tds__SecurityCapabilities->TLS1_x002e0;
		p_dev->TLS11 = pvz_tds__SecurityCapabilities->TLS1_x002e1;
		p_dev->TLS12 = pvz_tds__SecurityCapabilities->TLS1_x002e2;
		p_dev->OnboardKeyGeneration =  pvz_tds__SecurityCapabilities->OnboardKeyGeneration;
		p_dev->AccessPolicyConfig =  pvz_tds__SecurityCapabilities->AccessPolicyConfig;	
		p_dev->DefaultAccessPolicy = pvz_tds__SecurityCapabilities->DefaultAccessPolicy;	
		p_dev->Dot1X = pvz_tds__SecurityCapabilities->Dot1X;	
		p_dev->RemoteUserHandling = pvz_tds__SecurityCapabilities->RemoteUserHandling;	
		p_dev->X509Token = pvz_tds__SecurityCapabilities->X_x002e509Token;	   
		p_dev->SAMLToken =  pvz_tds__SecurityCapabilities->SAMLToken;
		p_dev->KerberosToken =  pvz_tds__SecurityCapabilities->KerberosToken;	
		p_dev->RELToken =  pvz_tds__SecurityCapabilities->RELToken;
		p_dev->UsernameToken = pvz_tds__SecurityCapabilities->UsernameToken;
    	p_dev->HttpDigest =  pvz_tds__SecurityCapabilities->HttpDigest;

    	p_dev->SupportedEAPMethods =  (int)pvz_tds__SecurityCapabilities->SupportedEAPMethods[0];
	}
	if(haveCategoryAnalytics){
		onvif_AnalyticsCapabilities * p_analytics = &p_res->Capabilities.analytics;
		
		strncpy(p_analytics->XAddr,pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_ANALYTICS].XAddr,sizeof(p_analytics->XAddr));
		p_analytics->RuleSupport = pOnvifInfo->services.Analytic_cap.RuleSupport;
		p_analytics->AnalyticsModuleSupport = pOnvifInfo->services.Analytic_cap.AnalyticsModuleSupport;
		p_analytics->support = 1;
		p_analytics->CellBasedSceneDescriptionSupported = 0;
	}
	if(haveCategoryEvents){
		onvif_EventCapabilities	* p_events = &p_res->Capabilities.events;
		
		strncpy(p_events->XAddr,pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_EVENTS].XAddr,sizeof(p_events->XAddr));
		p_events->WSSubscriptionPolicySupport = pOnvifInfo->services.Event_cap.WSSubscriptionPolicySupport;
		p_events->WSPullPointSupport = pOnvifInfo->services.Event_cap.WSPullPointSupport;
		p_events->WSPausableSubscriptionManagerInterfaceSupport= pOnvifInfo->services.Event_cap.WSPausableSubscriptionManagerInterfaceSupport;
		p_events->PersistentNotificationStorage = 0;
		p_events->support = 1;
		p_events->MaxNotificationProducers = 10;
		p_events->MaxPullPoints = 0;
	}
	if(haveCategoryImaging){
		onvif_ImagingCapabilities * p_image = &p_res->Capabilities.image;

		p_image->ImageStabilization = 0;
		p_image->support = 1;
		strncpy(p_image->XAddr,pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_IMAGING].XAddr,sizeof(p_image->XAddr));
	}
	if(haveCategoryMedia){
		onvif_MediaCapabilities	*p_media = &p_res->Capabilities.media;

		strncpy(p_media->XAddr,pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_MEDIA].XAddr,sizeof(p_media->XAddr));
		p_media->RTPMulticast = pOnvifInfo->services.media_cap.StreamingCapabilities.RTPMulticast;
		p_media->RTP_TCP = pOnvifInfo->services.media_cap.StreamingCapabilities.RTP_USCORETCP;
		p_media->RTP_RTSP_TCP = pOnvifInfo->services.media_cap.StreamingCapabilities.RTP_USCORERTSP_USCORETCP;
		p_media->SnapshotUri = 1;
		p_media->Rotation = 0;
		p_media->VideoSourceMode = 0;
		p_media->OSD = 1;
		p_media->MaximumNumberOfProfiles = pOnvifInfo->services.media_cap.MediaProfileCapabilities.MaximumNumberOfProfiles;
		p_media->NonAggregateControl = 0;
		p_media->NoRTSPStreaming = 0;
		p_media->support = 1;
	}
	if(haveCategoryPTZ){
		onvif_PTZCapabilities *p_ptz = &p_res->Capabilities.ptz;
		p_ptz->EFlip = 1;
		p_ptz->Reverse = 1;
		p_ptz->GetCompatibleConfigurations = 0;
		p_ptz->support = 1;
		strncpy(p_ptz->XAddr,pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_PTZ].XAddr,sizeof(p_ptz->XAddr));
	}
	if(haveCategoryRecording){
		// record capabilities
		onvif_RecordingCapabilities *p_recording = &p_res->Capabilities.recording;
		strncpy(p_recording->XAddr,pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_RECORDING].XAddr,sizeof(p_recording->XAddr));
		p_recording->ReceiverSource = 0;
		p_recording->MediaProfileSource = 1;
		p_recording->DynamicRecordings = 1;
		p_recording->DynamicTracks = 1;
		p_recording->Options = 1;
		p_recording->MetadataRecording = 1;
		p_recording->JPEG = 1;
		p_recording->MPEG4 = 1;
		p_recording->H264 = 1;
		p_recording->G711 = 1;
		p_recording->G726 = 1;
		p_recording->AAC = 1;
		p_recording->support = 1;
		
		p_recording->MaxStringLength = 256;
		p_recording->MaxRate = 200;
		p_recording->MaxTotalRate = 2000;
		p_recording->MaxRecordings = 5;
		p_recording->MaxRecordingJobs = 5;
		
	}
	if(haveCategorySearch){
		// search capabilities
		onvif_SearchCapabilities *p_search = &p_res->Capabilities.search;
		p_search->MetadataSearch = 0;
		p_search->GeneralStartEvents = 0;
		p_search->support = 1;
		strncpy(p_search->XAddr,pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_SEARCH].XAddr,sizeof(p_search->XAddr));
	}
	if(haveCategoryReplay){
		// replay capabilities
		onvif_ReplayCapabilities *p_replay = &p_res->Capabilities.replay;
		p_replay->ReversePlayback = 0;
		p_replay->RTP_RTSP_TCP = 1;
		p_replay->support = 1;
		
		p_replay->SessionTimeoutRange.Min = 10;
		p_replay->SessionTimeoutRange.Max = 100;
		strncpy(p_replay->XAddr,pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_REPLAY].XAddr,sizeof(p_replay->XAddr));
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetServices(GetServices_RES * p_res)
{
	Onvif_Info* pOnvifInfo = GetOnvifInfo();
	onvif_DevicesCapabilities * p_dev = &p_res->Capabilities.device;
	onvif_AnalyticsCapabilities * p_analytics = &p_res->Capabilities.analytics;
	onvif_EventCapabilities * p_events = &p_res->Capabilities.events;
	onvif_ImagingCapabilities * p_image = &p_res->Capabilities.image;
	onvif_MediaCapabilities *p_media = &p_res->Capabilities.media;
	onvif_PTZCapabilities *p_ptz = &p_res->Capabilities.ptz;
	onvif_RecordingCapabilities *p_recording = &p_res->Capabilities.recording;
	onvif_SearchCapabilities *p_search = &p_res->Capabilities.search;
	onvif_ReplayCapabilities *p_replay = &p_res->Capabilities.replay;
	strncpy(p_dev->XAddr,pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_DEVICE_SERVICE].XAddr,sizeof(p_dev->XAddr));
	strncpy(p_analytics->XAddr,pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_ANALYTICS].XAddr,sizeof(p_analytics->XAddr));
	strncpy(p_events->XAddr,pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_EVENTS].XAddr,sizeof(p_events->XAddr));
	strncpy(p_image->XAddr,pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_IMAGING].XAddr,sizeof(p_image->XAddr));
	strncpy(p_media->XAddr,pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_MEDIA].XAddr,sizeof(p_media->XAddr));
	strncpy(p_ptz->XAddr,pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_PTZ].XAddr,sizeof(p_ptz->XAddr));
	strncpy(p_recording->XAddr,pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_RECORDING].XAddr,sizeof(p_recording->XAddr));
	strncpy(p_search->XAddr,pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_SEARCH].XAddr,sizeof(p_search->XAddr));
	strncpy(p_replay->XAddr,pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_REPLAY].XAddr,sizeof(p_replay->XAddr));
	p_analytics->support = 1;
	p_events->support = 1;
	p_image->support = 1;
	p_media->support = 1;
	p_ptz->support = 1;
	p_recording->support = 1;
	p_search->support = 1;
	p_replay->support = 1;


	if(p_res->bIncludeCapability)
	{
	//device  start
		p_dev->MaxUsers = MAX_USERS;			
		strncpy(p_dev->AuxiliaryCommands,pOnvifInfo->services.device_service_cap.Misc.AuxiliaryCommands,sizeof(p_dev->AuxiliaryCommands));

		// system capabilities	
		vz_tds__SystemCapabilities* pvz_tds__SystemCapabilities = &pOnvifInfo->services.device_service_cap.System;
	
		p_dev->DiscoveryResolve = pvz_tds__SystemCapabilities->DiscoveryResolve;
		p_dev->DiscoveryBye = pvz_tds__SystemCapabilities->DiscoveryBye;
		p_dev->RemoteDiscovery = pvz_tds__SystemCapabilities->RemoteDiscovery;
		p_dev->SystemBackup = pvz_tds__SystemCapabilities->SystemBackup;
		p_dev->SystemLogging = pvz_tds__SystemCapabilities->SystemLogging;
		p_dev->FirmwareUpgrade = pvz_tds__SystemCapabilities->FirmwareUpgrade;
		p_dev->HttpFirmwareUpgrade = pvz_tds__SystemCapabilities->HttpFirmwareUpgrade;	
		p_dev->HttpSystemBackup = pvz_tds__SystemCapabilities->HttpSystemBackup;
		p_dev->HttpSystemLogging = pvz_tds__SystemCapabilities->HttpSystemLogging;
		p_dev->HttpSupportInformation = pvz_tds__SystemCapabilities->HttpSupportInformation;	
		
		// network capabilities
		vz_tds__NetworkCapabilities* pvz_tds__NetworkCapabilities = &pOnvifInfo->services.device_service_cap.Network;

		p_dev->IPFilter = pvz_tds__NetworkCapabilities->IPFilter;
		p_dev->ZeroConfiguration = pvz_tds__NetworkCapabilities->ZeroConfiguration;
		p_dev->IPVersion6 = pvz_tds__NetworkCapabilities->IPVersion6;
		p_dev->DynDNS = pvz_tds__NetworkCapabilities->DynDNS;
		p_dev->Dot11Configuration = pvz_tds__NetworkCapabilities->Dot11Configuration;	 
		p_dev->HostnameFromDHCP = pvz_tds__NetworkCapabilities->HostnameFromDHCP;  
		p_dev->DHCPv6 = pvz_tds__NetworkCapabilities->DHCPv6; 
		p_dev->Dot1XConfigurations = pvz_tds__NetworkCapabilities->Dot1XConfigurations; 
		p_dev->NTP = pvz_tds__NetworkCapabilities->NTP; 								

		// scurity capabilities
		vz_tds__SecurityCapabilities* pvz_tds__SecurityCapabilities = &pOnvifInfo->services.device_service_cap.Security;
		
		p_dev->TLS10 = pvz_tds__SecurityCapabilities->TLS1_x002e0;
		p_dev->TLS11 = pvz_tds__SecurityCapabilities->TLS1_x002e1;
		p_dev->TLS12 = pvz_tds__SecurityCapabilities->TLS1_x002e2;
		p_dev->OnboardKeyGeneration =  pvz_tds__SecurityCapabilities->OnboardKeyGeneration;
		p_dev->AccessPolicyConfig =  pvz_tds__SecurityCapabilities->AccessPolicyConfig; 
		p_dev->DefaultAccessPolicy = pvz_tds__SecurityCapabilities->DefaultAccessPolicy;	
		p_dev->Dot1X = pvz_tds__SecurityCapabilities->Dot1X;	
		p_dev->RemoteUserHandling = pvz_tds__SecurityCapabilities->RemoteUserHandling;	
		p_dev->X509Token = pvz_tds__SecurityCapabilities->X_x002e509Token;	   
		p_dev->SAMLToken =	pvz_tds__SecurityCapabilities->SAMLToken;
		p_dev->KerberosToken =	pvz_tds__SecurityCapabilities->KerberosToken;	
		p_dev->RELToken =  pvz_tds__SecurityCapabilities->RELToken;
		p_dev->UsernameToken = pvz_tds__SecurityCapabilities->UsernameToken;
		p_dev->HttpDigest =  pvz_tds__SecurityCapabilities->HttpDigest;

		p_dev->SupportedEAPMethods =  (int)pvz_tds__SecurityCapabilities->SupportedEAPMethods[0];
		//device end 
		
		//Analytics start
		p_analytics->RuleSupport = pOnvifInfo->services.Analytic_cap.RuleSupport;
		p_analytics->AnalyticsModuleSupport = pOnvifInfo->services.Analytic_cap.AnalyticsModuleSupport;
		p_analytics->CellBasedSceneDescriptionSupported = 0;
		//Analytics end 

		//Events start
		p_events->WSSubscriptionPolicySupport = pOnvifInfo->services.Event_cap.WSSubscriptionPolicySupport;
		p_events->WSPullPointSupport = pOnvifInfo->services.Event_cap.WSPullPointSupport;
		p_events->WSPausableSubscriptionManagerInterfaceSupport= pOnvifInfo->services.Event_cap.WSPausableSubscriptionManagerInterfaceSupport;
		p_events->PersistentNotificationStorage = 0;
		p_events->MaxNotificationProducers = 10;
		p_events->MaxPullPoints = 0;
		//Events end 

		//Imaging start
		p_image->ImageStabilization = 0;
		//Imaging end 

		//Media start
		p_media->RTPMulticast = pOnvifInfo->services.media_cap.StreamingCapabilities.RTPMulticast;
		p_media->RTP_TCP = pOnvifInfo->services.media_cap.StreamingCapabilities.RTP_USCORETCP;
		p_media->RTP_RTSP_TCP = pOnvifInfo->services.media_cap.StreamingCapabilities.RTP_USCORERTSP_USCORETCP;
		p_media->SnapshotUri = 1;
		p_media->Rotation = 0;
		p_media->VideoSourceMode = 0;
		p_media->OSD = 1;
		p_media->MaximumNumberOfProfiles = pOnvifInfo->services.media_cap.MediaProfileCapabilities.MaximumNumberOfProfiles;
		p_media->NonAggregateControl = 0;
		p_media->NoRTSPStreaming = 0;
		//media end

		//PTZ start
		p_ptz->EFlip = 1;
		p_ptz->Reverse = 1;
		p_ptz->GetCompatibleConfigurations = 0;
		//PTZ end

		//record start
		p_recording->ReceiverSource = 0;
		p_recording->MediaProfileSource = 1;
		p_recording->DynamicRecordings = 1;
		p_recording->DynamicTracks = 1;
		p_recording->Options = 1;
		p_recording->MetadataRecording = 1;
		p_recording->JPEG = 1;
		p_recording->MPEG4 = 1;
		p_recording->H264 = 1;
		p_recording->G711 = 1;
		p_recording->G726 = 1;
		p_recording->AAC = 1;
		
		p_recording->MaxStringLength = 256;
		p_recording->MaxRate = 200;
		p_recording->MaxTotalRate = 2000;
		p_recording->MaxRecordings = 5;
		p_recording->MaxRecordingJobs = 5;
		//record end 
		
		// search start
		p_search->MetadataSearch = 0;
		p_search->GeneralStartEvents = 0;
		// search end

		// replay start
		p_replay->ReversePlayback = 0;
		p_replay->RTP_RTSP_TCP = 1;
		
		p_replay->SessionTimeoutRange.Min = 10;
		p_replay->SessionTimeoutRange.Max = 100;
		// replay end
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetStreamUri(GetStreamUri_REQ *p_req,GetStreamUri_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__MediaUri MediaUri;
	memset(&MediaUri,0x0,sizeof(LocalSDK__MediaUri));
	strcpy(MediaUri.profiletoken,p_req->ProfileToken);
	SdkResult=LocalSDK__GetStreamUri(&MediaUri);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	strncpy(p_res->stream_uri,MediaUri.Uri,sizeof(p_res->stream_uri));

	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetScopes(GetScopes_RES * p_res)
{
	Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();
	int i=0;
	int index = 0;
	
	for(i=0;i<pOnvifInfo->scopes.fixed_len && index < MAX_SCOPE_NUMS;i++){
		p_res->scopes[index].fixed = TRUE;
		strncpy(p_res->scopes[index++].scope,pOnvifInfo->scopes.fixed_scopes[i],sizeof(p_res->scopes[0].scope));
	}
	for(i=0;i<pOnvifInfo->scopes.add_len && index < MAX_SCOPE_NUMS;i++){
		p_res->scopes[index].fixed = FALSE;
		strncpy(p_res->scopes[index++].scope,pOnvifInfo->scopes.addtional_scopes[i],sizeof(p_res->scopes[0].scope));
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_add_scopes(ONVIF_SCOPE * p_scope, int scope_max)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int i =0;
	LocalSDK_ScopesInfo ScopesInfo;
	ScopesInfo.sizeScopes = scope_max;
	if(scope_max > VZ_LIST_MAX_NUM)
	{
		return ONVIF_OK;
	}
	for(i=0;i<ScopesInfo.sizeScopes;i++)
		ScopesInfo.Scopes[i] = p_scope[i].scope;
	SdkResult = LocalSDK__AddScopes(&ScopesInfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_set_scopes(ONVIF_SCOPE * p_scope, int scope_max)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int i = 0;
	LocalSDK_ScopesInfo ScopesInfo;
	ScopesInfo.sizeScopes = scope_max;
	if(scope_max > VZ_LIST_MAX_NUM)
	{
		return ONVIF_OK;
	}
	for(i=0;i<ScopesInfo.sizeScopes;i++)
	{
		ScopesInfo.Scopes[i] = p_scope[i].scope;
	}
	SdkResult = LocalSDK__SetScopes(&ScopesInfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_remove_scopes(ONVIF_SCOPE * p_scope, int scope_max)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int i =0;
	char **ScopeItem=NULL;
	LocalSDK_ScopesInfo ScopesInfo;
	ScopesInfo.sizeScopes = scope_max;
	if(scope_max > VZ_LIST_MAX_NUM)
	{
		return ONVIF_OK;
	}
	for(i=0;i<ScopesInfo.sizeScopes;i++)
	{
		ScopesInfo.Scopes[i] = p_scope[i].scope;
	}
	
	SdkResult = LocalSDK__RemoveScopes(&ScopesInfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetUsers(GetUsers_RES * p_res)
{
	Acount_t acounts[ACOUNT_NUM];
	if(ControlSystemData(MSG_GET_USER, acounts, sizeof(Acount_t)*ACOUNT_NUM) < 0)
		return ONVIF_ERR_SERVICE_NOT_SUPPORT;
	
	int i=0;
	for(;i<ACOUNT_NUM;i++){
		if(strlen(acounts[i].user)==0)break;
	}
	int acount_len = i;
	for(i=0;i<acount_len;i++){
		strncpy(p_res->users[i].Username,acounts[i].user,sizeof(p_res->users[i].Username));
		strcpy(p_res->users[i].Password," ");
		p_res->users[i].UserLevel = (onvif_UserLevel)acounts[i].authority;//取值范围0,1,2
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_add_users(ONVIF_USER * p_user, int user_max)
{
	int i = 0;
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK_UsersInfo UsersInfo;
	memset(&UsersInfo,0x0,sizeof(LocalSDK_UsersInfo));
	UsersInfo.sizeUser = user_max;
	if(user_max > VZ_LIST_MAX_NUM)
	{
		return ONVIF_OK;
	}
	for(i=0;i<UsersInfo.sizeUser;i++)
	{
		UsersInfo.UserInfo[i].Username = p_user[i].Username;
		UsersInfo.UserInfo[i].Password = p_user[i].Password;
		UsersInfo.UserInfo[i].UserLevel = p_user[i].UserLevel;
	}
	
	SdkResult = LocalSDK__CreateUsers(&UsersInfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}

const char * my_onvif_get_user_pass(const char * username)
{
	Acount_t acounts[ACOUNT_NUM];
	if(ControlSystemData(MSG_GET_USER, acounts, sizeof(Acount_t)*ACOUNT_NUM) < 0)
		return NULL;

	int i=0;
	for(i=0;i<ACOUNT_NUM;i++){
		if(strlen(acounts[i].user) && strcmp(acounts[i].user,(char *)username)==0)
			break;
	}
	if(i>=ACOUNT_NUM)
	{
		return NULL;
	}

	return acounts[i].password;
}



ONVIF_RET my_onvif_delete_users(ONVIF_USER * p_user, int user_max)
{
	int i = 0;
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	
	LocalSDK_UsersInfo UsersInfo;
	memset(&UsersInfo,0x0,sizeof(LocalSDK_UsersInfo));
	UsersInfo.sizeUser = user_max;
	if(user_max > VZ_LIST_MAX_NUM)
	{
		return ONVIF_OK;
	}
	for(i=0;i<UsersInfo.sizeUser;i++)
	{
		UsersInfo.UserInfo[i].Username = p_user[i].Username;
	}
	SdkResult = LocalSDK__DeleteUsers(&UsersInfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_set_users(ONVIF_USER * p_user, int user_max)
{
	int i = 0;
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK_UsersInfo UsersInfo;
	memset(&UsersInfo,0x0,sizeof(LocalSDK_UsersInfo));
	UsersInfo.sizeUser = user_max;
	if(user_max > VZ_LIST_MAX_NUM)
	{
		return ONVIF_OK;
	}
	for(i=0;i<UsersInfo.sizeUser;i++)
	{
		UsersInfo.UserInfo[i].Username = p_user[i].Username;
		UsersInfo.UserInfo[i].Password = p_user[i].Password;
		UsersInfo.UserInfo[i].UserLevel = p_user[i].UserLevel;
	}
	SdkResult = LocalSDK__SetUser(&UsersInfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}


ONVIF_RET my_onvif_CreateProfile(CreateProfile_REQ * p_req,CreateProfile_RES * p_reqs)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_DeleteProfile(const char * token)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_AddVideoSourceConfiguration(AddVideoSourceConfiguration_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__AddCFG cfginfo;
	memset(&cfginfo,0x0,sizeof(LocalSDK__AddCFG));
	strcpy(cfginfo.ProfileToken,p_req->ProfileToken);
	strcpy(cfginfo.ConfigurationToken,p_req->ConfigurationToken);
	cfginfo.type = vz_CFGType_VideoSource;
	SdkResult = LocalSDK__AddCFG2Profile(&cfginfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}


ONVIF_RET my_onvif_AddVideoEncoderConfiguration(AddVideoEncoderConfiguration_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__AddCFG cfginfo;
	memset(&cfginfo,0x0,sizeof(LocalSDK__AddCFG));
	strcpy(cfginfo.ProfileToken,p_req->ProfileToken);
	strcpy(cfginfo.ConfigurationToken,p_req->ConfigurationToken);
	cfginfo.type = vz_CFGType_VideoEncoder;
	SdkResult = LocalSDK__AddCFG2Profile(&cfginfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}


ONVIF_RET my_onvif_AddAudioSourceConfiguration(AddAudioSourceConfiguration_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__AddCFG cfginfo;
	memset(&cfginfo,0x0,sizeof(LocalSDK__AddCFG));
	strcpy(cfginfo.ProfileToken,p_req->ProfileToken);
	strcpy(cfginfo.ConfigurationToken,p_req->ConfigurationToken);
	cfginfo.type = vz_CFGType_AudioSource;
	SdkResult = LocalSDK__AddCFG2Profile(&cfginfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}


ONVIF_RET my_onvif_AddAudioEncoderConfiguration(AddAudioEncoderConfiguration_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__AddCFG cfginfo;
	memset(&cfginfo,0x0,sizeof(LocalSDK__AddCFG));
	strcpy(cfginfo.ProfileToken,p_req->ProfileToken);
	strcpy(cfginfo.ConfigurationToken,p_req->ConfigurationToken);
	cfginfo.type = vz_CFGType_AudioEncoder;
	SdkResult = LocalSDK__AddCFG2Profile(&cfginfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}


ONVIF_RET my_onvif_AddPTZConfiguration(AddPTZConfiguration_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__AddCFG cfginfo;
	memset(&cfginfo,0x0,sizeof(LocalSDK__AddCFG));
	strcpy(cfginfo.ProfileToken,p_req->ProfileToken);
	strcpy(cfginfo.ConfigurationToken,p_req->ConfigurationToken);
	cfginfo.type = vz_CFGType_PTZ;
	SdkResult = LocalSDK__AddCFG2Profile(&cfginfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}


ONVIF_RET my_onvif_RemoveVideoEncoderConfiguration(const char * token)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__DelCFG cfginfo;
	memset(&cfginfo,0x0,sizeof(LocalSDK__DelCFG));
	strcpy(cfginfo.ProfileToken,(char *)token);
	cfginfo.type = vz_CFGType_VideoEncoder;
	SdkResult = LocalSDK__DelCFGFromProfile(&cfginfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}


ONVIF_RET my_onvif_RemoveVideoSourceConfiguration(const char * token)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__DelCFG cfginfo;
	memset(&cfginfo,0x0,sizeof(LocalSDK__DelCFG));
	strcpy(cfginfo.ProfileToken,(char *)token);
	cfginfo.type = vz_CFGType_VideoSource;
	SdkResult = LocalSDK__DelCFGFromProfile(&cfginfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}


ONVIF_RET my_onvif_RemoveAudioEncoderConfiguration(const char * token)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__DelCFG cfginfo;
	memset(&cfginfo,0x0,sizeof(LocalSDK__DelCFG));
	strcpy(cfginfo.ProfileToken,(char *)token);
	cfginfo.type = vz_CFGType_AudioEncoder;
	SdkResult = LocalSDK__DelCFGFromProfile(&cfginfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}


ONVIF_RET my_onvif_RemoveAudioSourceConfiguration(const char * token)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__DelCFG cfginfo;
	memset(&cfginfo,0x0,sizeof(LocalSDK__DelCFG));
	strcpy(cfginfo.ProfileToken,(char *)token);
	cfginfo.type = vz_CFGType_AudioSource;
	SdkResult = LocalSDK__DelCFGFromProfile(&cfginfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}


ONVIF_RET my_onvif_RemovePTZConfiguration(const char * token)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__DelCFG cfginfo;
	memset(&cfginfo,0x0,sizeof(LocalSDK__DelCFG));
	strcpy(cfginfo.ProfileToken,(char *)token);
	cfginfo.type = vz_CFGType_PTZ;
	SdkResult = LocalSDK__DelCFGFromProfile(&cfginfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}


ONVIF_RET my_onvif_SetVideoEncoderConfiguration(SetVideoEncoderConfiguration_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__OneVideoEncoderConfig Config;
	memset(&Config,0x0,sizeof(LocalSDK__OneVideoEncoderConfig));
	Config.BaseConfig.RateControl.BitrateLimit=-1;
	Config.BaseConfig.RateControl.FrameRateLimit=-1;
	strcpy(Config.BaseConfig.token,p_req->Configuration.token);
	strcpy(Config.BaseConfig.Name,p_req->Configuration.Name);
	Config.BaseConfig.Quality = p_req->Configuration.Quality;
	
	Config.BaseConfig.Resolution.Height=p_req->Configuration.Resolution.Height;
	Config.BaseConfig.Resolution.Width=p_req->Configuration.Resolution.Width;
	
	if(p_req->Configuration.RateControlFlag){
		Config.BaseConfig.RateControl.FrameRateLimit=p_req->Configuration.RateControl.FrameRateLimit;
		Config.BaseConfig.RateControl.EncodingInterval =p_req->Configuration.RateControl.EncodingInterval;
		Config.BaseConfig.RateControl.BitrateLimit=p_req->Configuration.RateControl.BitrateLimit;
	}
	if(p_req->Configuration.H264Flag){
		Config.H264flag =1;
		Config.BaseConfig.H264.GovLength =p_req->Configuration.H264.GovLength;
		Config.BaseConfig.H264.H264Profile=(unsigned char)p_req->Configuration.H264.H264Profile;
	}
	Config.BaseConfig.Encoding =(unsigned char)p_req->Configuration.Encoding;
	
	SdkResult = LocalSDK__SetVideoEncoderConfiguration(&Config);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_SetVideoSourceConfiguration(SetVideoSourceConfiguration_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__GetVideoSourceConfig VideoSourceConfig;
	memset(&VideoSourceConfig,0x0,sizeof(VideoSourceConfig));
	strcpy(VideoSourceConfig.Token,p_req->Configuration.SourceToken);
	VideoSourceConfig.Config.Bounds.x=p_req->Configuration.Bounds.x;
	VideoSourceConfig.Config.Bounds.y=p_req->Configuration.Bounds.y;
	VideoSourceConfig.Config.Bounds.width=p_req->Configuration.Bounds.width;
	VideoSourceConfig.Config.Bounds.height=p_req->Configuration.Bounds.height;

	SdkResult =LocalSDK__SetVideoSource(&VideoSourceConfig);
	if(SdkResult != VZ_LOCALAPI_SUCCESS){
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_SetAudioSourceConfiguration(SetAudioSourceConfiguration_REQ * p_req)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_SetAudioEncoderConfiguration(SetAudioEncoderConfiguration_REQ * p_req)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_GetAudioSources(GetAudioSources_RES * p_res)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_GetAudioEncoderConfigurations(GetAudioEncoderCfgs_RES * p_res)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_GetCompatibleAudioEncoderConfigurations(GetCompatibleA_Enc_Cfgs_RES * p_res)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_GetAudioEncoderConfiguration(GetAudioEncoderCfg_RES * p_res)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_GetAudioSourceConfigurations(GetAudioSourceCfgs_RES * p_res)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_GetCompatibleAudioSourceConfigurations(GetCompatibleA_Src_Cfgs_RES * p_res)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_GetAudioSourceConfigurationOptions(GetAudioSourceConfigurationOptions_REQ *p_req,GetA_Src_Options_RES * p_res)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_GetAudioSourceConfiguration(GetAudioSourceCfg_RES * p_res)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_GetAudioEncoderConfigurationOptions(GetAudioEncoderConfigurationOptions_REQ *p_req,GetA_Enc_Options_RES * p_res)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

static void iGetOSDColorOptions(onvif_OSDColorOptions *Color,vz_onvif_OSDColorOptions *vz_Color)
{
	Color->ColorFlag = vz_Color->ColorFlag;
	Color->TransparentFlag = vz_Color->TransparentFlag;
	if(vz_Color->ColorFlag)
	{
		Color->Color.ColorListSize = vz_Color->Color.ColorListSize;
		Color->Color.ColorspaceRangeSize = vz_Color->Color.ColorspaceRangeSize;
		if(Color->Color.ColorListSize)
		{
			int i =0;
			for(i =0;i<Color->Color.ColorListSize;i++)
			{
				Color->Color.ColorList[i].X = vz_Color->Color.ColorList[i].X;
				Color->Color.ColorList[i].Y = vz_Color->Color.ColorList[i].Y;
				Color->Color.ColorList[i].Z = vz_Color->Color.ColorList[i].Z;
				Color->Color.ColorList[i].ColorspaceFlag = vz_Color->Color.ColorList[i].ColorspaceFlag;
				if(vz_Color->Color.ColorList[i].ColorspaceFlag)
					strncpy(Color->Color.ColorList[i].Colorspace,vz_Color->Color.ColorList[i].Colorspace,sizeof(Color->Color.ColorList[i].Colorspace));
			}
		}
		if(Color->Color.ColorspaceRangeSize)
		{
			int i =0;
			for(i =0;i<Color->Color.ColorspaceRangeSize;i++)
			{
				Color->Color.ColorspaceRange[i].X.Max = vz_Color->Color.ColorspaceRange[i].X.Max;
				Color->Color.ColorspaceRange[i].X.Min = vz_Color->Color.ColorspaceRange[i].X.Min;
				Color->Color.ColorspaceRange[i].Y.Max = vz_Color->Color.ColorspaceRange[i].Y.Max;
				Color->Color.ColorspaceRange[i].Y.Min = vz_Color->Color.ColorspaceRange[i].Y.Min;
				Color->Color.ColorspaceRange[i].Z.Max = vz_Color->Color.ColorspaceRange[i].Z.Max;
				Color->Color.ColorspaceRange[i].Z.Min = vz_Color->Color.ColorspaceRange[i].Z.Min;
				strncpy(Color->Color.ColorspaceRange[i].Colorspace,vz_Color->Color.ColorspaceRange[i].Colorspace,sizeof(Color->Color.ColorspaceRange[i].Colorspace));
			}
		}
	}
	if(vz_Color->TransparentFlag)
	{
		Color->Transparent.Min = vz_Color->Transparent.Min;
		Color->Transparent.Max = vz_Color->Transparent.Max;
	}
}

ONVIF_RET my_onvif_GetOSDOptions(GetOSDOptions_RES * p_res)
{
	Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();
	
	vz_onvif_OSDCfgOptions	*vz_Options = &pOnvifInfo->OSDCfgOptions;
	onvif_OSDConfigurationOptions *Options = &p_res->OSDConfigurationOptions;
	Options->OSDType_Text = vz_Options->OSDType_Text;
	Options->OSDType_Image = vz_Options->OSDType_Image;
	Options->OSDType_Extended = vz_Options->OSDType_Extended;
	Options->OSDPosType_UpperLeft = vz_Options->OSDPosType_UpperLeft;
	Options->OSDPosType_UpperRight = vz_Options->OSDPosType_UpperRight;
	Options->OSDPosType_LowerLeft = vz_Options->OSDPosType_LowerLeft;
	Options->OSDPosType_LowerRight = vz_Options->OSDPosType_LowerRight;
	Options->OSDPosType_Custom = vz_Options->OSDPosType_Custom;
	Options->TextOptionFlag = vz_Options->TextOptionFlag;
	Options->ImageOptionFlag = vz_Options->ImageOptionFlag;

	if(vz_Options->OSDType_Text)
	{
		Options->TextOption.OSDTextType_Plain = vz_Options->TextOption.OSDTextType_Plain;
		Options->TextOption.OSDTextType_Date = vz_Options->TextOption.OSDTextType_Date;
		Options->TextOption.OSDTextType_Time = vz_Options->TextOption.OSDTextType_Time;
		Options->TextOption.OSDTextType_DateAndTime = vz_Options->TextOption.OSDTextType_DateAndTime;
		Options->TextOption.FontSizeRangeFlag = vz_Options->TextOption.FontSizeRangeFlag;
		Options->TextOption.FontColorFlag = vz_Options->TextOption.FontColorFlag;
		Options->TextOption.BackgroundColorFlag = vz_Options->TextOption.BackgroundColorFlag;
		Options->TextOption.DateFormatSize = vz_Options->TextOption.DateFormatSize;
		Options->TextOption.TimeFormatSize = vz_Options->TextOption.TimeFormatSize;
		if(vz_Options->TextOption.DateFormatSize)
		{
			int i =0;
			for(i =0;i<vz_Options->TextOption.DateFormatSize;i++)
			{
				strncpy(Options->TextOption.DateFormat[i],vz_Options->TextOption.DateFormat[i],sizeof(Options->TextOption.DateFormat[i]));
			}
		}
		
		if(vz_Options->TextOption.TimeFormatSize)
		{
			int i =0;
			for(i =0;i<vz_Options->TextOption.TimeFormatSize;i++)
			{
				strncpy(Options->TextOption.TimeFormat[i],vz_Options->TextOption.TimeFormat[i],sizeof(Options->TextOption.TimeFormat[i]));
			}
		}
		
		if(vz_Options->TextOption.FontColorFlag)
		{
			onvif_OSDColorOptions *FontColor = &Options->TextOption.FontColor;
			vz_onvif_OSDColorOptions *vz_FontColor = &vz_Options->TextOption.FontColor;	
			iGetOSDColorOptions(FontColor,vz_FontColor);
		}

		if(vz_Options->TextOption.BackgroundColorFlag)
		{
			onvif_OSDColorOptions *BackColor = &Options->TextOption.BackgroundColor;
			vz_onvif_OSDColorOptions *vz_BackColor = &vz_Options->TextOption.BackgroundColor;	
			iGetOSDColorOptions(BackColor,vz_BackColor);
		}
		
		Options->TextOption.FontSizeRange.Max= vz_Options->TextOption.FontSizeRange.Max;
		Options->TextOption.FontSizeRange.Min= vz_Options->TextOption.FontSizeRange.Min;
	}
	if(vz_Options->OSDType_Image)
	{
		Options->ImageOption.ImagePathSize = vz_Options->ImageOption.ImagePathSize;
		int i =0;
		for(i =0;i<vz_Options->ImageOption.ImagePathSize;i++)
		{
			strncpy(Options->ImageOption.ImagePath[i],vz_Options->ImageOption.ImagePath[i],sizeof(Options->ImageOption.ImagePath[i]));
		}
	}
	
	Options->MaximumNumberOfOSDs.ImageFlag = vz_Options->MaximumNumberOfOSDs.ImageFlag;
	Options->MaximumNumberOfOSDs.PlainTextFlag = vz_Options->MaximumNumberOfOSDs.PlainTextFlag;
	Options->MaximumNumberOfOSDs.DateFlag = vz_Options->MaximumNumberOfOSDs.DateFlag;
	Options->MaximumNumberOfOSDs.TimeFlag = vz_Options->MaximumNumberOfOSDs.TimeFlag;
	Options->MaximumNumberOfOSDs.DateAndTimeFlag = vz_Options->MaximumNumberOfOSDs.DateAndTimeFlag;
	Options->MaximumNumberOfOSDs.Total = vz_Options->MaximumNumberOfOSDs.Total;
	Options->MaximumNumberOfOSDs.Image = vz_Options->MaximumNumberOfOSDs.Image;
	Options->MaximumNumberOfOSDs.PlainText = vz_Options->MaximumNumberOfOSDs.PlainText;
	Options->MaximumNumberOfOSDs.Date = vz_Options->MaximumNumberOfOSDs.Date;
	Options->MaximumNumberOfOSDs.Time = vz_Options->MaximumNumberOfOSDs.Time;
	Options->MaximumNumberOfOSDs.DateAndTime = vz_Options->MaximumNumberOfOSDs.DateAndTime;

	return ONVIF_OK;
}


ONVIF_RET my_onvif_GetMetadataConfigurations(GetMetadataConfigurations_RES * p_res)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_GetMetadataConfiguration(GetMetadataConfiguration_RES * p_res)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_GetCompatibleMetadataConfigurations(GetCompatibleM_Cfgs_RES * p_res)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_GetMetadataConfigurationOptions(GetMetadataConfigurationOptions_REQ *p_req,GetMetadataCfgOptions_RES * p_res)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}





ONVIF_RET my_onvif_GetSnapshot(char * buff, int * rlen, char * profile_token)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	SdkResult=LocalSDK__GetSnapshotData(buff,rlen,profile_token);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	//获取图片数据到buff
	return ONVIF_OK;
}

ONVIF_RET my_onvif_SetOSD(SetOSD_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	VZ_ONVIF_OSD Config;
	memset(&Config,0x0,sizeof(VZ_ONVIF_OSD));
	strcpy(Config.OSDCfg.token,(char *)p_req->OSD.token);
	strcpy(Config.OSDCfg.VideoSourceConfigurationToken,p_req->OSD.VideoSourceConfigurationToken);
	Config.OSDCfg.TextStringFlag=p_req->OSD.TextStringFlag;
	Config.OSDCfg.ImageFlag=p_req->OSD.ImageFlag;
	Config.OSDCfg.ImageEnableFlag=p_req->OSD.ImageEnableFlag;
	Config.OSDCfg.Type =p_req->OSD.Type;
	/*if(Config.Type == vz__OSDType__Extended)
	{
		Config.OSDCfg.ImageEnableFlag = p_req->OSD.ImageEnableFlag;
	}*/
	
	{
		Config.OSDCfg.Position.Type = p_req->OSD.Position.Type;
		if(p_req->OSD.Position.PosFlag)
		{
			Config.OSDCfg.Position.Pos.x=p_req->OSD.Position.Pos.x;
			Config.OSDCfg.Position.Pos.y=p_req->OSD.Position.Pos.y;
		}
	}
	if(p_req->OSD.TextStringFlag)
	{
		Config.OSDCfg.TextString.DateFormatFlag=p_req->OSD.TextString.DateFormatFlag;
		Config.OSDCfg.TextString.TimeFormatFlag=p_req->OSD.TextString.TimeFormatFlag;
		Config.OSDCfg.TextString.FontSizeFlag=p_req->OSD.TextString.FontSizeFlag;
		Config.OSDCfg.TextString.FontColorFlag=p_req->OSD.TextString.FontColorFlag;
		Config.OSDCfg.TextString.BackgroundColorFlag=p_req->OSD.TextString.BackgroundColorFlag;
		Config.OSDCfg.TextString.PlainTextFlag=p_req->OSD.TextString.PlainTextFlag;
		Config.OSDCfg.TextString.DateEnableFlag=p_req->OSD.TextString.DateEnableFlag;
		Config.OSDCfg.TextString.TimeEnableFlag=p_req->OSD.TextString.TimeEnableFlag;
		Config.OSDCfg.TextString.PlainEnableFlag =p_req->OSD.TextString.PlainEnableFlag;
		
		Config.OSDCfg.TextString.Type = p_req->OSD.TextString.Type;
		if(p_req->OSD.TextString.DateFormatFlag)
		{
			strcpy(Config.OSDCfg.TextString.DateFormat,p_req->OSD.TextString.DateFormat);
		}
		if(p_req->OSD.TextString.TimeFormatFlag)
		{
			strcpy(Config.OSDCfg.TextString.TimeFormat,p_req->OSD.TextString.TimeFormat);
		}
		if(p_req->OSD.TextString.PlainTextFlag)
		{
			strncpy(Config.OSDCfg.TextString.PlainText,p_req->OSD.TextString.PlainText,VZ_OSD_TEXT_LEN);
		}
		if(p_req->OSD.TextString.FontSizeFlag)
		{
			Config.OSDCfg.TextString.FontSize = p_req->OSD.TextString.FontSize;
		}
		if(p_req->OSD.TextString.FontColorFlag)
		{
			Config.OSDCfg.TextString.FontColor.X= p_req->OSD.TextString.FontColor.X;
			Config.OSDCfg.TextString.FontColor.Y= p_req->OSD.TextString.FontColor.Y;
			Config.OSDCfg.TextString.FontColor.Z= p_req->OSD.TextString.FontColor.Z;
			if(p_req->OSD.TextString.FontColor.ColorspaceFlag)
				strcpy(Config.OSDCfg.TextString.FontColor.Colorspace,p_req->OSD.TextString.FontColor.Colorspace);
			if(p_req->OSD.TextString.FontColor.TransparentFlag)
				Config.OSDCfg.TextString.FontColor.Transparent= p_req->OSD.TextString.FontColor.Transparent;
		}
		if(p_req->OSD.TextString.BackgroundColorFlag)
		{
			Config.OSDCfg.TextString.BackgroundColor.X= p_req->OSD.TextString.BackgroundColor.X;
			Config.OSDCfg.TextString.BackgroundColor.Y= p_req->OSD.TextString.BackgroundColor.Y;
			Config.OSDCfg.TextString.BackgroundColor.Z= p_req->OSD.TextString.BackgroundColor.Z;
			if(p_req->OSD.TextString.BackgroundColor.ColorspaceFlag)
				strcpy(Config.OSDCfg.TextString.BackgroundColor.Colorspace,p_req->OSD.TextString.BackgroundColor.Colorspace);
			if(p_req->OSD.TextString.BackgroundColor.TransparentFlag)
				Config.OSDCfg.TextString.BackgroundColor.Transparent= p_req->OSD.TextString.BackgroundColor.Transparent;
		}
	}
	
	if(p_req->OSD.ImageFlag)
	{
		strcpy(Config.OSDCfg.Image.ImgPath,p_req->OSD.Image.ImgPath);
	}

	SdkResult=LocalSDK__SetOSD_ex(&Config);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}


ONVIF_RET my_onvif_CreateOSD(CreateOSD_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	VZ_ONVIF_OSD OSD;
	memset(&OSD,0x0,sizeof(OSD));
	if (p_req->OSD.token[0] == '\0')
	{
		snprintf(OSD.OSDCfg.token, ONVIF_TOKEN_LEN, "OSD_%03d", g_onvif_cls.osd_idx);
		g_onvif_cls.osd_idx++;
		
		strcpy(p_req->OSD.token, OSD.OSDCfg.token);
	}
	else
	{
		strcpy(OSD.OSDCfg.token,p_req->OSD.token);
	}
	
	OSD.OSDCfg.TextStringFlag = p_req->OSD.TextStringFlag;
	OSD.OSDCfg.ImageFlag = p_req->OSD.ImageFlag;
	OSD.OSDCfg.ImageEnableFlag = p_req->OSD.ImageEnableFlag;
	OSD.OSDCfg.Type = p_req->OSD.Type;
	strcpy(OSD.OSDCfg.VideoSourceConfigurationToken,p_req->OSD.VideoSourceConfigurationToken);
	
	onvif_OSDPosConfiguration	*p_Position = &p_req->OSD.Position;
	vz_onvif_OSDPosConfiguration	*vz_Position = &OSD.OSDCfg.Position;
	
	vz_Position->PosFlag = p_Position->PosFlag;
	vz_Position->Type = p_Position->Type;
	if(p_Position->PosFlag)

	{
		vz_Position->Pos.x = p_Position->Pos.x;
		vz_Position->Pos.y = p_Position->Pos.y;
	}

	if(p_req->OSD.TextStringFlag)
	{
		onvif_OSDTextConfiguration	*p_Text = &p_req->OSD.TextString;
		vz_onvif_OSDTextConfiguration	*vz_Text = &OSD.OSDCfg.TextString;
		
		vz_Text->DateFormatFlag = p_Text->DateFormatFlag;
		vz_Text->TimeFormatFlag = p_Text->TimeFormatFlag;
		vz_Text->FontSizeFlag = p_Text->FontSizeFlag;
		vz_Text->FontColorFlag = p_Text->FontColorFlag;
		vz_Text->BackgroundColorFlag = p_Text->BackgroundColorFlag;
		vz_Text->PlainTextFlag = p_Text->PlainTextFlag;
		vz_Text->DateEnableFlag = p_Text->DateEnableFlag;
		vz_Text->TimeEnableFlag = p_Text->TimeEnableFlag;
		vz_Text->PlainEnableFlag = p_Text->PlainEnableFlag;
		vz_Text->Type = p_Text->Type;
		if(p_Text->FontSizeFlag)
			vz_Text->FontSize = p_Text->FontSize;
		if(p_Text->DateFormatFlag)
			strcpy(vz_Text->DateFormat,p_Text->DateFormat);
		if(p_Text->TimeFormatFlag)
			strcpy(vz_Text->TimeFormat,p_Text->TimeFormat);
		if(p_Text->PlainTextFlag)
			strcpy(vz_Text->PlainText,p_Text->PlainText);
		if(p_Text->FontColorFlag)
		{
			vz_Text->FontColor.ColorspaceFlag = p_Text->FontColor.ColorspaceFlag;
			vz_Text->FontColor.TransparentFlag = p_Text->FontColor.TransparentFlag;
			vz_Text->FontColor.X= p_Text->FontColor.X;
			vz_Text->FontColor.Y= p_Text->FontColor.Y;
			vz_Text->FontColor.Z= p_Text->FontColor.Z;
			if(vz_Text->FontColor.ColorspaceFlag)
				strcpy(vz_Text->FontColor.Colorspace,p_Text->FontColor.Colorspace);
			if(vz_Text->FontColor.TransparentFlag)
				vz_Text->FontColor.Transparent = p_Text->FontColor.Transparent;
		}
		if(p_Text->BackgroundColorFlag)
		{
			vz_Text->BackgroundColor.ColorspaceFlag = p_Text->BackgroundColor.ColorspaceFlag;
			vz_Text->BackgroundColor.TransparentFlag = p_Text->BackgroundColor.TransparentFlag;
			vz_Text->BackgroundColor.X= p_Text->BackgroundColor.X;
			vz_Text->BackgroundColor.Y= p_Text->BackgroundColor.Y;
			vz_Text->BackgroundColor.Z= p_Text->BackgroundColor.Z;
			if(vz_Text->BackgroundColor.ColorspaceFlag)
				strcpy(vz_Text->BackgroundColor.Colorspace,p_Text->BackgroundColor.Colorspace);
			if(vz_Text->BackgroundColor.TransparentFlag)
				vz_Text->BackgroundColor.Transparent = p_Text->BackgroundColor.Transparent;
		}
	}
	
	if(p_req->OSD.ImageFlag)
	{
		strcpy(OSD.OSDCfg.Image.ImgPath,p_req->OSD.Image.ImgPath);
	}

	SdkResult = LocalSDK__CreateOSD(&OSD);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_DeleteOSD(DeleteOSD_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int channel = -1;
	char Token[64]={0}; 
	strcpy(Token,p_req->OSDToken);
	SdkResult = LocalSDK__DeleteOSD(Token,&channel);
	//SdkResult = LocalSDK__DeleteOSD(p_req->OSDToken);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	return ONVIF_OK;
}

ONVIF_RET my_onvif_StartMulticastStreaming(const char * token)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	SdkResult = LocalSDK__Find_Profile((char *)token);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_StopMulticastStreaming(const char * token)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	SdkResult = LocalSDK__Find_Profile((char *)token);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_SetMetadataConfiguration(SetMetadataConfiguration_REQ * p_req)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_AddMetadataConfiguration(AddMetadataConfiguration_REQ * p_req)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_RemoveMetadataConfiguration(const char * profile_token)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_GetLocalProfiles(GetProfiles_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int ret =0;
	LocalSDK__Profiles Profiles;
	memset(&Profiles,0x0,sizeof(LocalSDK__Profiles));
	SdkResult = LocalSDK__GetLocalProfiles(&Profiles);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{	
		return LocalSDK_ErrHandle();
	}
	
	if(Profiles.Profile_num){
		int i=0;
		for(i=0;i<Profiles.Profile_num;i++)
		{
			ONVIF_PROFILE * p_profile = p_res->profiles;
			ONVIF_PROFILE * pProfile = (ONVIF_PROFILE *)get_one_item(ONVIF_TYPE_PROFILE);//(ONVIF_PROFILE *) malloc(sizeof(ONVIF_PROFILE));
			if (NULL == pProfile)
			{
				return ONVIF_OK;
			}
			memset(pProfile,0,sizeof(ONVIF_PROFILE));
			pProfile->next = NULL;
			ret = iGetOneProfile(pProfile,&Profiles.Profile[i]);
			if(ret < 0)
			{
				return ONVIF_OK;
			}

			if (NULL == p_profile)
			{
				p_res->profiles = pProfile;
			}
			else
			{
				while (p_profile && p_profile->next) p_profile = p_profile->next;
				p_profile->next = pProfile;
			}
		}
	}
	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetProfiles(GetProfiles_RES * p_res)
{
	return my_onvif_GetLocalProfiles(p_res);
}

ONVIF_RET my_onvif_GetProfile(GetProfile_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int ret =0;
	LocalSDK__Profiles Profiles;
	memset(&Profiles,0x0,sizeof(LocalSDK__Profiles));
	strcpy(Profiles.Profile[0].token,p_res->token);
	Profiles.Profile_num=1;
	SdkResult = LocalSDK__GetProfile(&Profiles);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{	
		return LocalSDK_ErrHandle();
	}
	
	ONVIF_PROFILE * pProfile = &p_res->profile;
	pProfile->next = NULL;
	ret = iGetOneProfile(pProfile,&Profiles.Profile[0]);
	if(ret < 0)
	{
		return ONVIF_OK;
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetSnapshotUri(GetSnapshotUri_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__SnapshotUri SnapshotUri;
	memset(&SnapshotUri,0x0,sizeof(SnapshotUri));
	strcpy(SnapshotUri.ProfileToken,p_res->token);
	SdkResult=LocalSDK__GetSnapshotUri(&SnapshotUri);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	strncpy(p_res->stream_uri,SnapshotUri.Uri,sizeof(p_res->stream_uri));

	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetVideoEncoderConfigurations(Get_v_enc_cfgs_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__VideoEncoderConfigs EncoderConfigs;
	memset(&EncoderConfigs,0x0,sizeof(LocalSDK__VideoEncoderConfigs));
	SdkResult = LocalSDK__GetVideoEncoderConfigurations(&EncoderConfigs);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	if(EncoderConfigs.VideoEncoder_num){
		int i=0;
		for(i=0;i<EncoderConfigs.VideoEncoder_num;i++)
		{
			ONVIF_V_ENC_CFG * p_venc_cfg = p_res->p_v_enc_cfg;
			ONVIF_V_ENC_CFG * p_cfg = (ONVIF_V_ENC_CFG *)get_one_item(ONVIF_TYPE_V_ENC);// malloc(sizeof(ONVIF_V_ENC_CFG));
			if (NULL == p_cfg)
			{
				return ONVIF_OK;
			}
			memset(p_cfg,0,sizeof(ONVIF_V_ENC_CFG));
			p_cfg->next = NULL;
			iGetOneVideoEncoderCfg(p_cfg,&EncoderConfigs.VideoEncoderConfig[i]);

			if (NULL == p_venc_cfg)
			{
				p_res->p_v_enc_cfg = p_cfg;
			}
			else
			{
				while (p_venc_cfg && p_venc_cfg->next) p_venc_cfg = p_venc_cfg->next;
				p_venc_cfg->next = p_cfg;
			}
		}

	}
	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetCompatibleVideoEncoderConfigurations(GetCompatiblev_enc_cfgs_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	SdkResult = LocalSDK__Find_Profile(p_res->token);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	Get_v_enc_cfgs_RES res;
	memset(&res,0,sizeof(res));
	int ret = my_onvif_GetVideoEncoderConfigurations(&res);
	p_res->p_v_enc_cfg = res.p_v_enc_cfg;
	return ret;
	
}


ONVIF_RET my_onvif_GetVideoEncoderConfiguration(Get_v_enc_cfg_RES * p_res)
{ 
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__OneVideoEncoderConfig VideoEncoderConfig;
	memset(&VideoEncoderConfig,0x0,sizeof(LocalSDK__OneVideoEncoderConfig));
	strcpy(VideoEncoderConfig.BaseConfig.token,p_res->ConfigurationToken);
	SdkResult = LocalSDK__GetVideoEncoderConfiguration(&VideoEncoderConfig);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	p_res->v_enc_cfg.next = NULL;
	iGetOneVideoEncoderCfg(&p_res->v_enc_cfg,&VideoEncoderConfig.BaseConfig);
	
	return ONVIF_OK; 
}

ONVIF_RET my_onvif_GetVideoSourceConfigurations(Get_v_src_cfgs_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__VideoSourceConfigs SourceConfigs;
	memset(&SourceConfigs,0x0,sizeof(LocalSDK__VideoSourceConfigs));
	SdkResult = LocalSDK__GetLocalVideoSourceConfigs(&SourceConfigs);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{	
		return LocalSDK_ErrHandle();
	}
	if(SourceConfigs.VideoSource_num){
		int i=0;
		for(i=0;i<SourceConfigs.VideoSource_num;i++){
			ONVIF_V_SRC_CFG * p_v_src_cfg = p_res->p_v_src_cfgs;
			ONVIF_V_SRC_CFG * p_cfg = (ONVIF_V_SRC_CFG *)get_one_item(ONVIF_TYPE_V_SRC);// malloc(sizeof(ONVIF_V_SRC_CFG));
			if (NULL == p_cfg)
			{
				return ONVIF_OK;
			}
			memset(p_cfg,0,sizeof(ONVIF_V_SRC_CFG));
			p_cfg->next = NULL;
			
			iGetOneVideoSourceCfg(p_cfg,&SourceConfigs.VideoSourceConfig[i]);

			if (NULL == p_v_src_cfg)
			{
				p_res->p_v_src_cfgs = p_cfg;
			}
			else
			{
				while (p_v_src_cfg && p_v_src_cfg->next) p_v_src_cfg = p_v_src_cfg->next;
				p_v_src_cfg->next = p_cfg;
			}
		}
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetCompatibleVideoSourceConfigurations(GetCompatiblev_src_cfgs_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	SdkResult = LocalSDK__Find_Profile(p_res->token);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	Get_v_src_cfgs_RES res;
	memset(&res,0,sizeof(res));
	int ret = my_onvif_GetVideoSourceConfigurations(&res);
	p_res->p_v_src_cfgs = res.p_v_src_cfgs;
	return ret;
}


ONVIF_RET my_onvif_GetVideoSourceConfiguration(Get_v_src_cfg_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__GetVideoSourceConfig VideoSourceConfig;
	memset(&VideoSourceConfig,0x0,sizeof(VideoSourceConfig));
	strcpy(VideoSourceConfig.Token,p_res->ConfigurationToken);
	SdkResult =LocalSDK__GetSpecifiedVideoSource(&VideoSourceConfig);
	if(SdkResult != VZ_LOCALAPI_SUCCESS){
		return LocalSDK_ErrHandle();
	}

	p_res->v_src_cfg.next = NULL;
	iGetOneVideoSourceCfg(&p_res->v_src_cfg,&VideoSourceConfig.Config);

	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetVideoSourceConfigurationOptions(GetVideoSourceConfigurationOptions_REQ * p_req,Get_v_src_options_RES * p_res)
{
	int i = 0;
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__VideoSourceConfigurationOption Option;
	memset(&Option,0x0,sizeof(LocalSDK__VideoSourceConfigurationOption));
	if(p_req->ConfigurationTokenFlag)
		strcpy(Option.ConfigurationToken,p_req->ConfigurationToken);
	if(p_req->ProfileTokenFlag)
		strcpy(Option.ProfileToken,p_req->ProfileToken);
	SdkResult = LocalSDK__GetVideoSourceConfigurationOptions(&Option);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	p_res->v_src_option.BoundsRange.XRange.Max = Option.XRange.Max;
	p_res->v_src_option.BoundsRange.XRange.Min= Option.XRange.Min;
	p_res->v_src_option.BoundsRange.YRange.Max = Option.YRange.Max;
	p_res->v_src_option.BoundsRange.YRange.Min= Option.YRange.Min;
	p_res->v_src_option.BoundsRange.WidthRange.Max = Option.WidthRange.Max;
	p_res->v_src_option.BoundsRange.WidthRange.Min= Option.WidthRange.Min;
	p_res->v_src_option.BoundsRange.HeightRange.Max = Option.HeightRange.Max;
	p_res->v_src_option.BoundsRange.HeightRange.Min= Option.HeightRange.Min;
	strcpy(p_res->SourceToken[i],Option.AvailableSourceToken[i++]);

	return ONVIF_OK; 
}

ONVIF_RET my_onvif_GetVideoEncoderConfigurationOptions(GetVideoEncoderConfigurationOptions_REQ * p_req,Get_v_enc_options_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__VideoEncoderConfigurationOption Option;
	memset(&Option,0x0,sizeof(LocalSDK__VideoEncoderConfigurationOption));
	if(p_req->ConfigurationTokenFlag)
		strcpy(Option.ConfigurationToken,p_req->ConfigurationToken);
	if(p_req->ProfileTokenFlag)
		strcpy(Option.ProfileToken,p_req->ProfileToken);
	SdkResult = LocalSDK__GetVideoEncoderConfigurationOptions(&Option);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	p_res->v_enc_option.QualityRange.Max=Option.QualityRange.Max;
	p_res->v_enc_option.QualityRange.Min=Option.QualityRange.Min;
	p_res->v_enc_option.MPEG4Flag = 0;
	p_res->v_enc_option.H264Flag = 1;
	onvif_H264Options * H264= &p_res->v_enc_option.H264;
	int i=0;
	for(i=0;i<Option.H264.Resolution_num;i++){
		H264->ResolutionsAvailable[i].Width=Option.H264.ResolutionsAvailable[i].Width;
		H264->ResolutionsAvailable[i].Height=Option.H264.ResolutionsAvailable[i].Height;
	}
	H264->GovLengthRange.Max=Option.H264.GovLengthRange.Max;
	H264->GovLengthRange.Min=Option.H264.GovLengthRange.Min;

	H264->EncodingIntervalRange.Max=Option.H264.EncodingIntervalRange.Max;
	H264->EncodingIntervalRange.Min=Option.H264.EncodingIntervalRange.Min;

	H264->FrameRateRange.Min=Option.H264.FrameRateRange.Min;
	H264->FrameRateRange.Max=Option.H264.FrameRateRange.Max;

	if(Option.H264.Profile == VZ__H264Profile__Baseline)
	{
		H264->H264Profile_Baseline = 1;
	}
	else if(Option.H264.Profile == VZ__H264Profile__Main)
	{
		H264->H264Profile_Main = 1;
	}
	else if(Option.H264.Profile == VZ__H264Profile__Extended)
	{
		H264->H264Profile_Extended = 1;
	}
	else if(Option.H264.Profile == VZ__H264Profile__High)
	{
		H264->H264Profile_High = 1;
	}

	if(Option.JPEG.Resolution_num){
		
		p_res->v_enc_option.JPEGFlag = 1;
		onvif_JpegOptions *JPEG = &p_res->v_enc_option.JPEG;

		for(i=0;i<Option.JPEG.Resolution_num;i++){
			JPEG->ResolutionsAvailable[i].Width=Option.JPEG.ResolutionsAvailable[i].Width;
			JPEG->ResolutionsAvailable[i].Height=Option.JPEG.ResolutionsAvailable[i].Height;
		}

		JPEG->EncodingIntervalRange.Max=Option.JPEG.EncodingIntervalRange.Max;
		JPEG->EncodingIntervalRange.Min=Option.JPEG.EncodingIntervalRange.Min;

		JPEG->FrameRateRange.Min=Option.JPEG.FrameRateRange.Min;
		JPEG->FrameRateRange.Max=Option.JPEG.FrameRateRange.Max;
	}
	else{
		p_res->v_enc_option.JPEGFlag = 0;
	}
	return ONVIF_OK; 
}

ONVIF_RET my_onvif_GetOSDs(GetOSDs_REQ * p_req,GetOSDs_RES * p_res)
{
	Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();
	int i = 0;
	for(i = 0;i<pOnvifInfo->OSDS.OSD_num;i++)
	{
		if(p_req->ConfigurationTokenFlag
			&&(strcmp(p_req->ConfigurationToken,pOnvifInfo->OSDS.OSD[i].OSDCfg.VideoSourceConfigurationToken)!=0))
		{
			continue;
		}
		ONVIF_OSD * p_OSD = p_res->OSDs;
		ONVIF_OSD * p_cfg = (ONVIF_OSD *)get_one_item(ONVIF_TYPE_OSD);// malloc(sizeof(ONVIF_OSD));
		if (NULL == p_cfg)
		{
			return ONVIF_OK;
		}
		memset(p_cfg,0,sizeof(ONVIF_OSD));
		p_cfg->next = NULL;
		VZ_ONVIF_OSD *vz_osd=&pOnvifInfo->OSDS.OSD[i];
		iGetOneOSD(p_cfg,vz_osd);

		if (NULL == p_OSD)
		{
			p_res->OSDs = p_cfg;
		}
		else
		{
			while (p_OSD && p_OSD->next) p_OSD = p_OSD->next;
		
			p_OSD->next = p_cfg;
		}
	}
	
 	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetOSD(GetOSD_REQ * p_req,GetOSD_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	VZ_ONVIF_OSD OSD;
	memset(&OSD,0x0,sizeof(VZ_ONVIF_OSD));
	SdkResult=LocalSDK__GetOSD_ex(p_req->OSDToken,&OSD);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}	

	iGetOneOSD(&p_res->struOSD,&OSD);

	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetVideoSources(GetVideoSources_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__VideoSources Sources;
	memset(&Sources,0x0,sizeof(LocalSDK__VideoSources));
	SdkResult = LocalSDK__GetVideoSources(&Sources);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	if(Sources.VideoSource_num){
		int i=0;
		for(i=0;i<Sources.VideoSource_num;i++){
			ONVIF_V_SRC * p_video_src = p_res->p_v_src;
			ONVIF_V_SRC * p_temp_v_src = (ONVIF_V_SRC *)get_one_item(ONVIF_TYPE_V_SRC);// malloc(sizeof(ONVIF_V_SRC));
			if (NULL == p_temp_v_src)
			{
				return ONVIF_OK;
			}
			memset(p_temp_v_src,0,sizeof(ONVIF_V_SRC));
			p_temp_v_src->next = NULL;
			onvif_VideoSource *PVideoSource = &p_temp_v_src->VideoSource;
			strncpy(PVideoSource->token,Sources.VideoSource[i].VideoSourceToken,sizeof(PVideoSource->token));
			PVideoSource->Framerate=Sources.VideoSource[i].Framerate;
			PVideoSource->Resolution.Height=Sources.VideoSource[i].Resolution.Height;
			PVideoSource->Resolution.Width=Sources.VideoSource[i].Resolution.Width;

			PVideoSource->Imaging.BacklightCompensationFlag=0;
			PVideoSource->Imaging.ExposureFlag=0;
			PVideoSource->Imaging.FocusFlag=0;
			PVideoSource->Imaging.IrCutFilterFlag=0;
			PVideoSource->Imaging.SharpnessFlag=0;
			PVideoSource->Imaging.WideDynamicRangeFlag=0;
			PVideoSource->Imaging.WhiteBalanceFlag=0;
			PVideoSource->Imaging.BrightnessFlag=1;
			PVideoSource->Imaging.ColorSaturationFlag=1;
			PVideoSource->Imaging.ContrastFlag=1;
			PVideoSource->Imaging.Brightness=Sources.VideoSource[i].Imaging.Brightness;
			PVideoSource->Imaging.ColorSaturation=Sources.VideoSource[i].Imaging.ColorSaturation;
			PVideoSource->Imaging.Contrast=Sources.VideoSource[i].Imaging.Contrast;

			if (NULL == p_video_src)
			{
				p_res->p_v_src = p_temp_v_src;
			}
			else
			{
				while (p_video_src && p_video_src->next) p_video_src = p_video_src->next;
			
				p_video_src->next = p_temp_v_src;
			}
		}
	}
	return ONVIF_OK;
}

ONVIF_RET my_onvif_AddVideoAnalyticsConfiguration(AddVideoAnalyticsConfiguration_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__AddCFG cfginfo;
	memset(&cfginfo,0x0,sizeof(LocalSDK__AddCFG));
	strcpy(cfginfo.ProfileToken,p_req->ProfileToken);
	strcpy(cfginfo.ConfigurationToken,p_req->ConfigurationToken);
	cfginfo.type = vz_CFGType_VideoAnalytics;
	SdkResult = LocalSDK__AddCFG2Profile(&cfginfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_RemoveVideoAnalyticsConfiguration(RemoveVideoAnalyticsConfiguration_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__DelCFG cfginfo;
	memset(&cfginfo,0x0,sizeof(LocalSDK__DelCFG));
	strcpy(cfginfo.ProfileToken,p_req->ProfileToken);
	cfginfo.type = vz_CFGType_VideoAnalytics;
	SdkResult = LocalSDK__DelCFGFromProfile(&cfginfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}
//对算法进行配置
ONVIF_RET my_onvif_SetVideoAnalyticsConfiguration(SetVideoAnalyticsConfiguration_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int i = 0;
	LocalSDK__SetVideoAnalyticsConfig Config;
	memset(&Config,0x0,sizeof(LocalSDK__SetVideoAnalyticsConfig));
	strcpy(Config.cfg.VACfg.token,p_req->Configuration.token);

	if(p_req->Configuration.AnalyticsEngineConfiguration.AnalyticsModule)
	{
		ONVIF_CONFIG * P_Module = p_req->Configuration.AnalyticsEngineConfiguration.AnalyticsModule;
		vz_onvif_AnalyticsEngineCfg *vz_AEngine = &Config.cfg.VACfg.AnalyticsEngineCfg; // required
		while(P_Module && i < MAX_ANALY_MODULE_NUM)
		{
			strcpy(vz_AEngine->AnalyticsModule[i].Name,P_Module->Config.Name);
			strcpy(vz_AEngine->AnalyticsModule[i].Type,P_Module->Config.Type);
			P_Module = P_Module->next;
			i++;
		}
	}
	i = 0;
	if(p_req->Configuration.RuleEngineConfiguration.Rule)
	{
		ONVIF_CONFIG * P_Rule = p_req->Configuration.RuleEngineConfiguration.Rule;
		vz_onvif_RuleEngineCfg	*vz_REngine = &Config.cfg.VACfg.RuleEngineCfg;		// required 
		while(P_Rule&& i < MAX_ANALY_RULE_NUM)
		{
			strcpy(vz_REngine->Rule[i].Name,P_Rule->Config.Name);
			strcpy(vz_REngine->Rule[i].Type,P_Rule->Config.Type);
			i++;
			P_Rule = P_Rule->next;
		}
	}

	Config.ForcePersistence=p_req->ForcePersistence;
	SdkResult = LocalSDK__SetVideoAnalyticsConfiguration(&Config);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetVideoAnalyticsConfigurations(Get_VA_Configurations_RES * p_res)
{
	Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();
	int i = 0;
	for(i = 0;i<MAX_VIDEO_ANALYTICS_NUM;i++)
	{
		if(pOnvifInfo->VideoAnalytics[i].used == 0)
		{
			continue;
		}
		ONVIF_VACFG * p_vas = p_res->p_va_cfg;
		ONVIF_VACFG * p_cfg = (ONVIF_VACFG *)get_one_item(ONVIF_TYPE_VACFG);// malloc(sizeof(ONVIF_VACFG));
		if (NULL == p_cfg)
		{
			return ONVIF_OK;
		}
		memset(p_cfg,0,sizeof(ONVIF_VACFG));
		p_cfg->next = NULL;
		vz_onvif_VideoAnalyticsCfg *vz_AnalyticsCfg=&pOnvifInfo->VideoAnalytics[i].VACfg;
		iGetOneVideoAnalyticsCfg(p_cfg,vz_AnalyticsCfg);

		if (NULL == p_vas)
		{
			p_res->p_va_cfg = p_cfg;
		}
		else
		{
			while (p_vas && p_vas->next) p_vas = p_vas->next;
		
			p_vas->next = p_cfg;
		}
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetVideoAnalyticsConfiguration(GetVideoAnalyticsConfiguration_REQ* p_req,Get_VA_Configuration_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int index = 0;
	
	SdkResult = LocalSDK__Find_Video_Analytics(p_req->ConfigurationToken,&index);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();
	
	iGetOneVideoAnalyticsCfg(&p_res->struva_cfg,&pOnvifInfo->VideoAnalytics[index].VACfg);
	
	return ONVIF_OK;
}



ONVIF_RET my_onvif_GetSupportedRules(GetSupportedRules_REQ * p_req, GetSupportedRules_RES * p_res)
{ 
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int i = 0;
	LocalSDK__SupportedRules SupportedRules;
	memset(&SupportedRules,0x0,sizeof(LocalSDK__SupportedRules));
	
	SdkResult = LocalSDK__GetSupportedRules(p_req->ConfigurationToken,&SupportedRules);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	if(SupportedRules.sizeSchemaLocation)
	{
		for(i=0;i<SupportedRules.sizeSchemaLocation && i < 10;i++)
		{
			strcpy(p_res->SupportedRules.RuleContentSchemaLocation[i],SupportedRules.SchemaLocation[i]);
		}
	}
	p_res->SupportedRules.sizeRuleContentSchemaLocation = SupportedRules.sizeSchemaLocation;
	
	if(SupportedRules.sizeRuleDescription)
	{
		for(i=0;i<SupportedRules.sizeRuleDescription;i++)
		{
			ONVIF_CFG_DESC * p_RuleDescription = p_res->SupportedRules.RuleDescription;
			ONVIF_CFG_DESC * p_temp = (ONVIF_CFG_DESC *)get_one_item(ONVIF_TYPE_CFG_DESC);//malloc(sizeof(ONVIF_CFG_DESC));
			if (NULL == p_temp)
			{
				return ONVIF_OK;
			}
			memset(p_temp,0,sizeof(ONVIF_CFG_DESC));
			p_temp->next = NULL;
			
			onvif_ItemListDescription *Parameters =&p_temp->ConfigDescription.Parameters;
			if((SupportedRules.RuleDescription[i].Parameters.__sizeElementItem != 0) || (SupportedRules.RuleDescription[i].Parameters.__sizeSimpleItem != 0))
			{
				int j =0;
				if(SupportedRules.RuleDescription[i].Parameters.__sizeSimpleItem)
				{
					for(j=0;j<SupportedRules.RuleDescription[i].Parameters.__sizeSimpleItem;j++)
					{
						ONVIF_SIMPLE_ITEM_DESC * p_SimpleItem = Parameters->SimpleItemDescription;
						ONVIF_SIMPLE_ITEM_DESC * p_SimpleItem_temp = (ONVIF_SIMPLE_ITEM_DESC *)get_one_item(ONVIF_TYPE_SITEM_DESC);//malloc(sizeof(ONVIF_SIMPLE_ITEM_DESC));
						if (NULL == p_SimpleItem_temp)
						{
							return ONVIF_OK;
						}
						memset(p_SimpleItem_temp,0,sizeof(ONVIF_SIMPLE_ITEM_DESC));
						p_SimpleItem_temp->next = NULL;
						onvif_SimpleItemDescription *p_SIDes = &p_SimpleItem_temp->SimpleItemDescription;
						strncpy(p_SIDes->Name,SupportedRules.RuleDescription[i].Parameters.SimpleItem[j].Name,sizeof(p_SIDes->Name));
						strncpy(p_SIDes->Type,SupportedRules.RuleDescription[i].Parameters.SimpleItem[j].Value,sizeof(p_SIDes->Type));
						if (NULL == p_SimpleItem)
						{
							Parameters->SimpleItemDescription = p_SimpleItem_temp;
						}
						else
						{
							while (p_SimpleItem && p_SimpleItem->next) p_SimpleItem = p_SimpleItem->next;
						
							p_SimpleItem->next = p_SimpleItem_temp;
						}
					}
				}
				else
				{
					Parameters->SimpleItemDescription = NULL;
				}
				
				if(SupportedRules.RuleDescription[i].Parameters.__sizeElementItem)
				{
					for(j=0;j<SupportedRules.RuleDescription[i].Parameters.__sizeElementItem;j++)
					{
						ONVIF_SIMPLE_ITEM_DESC * p_EleItem = Parameters->ElementItemDescription;
						ONVIF_SIMPLE_ITEM_DESC * p_EleItem_temp = (ONVIF_SIMPLE_ITEM_DESC *)get_one_item(ONVIF_TYPE_SITEM_DESC);//malloc(sizeof(ONVIF_SIMPLE_ITEM_DESC));
						if (NULL == p_EleItem_temp)
						{
							return ONVIF_OK;
						}
						memset(p_EleItem_temp,0,sizeof(ONVIF_SIMPLE_ITEM_DESC));
						p_EleItem_temp->next = NULL;
	
						onvif_SimpleItemDescription *p_EIDes = &p_EleItem_temp->SimpleItemDescription;
						strncpy(p_EIDes->Name,SupportedRules.RuleDescription[i].Parameters.ElementItem[j].Name,sizeof(p_EIDes->Name));
						strncpy(p_EIDes->Type,SupportedRules.RuleDescription[i].Parameters.ElementItem[j].Value,sizeof(p_EIDes->Type));
						//free(SupportedRules.RuleDescription[i].Parameters.ElementItem[j].Value);

						if (NULL == p_EleItem)
						{
							Parameters->ElementItemDescription = p_EleItem_temp;
						}
						else
						{
							while (p_EleItem && p_EleItem->next) p_EleItem = p_EleItem->next;
						
							p_EleItem->next = p_EleItem_temp;
						}
					
					}
				}
				else
				{
					Parameters->ElementItemDescription=NULL;
				}

			}
			else
			{
				Parameters->SimpleItemDescription = NULL;
				Parameters->ElementItemDescription = NULL;
			}
				
			if(SupportedRules.RuleDescription[i].__sizeMessages)
			{
				int j = 0;
				for(j=0;j<SupportedRules.RuleDescription[i].__sizeMessages;j++)
				{
					ONVIF_CFG_DESC_MSG * p_Messages = p_temp->ConfigDescription.Messages;
					ONVIF_CFG_DESC_MSG * p_Mgtemp = (ONVIF_CFG_DESC_MSG *)get_one_item(ONVIF_TYPE_CFGDESC_MSG);//malloc(sizeof(ONVIF_CFG_DESC_MSG));
					if (NULL == p_Mgtemp)
					{
						return ONVIF_OK;
					}
					memset(p_Mgtemp,0,sizeof(ONVIF_CFG_DESC_MSG));
					p_Mgtemp->next = NULL;
					
					onvif_ConfigDescription_Messages *Parameters =&p_Mgtemp->Messages;
					Parameters->SourceFlag = 0;
					Parameters->KeyFlag = 0;
					Parameters->DataFlag = 0;
					Parameters->IsPropertyFlag = 0;
					if (NULL == p_Messages)
					{
						p_temp->ConfigDescription.Messages = p_Mgtemp;
					}
					else
					{
						while (p_Messages && p_Messages->next) p_Messages = p_Messages->next;
					
						p_Messages->next = p_Mgtemp;
					}
				}
			}
			else
			{
				p_temp->ConfigDescription.Messages=NULL;
			}
			strncpy(p_temp->ConfigDescription.Name,SupportedRules.RuleDescription[i].Name,sizeof(p_temp->ConfigDescription.Name));

			if (NULL == p_RuleDescription)
			{
				p_res->SupportedRules.RuleDescription = p_temp;
			}
			else
			{
				while (p_RuleDescription && p_RuleDescription->next) p_RuleDescription = p_RuleDescription->next;
			
				p_RuleDescription->next = p_temp;
			}
		}
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_CreateRules(CreateRules_REQ * p_req)
{
	int i = 0;
	int j = 0;
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__OneSetOfRule RulesInfo;
	memset(&RulesInfo,0x0,sizeof(RulesInfo));
	RulesInfo.token = (char *)p_req->ConfigurationToken;
	ONVIF_CONFIG * p_rules = p_req->Rule;
	while(p_rules)
	{
		strcpy(RulesInfo.Rule[i].Name,p_rules->Config.Name); 
		//ignor tan__CreateRules->Rule[i]->Type
		ONVIF_SIMPLEITEM	* P_Simple = p_rules->Config.Parameters.SimpleItem;
		ONVIF_ELEMENTITEM	* P_Element = p_rules->Config.Parameters.ElementItem;
		while(P_Simple)
		{
			strcpy(RulesInfo.Rule[i].Parameters.SimpleItem[j].Name,P_Simple->SimpleItem.Name);
			strcpy(RulesInfo.Rule[i].Parameters.SimpleItem[j].Value,P_Simple->SimpleItem.Value);
			P_Simple = P_Simple->next;
			j++;
		}
		RulesInfo.Rule[i].Parameters.__sizeSimpleItem = j;
		j = 0;
		while(P_Element)
		{
			strcpy(RulesInfo.Rule[i].Parameters.ElementItem[j].Name,P_Element->ElementItem.Name);
      int len = strlen(P_Element->ElementItem.Any)+1;
      len = len > VZ_LIST_MAX_NUM ? VZ_LIST_MAX_NUM : len;
			strncpy(RulesInfo.Rule[i].Parameters.ElementItem[j].Value, P_Element->ElementItem.Any,len);
			P_Element = P_Element->next;
			j++;
		}
		RulesInfo.Rule[i].Parameters.__sizeElementItem = j;
		p_rules = p_rules->next;
		i++;
	}
	RulesInfo.sizeRule = i;
	
	SdkResult = LocalSDK__CreateRules(&RulesInfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;
}	

ONVIF_RET my_onvif_DeleteRules(DeleteRules_REQ * p_req)
{
	int i = 0;
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__OneSetOfRule RulesInfo;
	memset(&RulesInfo,0x0,sizeof(RulesInfo));
	RulesInfo.sizeRule = p_req->sizeRuleName;
	RulesInfo.token = p_req->ConfigurationToken;
	for(i=0;i<RulesInfo.sizeRule;i++)
	{
		strcpy(RulesInfo.Rule[i].Name,p_req->RuleName[i]);
	}
	SdkResult = LocalSDK__DeleteRules(&RulesInfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetRules(GetRules_REQ * p_req, GetRules_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int i =0;
	LocalSDK__OneSetOfRule OneSetOfRules;
	memset(&OneSetOfRules,0x0,sizeof(LocalSDK__OneSetOfRule));
	OneSetOfRules.token = p_req->ConfigurationToken;
	SdkResult = LocalSDK__GetRules(&OneSetOfRules);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	if(OneSetOfRules.sizeRule)
	{
		for(i=0;i<OneSetOfRules.sizeRule;i++)
		{
			ONVIF_CONFIG * p_Rules = p_res->Rule;
			ONVIF_CONFIG * p_temp = (ONVIF_CONFIG *)get_one_item(ONVIF_TYPE_CONFIG);//malloc(sizeof(ONVIF_CONFIG));
			if (NULL == p_temp)
			{
				return ONVIF_OK;
			}
			memset(p_temp,0,sizeof(ONVIF_CONFIG));
			p_temp->next = NULL;

			strncpy(p_temp->Config.Name,OneSetOfRules.Rule[i].Name,sizeof(p_temp->Config.Name));
			strncpy(p_temp->Config.Type,OneSetOfRules.Rule[i].Name,sizeof(p_temp->Config.Type));
			onvif_ItemList	*Parameters = &p_temp->Config.Parameters;
			if((OneSetOfRules.Rule[i].Parameters.__sizeElementItem != 0) || (OneSetOfRules.Rule[i].Parameters.__sizeSimpleItem != 0))
			{
				int j =0;
				if(OneSetOfRules.Rule[i].Parameters.__sizeSimpleItem)
				{
					for(j=0;j<OneSetOfRules.Rule[i].Parameters.__sizeSimpleItem;j++)
					{
						ONVIF_SIMPLEITEM * p_SimpleItem = Parameters->SimpleItem;
						ONVIF_SIMPLEITEM * p_SimpleItem_temp = (ONVIF_SIMPLEITEM *)get_one_item(ONVIF_TYPE_SIMPLEITEM);//malloc(sizeof(ONVIF_SIMPLEITEM));
						if (NULL == p_SimpleItem_temp)
						{
							return ONVIF_OK;
						}
						memset(p_SimpleItem_temp,0,sizeof(ONVIF_SIMPLEITEM));
						p_SimpleItem_temp->next = NULL;
						onvif_SimpleItem *p_SIDes = &p_SimpleItem_temp->SimpleItem;
						strncpy(p_SIDes->Name,OneSetOfRules.Rule[i].Parameters.SimpleItem[j].Name,sizeof(p_SIDes->Name));
						strncpy(p_SIDes->Value,OneSetOfRules.Rule[i].Parameters.SimpleItem[j].Value,sizeof(p_SIDes->Value));
						if (NULL == p_SimpleItem)
						{
							Parameters->SimpleItem = p_SimpleItem_temp;
						}
						else
						{
							while (p_SimpleItem && p_SimpleItem->next) p_SimpleItem = p_SimpleItem->next;
						
							p_SimpleItem->next = p_SimpleItem_temp;
						}
					}
				}
				else
				{
					Parameters->SimpleItem = NULL;
				}
				
				if(OneSetOfRules.Rule[i].Parameters.__sizeElementItem)
				{
					for(j=0;j<OneSetOfRules.Rule[i].Parameters.__sizeElementItem;j++)
					{
						ONVIF_ELEMENTITEM * p_EleItem = Parameters->ElementItem;
						ONVIF_ELEMENTITEM * p_EleItem_temp = (ONVIF_ELEMENTITEM *)get_one_item(ONVIF_TYPE_ELEMENTITEM);//malloc(sizeof(ONVIF_ELEMENTITEM));
						if (NULL == p_EleItem_temp)
						{
							return ONVIF_OK;
						}
						memset(p_EleItem_temp,0,sizeof(ONVIF_ELEMENTITEM));
						p_EleItem_temp->next = NULL;

						onvif_ElementItem *p_EIDes = &p_EleItem_temp->ElementItem;
						strncpy(p_EIDes->Name,OneSetOfRules.Rule[i].Parameters.ElementItem[j].Name,sizeof(p_EIDes->Name));
						p_EIDes->Any = get_idle_net_buf();//(char *)malloc(strlen(OneSetOfRules.Rule[i].Parameters.ElementItem[j].Value)+1);
						if (NULL == p_EIDes->Any)
						{
							return ONVIF_OK;
						}
						strcpy(p_EIDes->Any,OneSetOfRules.Rule[i].Parameters.ElementItem[j].Value);
						//free(OneSetOfRules.Rule[i].Parameters.ElementItem[j].Value);

						if (NULL == p_EleItem)
						{
							Parameters->ElementItem = p_EleItem_temp;
						}
						else
						{
							while (p_EleItem && p_EleItem->next) p_EleItem = p_EleItem->next;
						
							p_EleItem->next = p_EleItem_temp;
						}
					
					}
				}
				else
				{
					Parameters->ElementItem = NULL;
				}

			}
			else
			{
				Parameters->SimpleItem = NULL;
				Parameters->ElementItem = NULL;
			}
			
			if (NULL == p_Rules)
			{
				p_res->Rule = p_temp;
			}
			else
			{
				while (p_Rules && p_Rules->next) p_Rules = p_Rules->next;
			
				p_Rules->next = p_temp;
			}
		}
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_ModifyRules(ModifyRules_REQ * p_req)
{
	int i = 0;
	int j = 0;
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__OneSetOfRule RulesInfo;
	memset(&RulesInfo,0x0,sizeof(RulesInfo));
	RulesInfo.token = (char *)p_req->ConfigurationToken;

	ONVIF_CONFIG * p_rules = p_req->Rule;
	while(p_rules)
	{
		strcpy(RulesInfo.Rule[i].Name,p_rules->Config.Name); 
		//ignor tan__CreateRules->Rule[i]->Type
		ONVIF_SIMPLEITEM	* P_Simple = p_rules->Config.Parameters.SimpleItem;
		ONVIF_ELEMENTITEM	* P_Element = p_rules->Config.Parameters.ElementItem;
		while(P_Simple)
		{
			strcpy(RulesInfo.Rule[i].Parameters.SimpleItem[j].Name,P_Simple->SimpleItem.Name);
			strcpy(RulesInfo.Rule[i].Parameters.SimpleItem[j].Value,P_Simple->SimpleItem.Value);
			P_Simple = P_Simple->next;
			j++;
		}
		RulesInfo.Rule[i].Parameters.__sizeSimpleItem = j;
		j = 0;
		while(P_Element)
		{
			strcpy(RulesInfo.Rule[i].Parameters.ElementItem[j].Name,P_Element->ElementItem.Name);
      int len = strlen(P_Element->ElementItem.Any)+1;
      len = len > VZ_LIST_MAX_NUM ? VZ_LIST_MAX_NUM : len;
			strncpy(RulesInfo.Rule[i].Parameters.ElementItem[j].Value, P_Element->ElementItem.Any,len);
			P_Element = P_Element->next;
			j++;
		}
		RulesInfo.Rule[i].Parameters.__sizeElementItem = j;
		p_rules = p_rules->next;
		i++;
	}
	RulesInfo.sizeRule = i;
	
	SdkResult = LocalSDK__ModifyRules(&RulesInfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;
}

//set preset_num
ONVIF_RET my_onvif_CreateAnalyticsModules(CreateAnalyticsModules_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__Analytics ModifyAnalytics;
	memset(&ModifyAnalytics,0x0,sizeof(LocalSDK__Analytics));
	ModifyAnalytics.token = (char *)p_req->ConfigurationToken;
	if(p_req->AnalyticsModule)
	{
		strcpy(ModifyAnalytics.Analytics.Parameters.SimpleItem[0].Name,"preset_num");
		ModifyAnalytics.Analytics.Parameters.__sizeSimpleItem= 1;
		strcpy(ModifyAnalytics.Analytics.Parameters.SimpleItem[0].Value,(char *)p_req->AnalyticsModule->Config.Name);
	}
	SdkResult = LocalSDK__SetPresetNum(&ModifyAnalytics);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;
}

//set ivs_drawmode
ONVIF_RET my_onvif_DeleteAnalyticsModules(DeleteAnalyticsModules_REQ * p_req)
{ 
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__Analytics ModifyAnalytics;
	memset(&ModifyAnalytics,0x0,sizeof(LocalSDK__Analytics));
	ModifyAnalytics.token = (char *)p_req->ConfigurationToken;
	
	if(p_req->sizeAnalyticsModuleName)
	{
		ModifyAnalytics.Analytics.Parameters.__sizeElementItem= 1;
		strcpy(ModifyAnalytics.Analytics.Parameters.ElementItem[0].Name,"ivs_drawmode");
		strcpy(ModifyAnalytics.Analytics.Parameters.ElementItem[0].Value,(char *)p_req->AnalyticsModuleName[0]);
	}
	SdkResult = LocalSDK__SetDrawMode(&ModifyAnalytics);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;
}

//set run_mode
ONVIF_RET my_onvif_ModifyAnalyticsModules(ModifyAnalyticsModules_REQ * p_req)
{ 
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__Analytics ModifyAnalytics;
	memset(&ModifyAnalytics,0x0,sizeof(LocalSDK__Analytics));
	ModifyAnalytics.token = (char *)p_req->ConfigurationToken;
	if(p_req->AnalyticsModule)
	{
		strcpy(ModifyAnalytics.Analytics.Parameters.SimpleItem[0].Name,"run_mode");
		ModifyAnalytics.Analytics.Parameters.__sizeSimpleItem= 1;
		strcpy(ModifyAnalytics.Analytics.Parameters.SimpleItem[0].Value,p_req->AnalyticsModule->Config.Name);
	}
	SdkResult = LocalSDK__SetRunMode(&ModifyAnalytics);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;
}	

ONVIF_RET my_onvif_GetAnalyticsModules(GetAnalyticsModules_REQ * p_req, GetAnalyticsModules_RES * p_res)
{ 
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__Analytics AnalyticsModule;
	memset(&AnalyticsModule,0x0,sizeof(LocalSDK__Analytics));
	AnalyticsModule.token = p_req->ConfigurationToken;

	SdkResult = LocalSDK__GetAnalyticsModules(&AnalyticsModule);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	if((AnalyticsModule.Analytics.Parameters.__sizeElementItem != 0) || (AnalyticsModule.Analytics.Parameters.__sizeSimpleItem != 0))
	{
		ONVIF_CONFIG * p_Analytics = p_res->AnalyticsModule;
		ONVIF_CONFIG * p_cfg = (ONVIF_CONFIG *)get_one_item(ONVIF_TYPE_CONFIG);// malloc(sizeof(ONVIF_CONFIG));
		if (NULL == p_cfg)
		{
			return ONVIF_OK;
		}
		memset(p_cfg,0,sizeof(ONVIF_CONFIG));
		p_cfg->next = NULL;
		onvif_ItemList	*Parameters = &p_cfg->Config.Parameters;
		int j =0;
		if(AnalyticsModule.Analytics.Parameters.__sizeElementItem)
		{
			for(j=0;j<AnalyticsModule.Analytics.Parameters.__sizeElementItem;j++)
			{
				ONVIF_ELEMENTITEM * p_EleItem = Parameters->ElementItem;
				ONVIF_ELEMENTITEM * p_EleItem_temp = (ONVIF_ELEMENTITEM *)get_one_item(ONVIF_TYPE_ELEMENTITEM);//malloc(sizeof(ONVIF_ELEMENTITEM));
				if (NULL == p_EleItem_temp)
				{
					return ONVIF_OK;
				}
				memset(p_EleItem_temp,0,sizeof(ONVIF_ELEMENTITEM));
				p_EleItem_temp->next = NULL;
				
				onvif_ElementItem *p_EIDes = &p_EleItem_temp->ElementItem;

				strncpy(p_EIDes->Name,AnalyticsModule.Analytics.Parameters.ElementItem[j].Name,sizeof(p_EIDes->Name));
				if(AnalyticsModule.Analytics.Parameters.ElementItem[j].Value){
					p_EIDes->Any = get_idle_net_buf();//(char *)malloc(strlen(AnalyticsModule.Analytics.Parameters.ElementItem[j].Value)+1);
					if (NULL == p_EIDes->Any)
					{
						return ONVIF_OK;
					}
					strcpy(p_EIDes->Any,AnalyticsModule.Analytics.Parameters.ElementItem[j].Value);
					//free(AnalyticsModule.Analytics.Parameters.ElementItem[j].Value);
				}

				if (NULL == p_EleItem)
				{
					Parameters->ElementItem = p_EleItem_temp;
				}
				else
				{
					while (p_EleItem && p_EleItem->next) p_EleItem = p_EleItem->next;
				
					p_EleItem->next = p_EleItem_temp;
				}
					
			}
		}
		else
		{
			Parameters->ElementItem = NULL;
		}
		
		if(AnalyticsModule.Analytics.Parameters.__sizeSimpleItem)
		{
			for(j=0;j<AnalyticsModule.Analytics.Parameters.__sizeSimpleItem;j++)
			{
				ONVIF_SIMPLEITEM * p_SimpleItem = Parameters->SimpleItem;
				ONVIF_SIMPLEITEM * p_SimpleItem_temp = (ONVIF_SIMPLEITEM *)get_one_item(ONVIF_TYPE_SIMPLEITEM);//malloc(sizeof(ONVIF_SIMPLEITEM));
				if (NULL == p_SimpleItem_temp)
				{
					return ONVIF_OK;
				}
				memset(p_SimpleItem_temp,0,sizeof(ONVIF_SIMPLEITEM));
				p_SimpleItem_temp->next = NULL;
				onvif_SimpleItem *p_SIDes = &p_SimpleItem_temp->SimpleItem;
				strncpy(p_SIDes->Name,AnalyticsModule.Analytics.Parameters.SimpleItem[j].Name,sizeof(p_SIDes->Name));
				strncpy(p_SIDes->Value,AnalyticsModule.Analytics.Parameters.SimpleItem[j].Value,sizeof(p_SIDes->Value));
				if (NULL == p_SimpleItem)
				{
					Parameters->SimpleItem = p_SimpleItem_temp;
				}
				else
				{
					while (p_SimpleItem && p_SimpleItem->next) p_SimpleItem = p_SimpleItem->next;
				
					p_SimpleItem->next = p_SimpleItem_temp;
				}
			}
		}
		else
		{
			Parameters->SimpleItem =NULL;
		}
	}
	else
	{
		 p_res->AnalyticsModule =NULL;
	}

	return ONVIF_OK;
}


ONVIF_RET my_onvif_PTZGetStatus(const char	*ProfileToken, onvif_PTZStatus * p_ptz_status)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	SdkResult = LocalSDK__Find_Profile((char *)ProfileToken);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	p_ptz_status->PositionFlag = 1;
	p_ptz_status->Position.PanTiltFlag = 1;
	p_ptz_status->Position.PanTilt.x = 0;
	p_ptz_status->Position.PanTilt.y = 0;
	p_ptz_status->Position.ZoomFlag = 1;
	p_ptz_status->Position.Zoom.x = 0;
	
	p_ptz_status->MoveStatusFlag = 1;
	p_ptz_status->MoveStatus.PanTiltFlag = 1;
	p_ptz_status->MoveStatus.PanTilt = MoveStatus_IDLE;
	p_ptz_status->MoveStatus.ZoomFlag = 1;
	p_ptz_status->MoveStatus.Zoom = MoveStatus_IDLE;

	p_ptz_status->ErrorFlag = 0;

	onvif_get_time_str(p_ptz_status->UtcTime, sizeof(p_ptz_status->UtcTime), 0);	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_ContinuousMove(ContinuousMove_REQ * p_req)
{ 
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__ptz__ContinuousMove move;
	memset(&move,0x0,sizeof(LocalSDK__ptz__ContinuousMove));
	strcpy(move.ProfileToken,p_req->ProfileToken);
	{
		if(p_req->Velocity.PanTiltFlag){
			move.PanTilt.x=p_req->Velocity.PanTilt.x;
			move.PanTilt.y=p_req->Velocity.PanTilt.y;
		}
		if(p_req->Velocity.ZoomFlag){
			move.Zoom.x=p_req->Velocity.Zoom.x;
		}
	}
	SdkResult = LocalSDK__PTZMove(&move);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	return ONVIF_OK;

}

ONVIF_RET my_onvif_PTZ_Stop(PTZ_Stop_REQ * p_req)
{ 
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__ptz__StopMove move;
	memset(&move,0x0,sizeof(LocalSDK__ptz__StopMove));
	strcpy(move.ProfileToken,p_req->ProfileToken);
	if(p_req->PanTiltFlag){
		move.PanTilt = p_req->PanTilt;
	}
	if(p_req->ZoomFlag){
		move.Zoom = p_req->Zoom;
	}
	SdkResult = LocalSDK__PTZStopMove(&move);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_AbsoluteMove(AbsoluteMove_REQ * p_req)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_RelativeMove(RelativeMove_REQ * p_req)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}

ONVIF_RET my_onvif_SetPreset(SetPreset_REQ * p_req)
{ 
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__PTZPreset Preset;
	memset(&Preset,0x0,sizeof(LocalSDK__PTZPreset));
	strcpy(Preset.profiletoken ,(char *)p_req->ProfileToken);
	if(p_req->PresetNameFlag){
		strcpy(Preset.Name,p_req->PresetName);
	}
	if(p_req->PresetTokenFlag){
		strcpy(Preset.token,p_req->PresetToken);
	}
	SdkResult = LocalSDK__SetPreset(&Preset);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_RemovePreset(RemovePreset_REQ * p_req)
{ 
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__PTZPreset Preset;
	memset(&Preset,0x0,sizeof(LocalSDK__PTZPreset));
	strcpy(Preset.profiletoken,p_req->ProfileToken);
	strcpy(Preset.token,p_req->PresetToken);
	SdkResult = LocalSDK__RemovePreset(&Preset);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK; 
}

ONVIF_RET my_onvif_GotoPreset(GotoPreset_REQ * p_req)
{ 
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__PTZPreset Preset;
	memset(&Preset,0x0,sizeof(LocalSDK__PTZPreset));
	strcpy(Preset.profiletoken,p_req->ProfileToken);
	strcpy(Preset.token,p_req->PresetToken);
	SdkResult = LocalSDK__GotoPreset(&Preset);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK; 
}

ONVIF_RET my_onvif_GotoHomePosition(GotoHomePosition_REQ * p_req)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}
	
ONVIF_RET my_onvif_SetHomePosition(const char * token)
{
	return ONVIF_ERR_SERVICE_NOT_SUPPORT;
}


ONVIF_RET my_onvif_SetPTZConfiguration(SetConfiguration_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	vz_onvif_PTZConfiguration Config;
	memset(&Config,0x0,sizeof(vz_onvif_PTZConfiguration));

	strcpy(Config.Name, p_req->PTZConfiguration.Name);
	strcpy(Config.token, p_req->PTZConfiguration.token);
	strcpy(Config.NodeToken, p_req->PTZConfiguration.NodeToken);
	Config.DefaultPTZSpeedFlag = p_req->PTZConfiguration.DefaultPTZSpeedFlag;
	Config.DefaultPTZTimeoutFlag = p_req->PTZConfiguration.DefaultPTZTimeoutFlag;
	Config.PanTiltLimitsFlag = p_req->PTZConfiguration.PanTiltLimitsFlag;
	Config.ZoomLimitsFlag = p_req->PTZConfiguration.ZoomLimitsFlag;
	Config.ExtensionFlag = p_req->PTZConfiguration.ExtensionFlag;
	Config.UseCount = p_req->PTZConfiguration.UseCount;
	
	if (p_req->PTZConfiguration.DefaultPTZSpeedFlag)
	{
		Config.DefaultPTZSpeed.PanTiltFlag = p_req->PTZConfiguration.DefaultPTZSpeed.PanTiltFlag;
		Config.DefaultPTZSpeed.ZoomFlag = p_req->PTZConfiguration.DefaultPTZSpeed.ZoomFlag;
		if (p_req->PTZConfiguration.DefaultPTZSpeed.PanTiltFlag)
		{
			Config.DefaultPTZSpeed.PanTilt.x = p_req->PTZConfiguration.DefaultPTZSpeed.PanTilt.x;
			Config.DefaultPTZSpeed.PanTilt.y = p_req->PTZConfiguration.DefaultPTZSpeed.PanTilt.y;
		}

		if (p_req->PTZConfiguration.DefaultPTZSpeed.ZoomFlag)
		{
			Config.DefaultPTZSpeed.Zoom.x = p_req->PTZConfiguration.DefaultPTZSpeed.Zoom.x;
		}
	}

	if (p_req->PTZConfiguration.DefaultPTZTimeoutFlag)
	{
		Config.DefaultPTZTimeout = p_req->PTZConfiguration.DefaultPTZTimeout;
	}

	if (p_req->PTZConfiguration.PanTiltLimitsFlag)
	{
		Config.PanTiltLimits.XRange.Min = p_req->PTZConfiguration.PanTiltLimits.XRange.Min;
		Config.PanTiltLimits.XRange.Max = p_req->PTZConfiguration.PanTiltLimits.XRange.Max;
		Config.PanTiltLimits.YRange.Min = p_req->PTZConfiguration.PanTiltLimits.YRange.Min;
		Config.PanTiltLimits.YRange.Max = p_req->PTZConfiguration.PanTiltLimits.YRange.Max;
	}

	if (p_req->PTZConfiguration.ZoomLimitsFlag)
	{
		Config.ZoomLimits.XRange.Min = p_req->PTZConfiguration.ZoomLimits.XRange.Min;
		Config.ZoomLimits.XRange.Max = p_req->PTZConfiguration.ZoomLimits.XRange.Max;
	}

	if (p_req->PTZConfiguration.ExtensionFlag)
	{
		if (p_req->PTZConfiguration.Extension.PTControlDirectionFlag)
		{
			if (p_req->PTZConfiguration.Extension.PTControlDirection.EFlipFlag)
			{
				Config.Extension.EFlip = p_req->PTZConfiguration.Extension.PTControlDirection.EFlip;
			}

			if (p_req->PTZConfiguration.Extension.PTControlDirection.ReverseFlag)
			{
				Config.Extension.Reverse = p_req->PTZConfiguration.Extension.PTControlDirection.Reverse;
			}
		}
	}

	SdkResult = LocalSDK__SetPTZConfiguration(&Config);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;
}





ONVIF_RET my_onvif_GetNodes(GetNodes_RES * p_res)
{ 
	Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();
	int i = 0;
	for( i = 0;i<MAX_PTZ_NODE_NUM;i++)
	{
		if(pOnvifInfo->ptz_node[i].used)
		{
			vz_onvif_PTZNode *vz_PTZNode = &pOnvifInfo->ptz_node[i].PTZNode;
			PTZ_NODE * p_node = p_res->p_node;
			PTZ_NODE * p_temp_node = (PTZ_NODE *)get_one_item(ONVIF_TYPE_PTZ_NODE);// malloc(sizeof(PTZ_NODE));
			if (NULL == p_temp_node)
			{
				return ONVIF_OK;
			}
			memset(p_temp_node,0,sizeof(PTZ_NODE));
			p_temp_node->next = NULL;
			iGetOneNode(p_temp_node,vz_PTZNode);

			if (NULL == p_node)
			{
				p_res->p_node = p_temp_node;
			}
			else
			{
				while (p_node && p_node->next) p_node = p_node->next;
			
				p_node->next = p_temp_node;
			}
		}
	}
	
	return ONVIF_OK; 
}


ONVIF_RET my_onvif_GetNode(const char *PTZNodetoken,GetNode_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int NodeID = 0;
	SdkResult = LocalSDK__GetNode((char *)PTZNodetoken,&NodeID);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();
	
	iGetOneNode(&p_res->struNode,&pOnvifInfo->ptz_node[NodeID].PTZNode);

	return ONVIF_OK;
}




ONVIF_RET my_onvif_GetPTZConfigurations(GetPTZCfgs_RES * p_res)
{ 
	Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();
	int i = 0;
	for( i = 0;i<MAX_PTZ_CFG_NUM;i++)
	{
		if(pOnvifInfo->ptz_cfg[i].used)
		{
			vz_onvif_PTZConfiguration *vz_PTZCfg = &pOnvifInfo->ptz_cfg[i].PTZConfiguration;
			PTZ_CFG * p_ptzcfg = p_res->p_PTZCfg;
			PTZ_CFG * p_temp_ptzcfg = (PTZ_CFG *)get_one_item(ONVIF_TYPE_PTZ);// malloc(sizeof(PTZ_CFG));
			if (NULL == p_temp_ptzcfg)
			{
				return ONVIF_OK;
			}
			memset(p_temp_ptzcfg,0,sizeof(PTZ_CFG));
			p_temp_ptzcfg->next = NULL;
			iGetOnePTZCfg(p_temp_ptzcfg,vz_PTZCfg);

			if (NULL == p_ptzcfg)
			{
				p_res->p_PTZCfg = p_temp_ptzcfg;
			}
			else
			{
				while (p_ptzcfg && p_ptzcfg->next) p_ptzcfg = p_ptzcfg->next;
			
				p_ptzcfg->next = p_temp_ptzcfg;
			}
		}
	}
	
	return ONVIF_OK; 
}


ONVIF_RET my_onvif_GetPTZConfiguration(const char *ptzCfgToken,GetPTZCfg_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int CfgID = 0;
	SdkResult = LocalSDK__GetPTZConfiguration((char *)ptzCfgToken,&CfgID);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();
	
	iGetOnePTZCfg(&p_res->struPTZCfg,&pOnvifInfo->ptz_cfg[CfgID].PTZConfiguration);

	return ONVIF_OK;
}


ONVIF_RET my_onvif_GetPTZConfigurationOptions(const char *ptzCfgToken,GetPTZCfgOptions_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int NodeID = 0;
	SdkResult = LocalSDK__GetPTZConfigurationOptions((char *)ptzCfgToken,&NodeID);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();
	
	iGetOneNode(&p_res->struNode,&pOnvifInfo->ptz_node[NodeID].PTZNode);
	
	p_res->PTZConfigurationOptions.PTControlDirectionFlag = pOnvifInfo->PTZCfgOptions.PTControlDirectionFlag;
	if(p_res->PTZConfigurationOptions.PTControlDirectionFlag)
	{
		vz_onvif_PTControlDirectionOptions *vz_Direction = &pOnvifInfo->PTZCfgOptions.PTControlDirection;
		onvif_PTControlDirectionOptions *Direction = &p_res->PTZConfigurationOptions.PTControlDirection;
		Direction->EFlipMode_OFF = vz_Direction->EFlipMode_OFF;
		Direction->EFlipMode_ON = vz_Direction->EFlipMode_ON;
		Direction->EFlipMode_Extended = vz_Direction->EFlipMode_Extended;
		Direction->ReverseMode_OFF = vz_Direction->ReverseMode_OFF;
		Direction->ReverseMode_ON = vz_Direction->ReverseMode_ON;
		Direction->ReverseMode_AUTO = vz_Direction->ReverseMode_AUTO;
		Direction->ReverseMode_Extended = vz_Direction->ReverseMode_Extended;
	}
	p_res->PTZConfigurationOptions.PTZTimeout.Min = pOnvifInfo->PTZCfgOptions.PTZTimeout.Min;
	p_res->PTZConfigurationOptions.PTZTimeout.Max = pOnvifInfo->PTZCfgOptions.PTZTimeout.Max;

	return ONVIF_OK;
}


ONVIF_RET my_onvif_GetPresets(const char *ptzProfileToken,GetPresets_RES * p_res)
{
	return ONVIF_OK;
}


ONVIF_NOTIFYMESSAGE * my_onvif_init_notify_message(EUA * p_eua,void *pEventInfo)
{
	if(p_eua == NULL || pEventInfo == NULL)
		return NULL;

	EventConfig *pEventCfg = (EventConfig *)pEventInfo;
	ONVIF_NOTIFYMESSAGE * p_messages = NULL;
	int index = 0;
	int first = 0;
	VZ_IVS_TARGET *pTag = &pEventCfg->tag;
	char topic[256]={0};
	switch(pEventCfg->dwRuleType)
	{
	case IVS_RULE_LINE:
	case IVS_RULE_LINE_EX:
		strcpy(topic, "tns1:RuleEngine/LineDetector/Crossed");
		break;
	case IVS_RULE_AREA:
	case IVS_RULE_AREA_EX:
		strcpy(topic, "tns1:RuleEngine/FieldDetector//.");
		
		break;
	default :
		strcpy(topic, "tns1:RuleEngine//.");
		break;
	}
	
	if (p_eua->FiltersFlag)
	{
		int i;
		int lenTopic = strlen(topic);
		if(topic[lenTopic -1] == '.')
		{
			lenTopic -= 3;
		}
		ONVIF_FILTER *p_filter =&p_eua->Filters;
		for (i = 0; i < 10; i++)
		{
			if (p_filter->TopicExpression[i][0] != '\0')
			{
				int len1 =strlen(p_filter->TopicExpression[i]);
				int minlen = lenTopic > len1 ? len1 : lenTopic;
				if(strncmp(topic,p_filter->TopicExpression[i],minlen) == 0)
					break;
			}
		}
		
		if(i >= 10)
		{
			printf("topic filter not match!!!\n");
			return NULL;
			//continue;
		}
	}
	
	ONVIF_NOTIFYMESSAGE * p_message = NULL;
	
	if(first == 0)
	{
		p_message = onvif_add_notify_message(NULL);
	}
	else 
		p_message = onvif_add_notify_message(&p_messages);
	if (p_message)
	{
		if(first == 0)
		{
			first = 1;
			p_messages = p_message;
		}
		ONVIF_SIMPLEITEM * p_simpleitem;
		
		strcpy(p_message->NotificationMessage.ConsumerAddress, p_eua->consumer_addr);
		strcpy(p_message->NotificationMessage.ProducterAddress, p_eua->producter_addr);
		strcpy(p_message->NotificationMessage.Dialect, "http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet");
		strcpy(p_message->NotificationMessage.Topic, topic);
		
		p_message->NotificationMessage.Message.PropertyOperationFlag = 1;
		p_message->NotificationMessage.Message.PropertyOperation = PropertyOperation_Changed;
		p_message->NotificationMessage.Message.UtcTime = time(NULL);
	
		//set target info start
		p_simpleitem = onvif_add_simple_item(&p_message->NotificationMessage.Message.Key.SimpleItem);
		if (p_simpleitem)
		{
			p_message->NotificationMessage.Message.KeyFlag = 1;
			strcpy(p_simpleitem->SimpleItem.Name, "ObjectId");
			sprintf(p_simpleitem->SimpleItem.Value,"%d",pTag->id);
		}
		p_simpleitem = onvif_add_simple_item(&p_message->NotificationMessage.Message.Data.SimpleItem);
		if (p_simpleitem)
		{
			p_message->NotificationMessage.Message.DataFlag = 1;
			strcpy(p_simpleitem->SimpleItem.Name, "kind");
			sprintf(p_simpleitem->SimpleItem.Value,"%d",pTag->kind);
		}
		p_simpleitem = onvif_add_simple_item(&p_message->NotificationMessage.Message.Data.SimpleItem);
		if (p_simpleitem)
		{
			strcpy(p_simpleitem->SimpleItem.Name, "x");
			sprintf(p_simpleitem->SimpleItem.Value,"%d",pTag->x);
		}
		p_simpleitem = onvif_add_simple_item(&p_message->NotificationMessage.Message.Data.SimpleItem);
		if (p_simpleitem)
		{
			strcpy(p_simpleitem->SimpleItem.Name, "y");
			sprintf(p_simpleitem->SimpleItem.Value,"%d",pTag->y);
		}
		p_simpleitem = onvif_add_simple_item(&p_message->NotificationMessage.Message.Data.SimpleItem);
		if (p_simpleitem)
		{
			strcpy(p_simpleitem->SimpleItem.Name, "with");
			sprintf(p_simpleitem->SimpleItem.Value,"%d",pTag->w);
		}
		p_simpleitem = onvif_add_simple_item(&p_message->NotificationMessage.Message.Data.SimpleItem);
		if (p_simpleitem)
		{
			strcpy(p_simpleitem->SimpleItem.Name, "height");
			sprintf(p_simpleitem->SimpleItem.Value,"%d",pTag->h);
		}
		p_simpleitem = onvif_add_simple_item(&p_message->NotificationMessage.Message.Data.SimpleItem);
		if (p_simpleitem)
		{
			strcpy(p_simpleitem->SimpleItem.Name, "dwBitsRules");
			sprintf(p_simpleitem->SimpleItem.Value,"%d",pTag->dwBitsRules);
		}
		p_simpleitem = onvif_add_simple_item(&p_message->NotificationMessage.Message.Data.SimpleItem);
		if (p_simpleitem)
		{
			strcpy(p_simpleitem->SimpleItem.Name, "dwBitsEvent");
			sprintf(p_simpleitem->SimpleItem.Value,"%d",pTag->dwBitsEvent);
		}
		p_simpleitem = onvif_add_simple_item(&p_message->NotificationMessage.Message.Data.SimpleItem);
		if (p_simpleitem)
		{
			strcpy(p_simpleitem->SimpleItem.Name, "tv_sec");
			sprintf(p_simpleitem->SimpleItem.Value,"%ld",pEventCfg->tv_sec);
		}
		p_simpleitem = onvif_add_simple_item(&p_message->NotificationMessage.Message.Data.SimpleItem);
		if (p_simpleitem)
		{
			strcpy(p_simpleitem->SimpleItem.Name, "tv_usec");
			sprintf(p_simpleitem->SimpleItem.Value,"%ld",pEventCfg->tv_usec);
		}

		p_simpleitem = onvif_add_simple_item(&p_message->NotificationMessage.Message.Source.SimpleItem);
		if (p_simpleitem)
		{
			p_message->NotificationMessage.Message.SourceFlag = 1;
			strcpy(p_simpleitem->SimpleItem.Name, "Rule");
			//strcpy(p_simpleitem->SimpleItem.Value, pEventCfg->byRuleName);
			char Out[128] = {0};
			int convert_ret = 0;
			convert_ret = GB2312_to_UTF8(pEventCfg->RuleName,strlen(pEventCfg->RuleName),Out,128);
			if(convert_ret == 0)
			{
				strncpy(p_simpleitem->SimpleItem.Value, Out,sizeof(p_simpleitem->SimpleItem.Value));
			}
			else
			{
				printf("code_convert error!!!\n");
				strncpy(p_simpleitem->SimpleItem.Value, pEventCfg->RuleName,sizeof(p_simpleitem->SimpleItem.Value));
			}
		}
		Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();
		int index = pOnvifInfo->Profiles[pEventCfg->channel].meta_profile[0].VA_ID;
		char *pVSrcToken = pOnvifInfo->Profiles[pEventCfg->channel].meta_profile[0].VideoSourceConfig.token;
		char *pVAToken = pOnvifInfo->VideoAnalytics[index].VACfg.token;
		p_simpleitem = onvif_add_simple_item(&p_message->NotificationMessage.Message.Source.SimpleItem);
		if (p_simpleitem)
		{
			strcpy(p_simpleitem->SimpleItem.Name, "VideoAnalyticsConfigurationToken");
			strncpy(p_simpleitem->SimpleItem.Value, pVAToken,sizeof(p_simpleitem->SimpleItem.Value));
		}
		p_simpleitem = onvif_add_simple_item(&p_message->NotificationMessage.Message.Source.SimpleItem);
		if (p_simpleitem)
		{
			strcpy(p_simpleitem->SimpleItem.Name, "VideoSourceConfigurationToken");
			strncpy(p_simpleitem->SimpleItem.Value, pVSrcToken,sizeof(p_simpleitem->SimpleItem.Value));
		}
		//set target info end
	}
	
	return p_messages;
}

ONVIF_RET my_onvif_SetImagingSettings(SetImagingSettings_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__ImagingSetting Imaging;
	memset(&Imaging,0x0,sizeof(LocalSDK__ImagingSetting));
	Imaging.Brightness=-1;
	Imaging.ColorSaturation=-1;
	Imaging.Contrast=-1;
	Imaging.hue=-1;
	strcpy(Imaging.VideoSourceToken,p_req->VideoSourceToken);
	if(p_req->ForcePersistenceFlag)
	{
		Imaging.ForcePersistence = p_req->ForcePersistence;
	}
/*
	if(p_req->ImagingSettings.BacklightCompensation)
	{
		if(p_req->ImagingSettings.BacklightCompensation->Level)
			Imaging.BLC.BLCLevel =*p_req->ImagingSettings.BacklightCompensation->Level;
		Imaging.BLC.Onoff =p_req->ImagingSettings.BacklightCompensation->Mode;
	}
	*/
	if(p_req->ImagingSettings.BrightnessFlag)
	{
		Imaging.Brightness=p_req->ImagingSettings.Brightness;
	}
	
	if(p_req->ImagingSettings.ColorSaturationFlag)
	{
		Imaging.ColorSaturation=p_req->ImagingSettings.ColorSaturation;
	}
	
	if(p_req->ImagingSettings.ContrastFlag)
	{
		Imaging.Contrast=p_req->ImagingSettings.Contrast;
	}/*
	if(p_req->ImagingSettings.SharpnessFlag)
	{
		Imaging.Sharpness= p_req->ImagingSettings.Sharpness;
	}*/
	
	SdkResult = LocalSDK__SetImagingSettings(&Imaging);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetImagingSettings(GetImagSettings_RES * p_res)
{ 
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__ImagingSetting Imaging;
	memset(&Imaging,0,sizeof(LocalSDK__ImagingSetting));
	strcpy(Imaging.VideoSourceToken,p_res->VideoSourceToken);
	SdkResult = LocalSDK__GetImagingSettings(&Imaging);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	p_res->Imaging.BacklightCompensationFlag=0;
	p_res->Imaging.ExposureFlag=0;
	p_res->Imaging.FocusFlag=0;
	p_res->Imaging.SharpnessFlag=0;
	p_res->Imaging.WideDynamicRangeFlag=0;
	p_res->Imaging.WhiteBalanceFlag=0;

	p_res->Imaging.IrCutFilterFlag=1;
	p_res->Imaging.IrCutFilter = IrCutFilterMode_OFF;
	p_res->Imaging.BrightnessFlag = 1;
	p_res->Imaging.Brightness=Imaging.Brightness;
	p_res->Imaging.ColorSaturationFlag = 1;
	p_res->Imaging.ColorSaturation=Imaging.ColorSaturation;
	p_res->Imaging.ContrastFlag = 1;
	p_res->Imaging.Contrast=Imaging.Contrast;

	return ONVIF_OK;
}


ONVIF_RET my_onvif_GetImagingOptions(GetImagingOptions_RES * p_res)
{ 
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__ImagingOption ImagingOption;
	memset(&ImagingOption,0x0,sizeof(LocalSDK__ImagingOption));
	strcpy(ImagingOption.VideoSourceToken,p_res->VideoSourceToken);
	SdkResult = LocalSDK__GetImagingOptions(&ImagingOption);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	p_res->ImagingOptions.IrCutFilterMode_ON=0;	
	p_res->ImagingOptions.IrCutFilterMode_OFF=1;
	p_res->ImagingOptions.IrCutFilterMode_AUTO=0;
	p_res->ImagingOptions.BacklightCompensationFlag = 0;
	p_res->ImagingOptions.ExposureFlag = 0;
	p_res->ImagingOptions.FocusFlag = 0;
	p_res->ImagingOptions.SharpnessFlag = 0;
	p_res->ImagingOptions.WideDynamicRangeFlag = 0;
	p_res->ImagingOptions.WhiteBalanceFlag = 0;

	p_res->ImagingOptions.BrightnessFlag = 1;;
	p_res->ImagingOptions.Brightness.Min = (float)ImagingOption.Brightness.Min;
	p_res->ImagingOptions.Brightness.Max= (float)ImagingOption.Brightness.Max;
	p_res->ImagingOptions.ColorSaturationFlag = 1;
	p_res->ImagingOptions.ColorSaturation.Min = (float)ImagingOption.ColorSaturation.Min;
	p_res->ImagingOptions.ColorSaturation.Max= (float)ImagingOption.ColorSaturation.Max;
	p_res->ImagingOptions.ContrastFlag = 1;
	p_res->ImagingOptions.Contrast.Min = (float)ImagingOption.Contrast.Min;
	p_res->ImagingOptions.Contrast.Max= (float)ImagingOption.Contrast.Max;

	return ONVIF_OK; 
}

ONVIF_RET my_onvif_Imaging_Move(Move_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	SdkResult = LocalSDK__Find_Video_Source(p_req->VideoSourceToken);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_ERR_NO_IMAGEING_FOR_SOURCE;
}

ONVIF_RET my_onvif_Imaging_Stop(char *VideoSourceToken)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	SdkResult = LocalSDK__Find_Video_Source(VideoSourceToken);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_ERR_NO_IMAGEING_FOR_SOURCE;
}


ONVIF_RET my_onvif_ImagingGetStatus(const char * VideoSourceToken, onvif_ImagingStatus * p_status)
{
	// todo : add get imaging status code ...
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	SdkResult = LocalSDK__Find_Video_Source((char *)VideoSourceToken);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	p_status->FocusStatusFlag = 1;
	p_status->FocusStatus.Position = 0.0;
	p_status->FocusStatus.MoveStatus = MoveStatus_IDLE;

	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetMoveOptions(const char * VideoSourceToken)
{
	// todo : add get imaging status code ...
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	SdkResult = LocalSDK__Find_Video_Source((char *)VideoSourceToken);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_CreateRecording(CreateRecording_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	vz_onvif_Recording Recording;
	memset(&Recording,0x0,sizeof(Recording));
	snprintf(Recording.RecordingToken, ONVIF_TOKEN_LEN, "RECORDING_%03d", g_onvif_cls.recording_idx);
	g_onvif_cls.recording_idx++;

	strcpy(p_req->RecordingToken, Recording.RecordingToken);
	
	Recording.Config.MaximumRetentionTimeFlag = p_req->RecordingConfiguration.MaximumRetentionTimeFlag;
	if(Recording.Config.MaximumRetentionTimeFlag)
	{
		Recording.Config.MaximumRetentionTime = p_req->RecordingConfiguration.MaximumRetentionTime;
	}
	strcpy(Recording.Config.Content,p_req->RecordingConfiguration.Content);
	
	onvif_RecordingSourceInformation *pSource=&p_req->RecordingConfiguration.Source; 
	strcpy(Recording.Config.Source.SourceId , pSource->SourceId);
	strcpy(Recording.Config.Source.Name ,pSource->Name);
	strcpy(Recording.Config.Source.Location,pSource->Location);
	strcpy(Recording.Config.Source.Description,pSource->Description);
	strcpy(Recording.Config.Source.Address,pSource->Address);

	SdkResult = LocalSDK__CreateRecording(&Recording);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_DeleteRecording(const char * p_RecordingToken)
{
	
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	SdkResult = LocalSDK__DeleteRecording((char *)p_RecordingToken);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	return ONVIF_OK;
}

ONVIF_RET my_onvif_SetRecordingConfiguration(SetRecordingConfiguration_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	vz_onvif_Recording Recording;
	memset(&Recording,0x0,sizeof(Recording));

	strcpy(Recording.RecordingToken,p_req->RecordingToken);
	
	Recording.Config.MaximumRetentionTimeFlag = p_req->RecordingConfiguration.MaximumRetentionTimeFlag;
	if(Recording.Config.MaximumRetentionTimeFlag)
	{
		Recording.Config.MaximumRetentionTime = p_req->RecordingConfiguration.MaximumRetentionTime;
	}
	strcpy(Recording.Config.Content,p_req->RecordingConfiguration.Content);
	
	onvif_RecordingSourceInformation *pSource=&p_req->RecordingConfiguration.Source; 
	strcpy(Recording.Config.Source.SourceId , pSource->SourceId);
	strcpy(Recording.Config.Source.Name ,pSource->Name);
	strcpy(Recording.Config.Source.Location,pSource->Location);
	strcpy(Recording.Config.Source.Description,pSource->Description);
	strcpy(Recording.Config.Source.Address,pSource->Address);
	SdkResult = LocalSDK__SetRecordingConfiguration(&Recording);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_CreateTrack(CreateTrack_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	vz_onvif_Track Track;
	memset(&Track,0x0,sizeof(Track));
	snprintf(Track.TrackToken, ONVIF_TOKEN_LEN, "TRACK%03d", g_onvif_cls.recording_idx);
	g_onvif_cls.recording_idx++;

	strcpy(p_req->TrackToken, Track.TrackToken);
	
	Track.Configuration.TrackType = p_req->TrackConfiguration.TrackType;
	strcpy(Track.Configuration.Description, p_req->TrackConfiguration.Description);

	SdkResult = LocalSDK__CreateTrack(p_req->RecordingToken,&Track);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}



ONVIF_RET my_onvif_DeleteTrack(DeleteTrack_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	SdkResult = LocalSDK__DeleteTrack(p_req->RecordingToken,p_req->TrackToken);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;
}


ONVIF_RET my_onvif_SetTrackConfiguration(SetTrackConfiguration_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	vz_onvif_Track Track;
	memset(&Track,0x0,sizeof(Track));
	strcpy(Track.TrackToken,p_req->TrackToken);
	
	Track.Configuration.TrackType = p_req->TrackConfiguration.TrackType;
	strcpy(Track.Configuration.Description, p_req->TrackConfiguration.Description);
	SdkResult = LocalSDK__SetTrackConfiguration(p_req->RecordingToken,&Track);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_CreateRecordingJob(CreateRecordingJob_REQ  * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	vz_onvif_RecordingJob RecordingJob;
	memset(&RecordingJob,0x0,sizeof(RecordingJob));
	snprintf(RecordingJob.RecordingJobToken, ONVIF_TOKEN_LEN, "RECORDINGJOB_%03d", g_onvif_cls.recordingjob_idx);
	g_onvif_cls.recordingjob_idx++;

	strcpy(p_req->JobToken, RecordingJob.RecordingJobToken);
	strcpy(RecordingJob.RecordingToken, p_req->JobConfiguration.RecordingToken);
	strcpy(RecordingJob.Mode, p_req->JobConfiguration.Mode);
	RecordingJob.Priority = p_req->JobConfiguration.Priority;
	RecordingJob.sizeSource = p_req->JobConfiguration.sizeSource;

	if(p_req->JobConfiguration.sizeSource)
	{
		int i = 0;
		for(i = 0;i<p_req->JobConfiguration.sizeSource&&i<MAX_RJTRACK_NUM;i++)
		{
			onvif_RecordingJobSource *p_source = &p_req->JobConfiguration.Source[i];
			RecordingJob.JobSource[i].SourceTokenFlag = p_source->SourceTokenFlag;
			if(p_source->SourceTokenFlag)
			{
				RecordingJob.JobSource[i].SourceToken.TypeFlag=p_source->SourceToken.TypeFlag;
				if(p_source->SourceToken.TypeFlag)
				{
					strcpy(RecordingJob.JobSource[i].SourceToken.Type,p_source->SourceToken.Type);
				}
				strcpy(RecordingJob.JobSource[i].SourceToken.Token,p_source->SourceToken.Token);
			}
			
			RecordingJob.JobSource[i].AutoCreateReceiverFlag= p_source->AutoCreateReceiverFlag;
			if(p_source->AutoCreateReceiverFlag)
			{
				RecordingJob.JobSource[i].AutoCreateReceiver = p_source->AutoCreateReceiver;
			}
			
			RecordingJob.JobSource[i].sizeTracks = p_source->sizeTracks;
			if(p_source->sizeTracks)
			{
				int j = 0;
				for(j = 0;j<p_source->sizeTracks&&j<MAX_RJTRACK_NUM;j++)
				{
					strcpy(RecordingJob.JobSource[i].Tracks[j].SourceTag,p_source->Tracks[j].SourceTag);
					strcpy(RecordingJob.JobSource[i].Tracks[j].Destination,p_source->Tracks[j].Destination);
				}
			}
		}
	}

	SdkResult = LocalSDK__CreateRecordingJob(&RecordingJob);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
 	int k =0;
	for (k = 0; k < RecordingJob.sizeSource; k++)
	{
		p_req->JobConfiguration.Source[k].SourceTokenFlag = RecordingJob.JobSource[k].SourceTokenFlag;
		p_req->JobConfiguration.Source[k].AutoCreateReceiverFlag= RecordingJob.JobSource[k].AutoCreateReceiverFlag;
		p_req->JobConfiguration.Source[k].AutoCreateReceiver= RecordingJob.JobSource[k].AutoCreateReceiver;
		p_req->JobConfiguration.Source[k].SourceToken.TypeFlag =RecordingJob.JobSource[k].SourceToken.TypeFlag;
		if(p_req->JobConfiguration.Source[k].SourceToken.TypeFlag)
			strcpy(p_req->JobConfiguration.Source[k].SourceToken.Type,RecordingJob.JobSource[k].SourceToken.Type);
		strcpy(p_req->JobConfiguration.Source[k].SourceToken.Token,RecordingJob.JobSource[k].SourceToken.Token);
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_DeleteRecordingJob(const char * p_JobToken)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	SdkResult = LocalSDK__DeleteRecordingJob((char *)p_JobToken);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	return ONVIF_OK;
}

ONVIF_RET my_onvif_SetRecordingJobConfiguration(SetRecordingJobConfiguration_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	vz_onvif_RecordingJob RecordingJob;
	memset(&RecordingJob,0x0,sizeof(RecordingJob));
	strcpy(RecordingJob.RecordingJobToken,p_req->JobToken);
	strcpy(RecordingJob.RecordingToken, p_req->JobConfiguration.RecordingToken);
	strcpy(RecordingJob.Mode, p_req->JobConfiguration.Mode);
	RecordingJob.Priority = p_req->JobConfiguration.Priority;
	RecordingJob.sizeSource = p_req->JobConfiguration.sizeSource;
	if(p_req->JobConfiguration.sizeSource)
	{
		int i = 0;
		for(i = 0;i<p_req->JobConfiguration.sizeSource&&i<MAX_RJTRACK_NUM;i++)
		{
			onvif_RecordingJobSource *p_source = &p_req->JobConfiguration.Source[i];
			RecordingJob.JobSource[i].SourceTokenFlag = p_source->SourceTokenFlag;
			if(p_source->SourceTokenFlag)
			{
				RecordingJob.JobSource[i].SourceToken.TypeFlag=p_source->SourceToken.TypeFlag;
				if(p_source->SourceToken.TypeFlag)
				{
					strcpy(RecordingJob.JobSource[i].SourceToken.Type,p_source->SourceToken.Type);
				}
				strcpy(RecordingJob.JobSource[i].SourceToken.Token,p_source->SourceToken.Token);
			}
			
			RecordingJob.JobSource[i].AutoCreateReceiverFlag= p_source->AutoCreateReceiverFlag;
			if(p_source->AutoCreateReceiverFlag)
			{
				RecordingJob.JobSource[i].AutoCreateReceiver = p_source->AutoCreateReceiver;
			}
			
			RecordingJob.JobSource[i].sizeTracks = p_source->sizeTracks;
			if(p_source->sizeTracks)
			{
				int j = 0;
				for(j = 0;j<p_source->sizeTracks&&j<MAX_RJTRACK_NUM;j++)
				{
					strcpy(RecordingJob.JobSource[i].Tracks[j].SourceTag,p_source->Tracks[j].SourceTag);
					strcpy(RecordingJob.JobSource[i].Tracks[j].Destination,p_source->Tracks[j].Destination);
				}
			}
		}
	}

	SdkResult = LocalSDK__SetRecordingJobConfiguration(&RecordingJob);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_SetRecordingJobMode(SetRecordingJobMode_REQ * p_req)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__SetRecordingJob RecordingJob;
	memset(&RecordingJob,0,sizeof(RecordingJob));
	strcpy(RecordingJob.JobToken,p_req->JobToken);
	strcpy(RecordingJob.Mode, p_req->Mode);

	SdkResult = LocalSDK__SetRecordingJobMode(&RecordingJob);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetRecordingJobState(const char * p_JobToken, onvif_RecordingJobStateInformation * p_res)
{
	int i, j;
	
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	vz_onvif_RecordingJob RecordingJob;
	memset(&RecordingJob,0x0,sizeof(RecordingJob));
	strcpy(RecordingJob.RecordingJobToken,p_JobToken);
	SdkResult = LocalSDK__GetRecordingJobState(&RecordingJob);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	strcpy(p_res->RecordingToken, RecordingJob.RecordingToken);
	strcpy(p_res->State, RecordingJob.Mode);

	p_res->sizeSources = RecordingJob.sizeSource;
	
	for (i = 0; i < RecordingJob.sizeSource; i++)
	{
		p_res->Sources[i].SourceToken.TypeFlag = 1;
		strcpy(p_res->Sources[i].SourceToken.Token, RecordingJob.JobSource[i].SourceToken.Token);
		strcpy(p_res->Sources[i].SourceToken.Type, RecordingJob.JobSource[i].SourceToken.Type);

		strcpy(p_res->Sources[i].State, "Idle");

		p_res->Sources[i].sizeTrack = RecordingJob.JobSource[i].sizeTracks;
		
		for (j = 0; j < RecordingJob.JobSource[i].sizeTracks; j++)
		{
			strcpy(p_res->Sources[i].Track[j].SourceTag, RecordingJob.JobSource[i].Tracks[j].SourceTag);
			strcpy(p_res->Sources[i].Track[j].Destination, RecordingJob.JobSource[i].Tracks[j].Destination);
			strcpy(p_res->Sources[i].Track[j].State, "Idle");
		}
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetRecordingOptions(const char * p_RecordingToken, onvif_RecordingOptions * p_res)
{
	
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__RecordingOptions Options;
	memset(&Options,0x0,sizeof(Options));
	strcpy(Options.RecordingToken ,(char *)p_RecordingToken);
	SdkResult = LocalSDK_GetRecordingOptions(&Options);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	p_res->Track.SpareVideoFlag = 1;
	p_res->Track.SpareVideo = Options.Track.SpareVideo;	
	p_res->Track.SpareAudioFlag = 1;
	p_res->Track.SpareAudio = Options.Track.SpareAudio;
	p_res->Track.SpareMetadataFlag = 1;
	p_res->Track.SpareMetadata = Options.Track.SpareMetadata;	
	p_res->Track.SpareTotalFlag = 1;
	p_res->Track.SpareTotal = Options.Track.SpareTotal;
	
	p_res->Job.SpareFlag = 1;
	p_res->Job.Spare = Options.Job.Spare;
	p_res->Job.CompatibleSourcesFlag = 1;

	strncpy(p_res->Job.CompatibleSources,Options.Job.CompatibleSources,sizeof(p_res->Job.CompatibleSources));
		
	return ONVIF_OK;
}


ONVIF_RET my_onvif_GetRecordingSummary(GetRecordingSummary_RES * p_summary)
{
	// todo : modify the information ...
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__RecordingSummary_Info Info;
	memset(&Info,0x0,sizeof(LocalSDK__RecordingSummary_Info));
	SdkResult=LocalSDK__GetRecordingSummary(&Info);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	p_summary->Summary.DataFrom = Info.DataFrom;
	p_summary->Summary.DataUntil = Info.DataUntil;
	p_summary->Summary.NumberRecordings = Info.NumberRecordings;

	return ONVIF_OK;
}


ONVIF_RET my_onvif_GetRecordingInformation(const char * p_RecordingToken, GetRecordingInformation_RES * p_res)
{ 
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__RecordingInfo RecordingInfo;
	memset(&RecordingInfo,0x0,sizeof(LocalSDK__RecordingInfo));
	strcpy(RecordingInfo.RecordingToken,(char *)p_RecordingToken);
	SdkResult=LocalSDK__GetRecordingList(&RecordingInfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	// todo : fill the recording information ...
	strcpy(p_res->RecordingInformation.RecordingToken, RecordingInfo.RecordingToken);
	p_res->RecordingInformation.EarliestRecordingFlag = 1;
	p_res->RecordingInformation.EarliestRecording = RecordingInfo.EarliestRecording;
	p_res->RecordingInformation.LatestRecordingFlag = 1;
	p_res->RecordingInformation.LatestRecording = RecordingInfo.LatestRecording;
	
	strncpy(p_res->RecordingInformation.Content, RecordingInfo.Content,sizeof(p_res->RecordingInformation.Content));
	onvif_RecordingSourceInformation *pSource=&p_res->RecordingInformation.Source; 
	strcpy(pSource->SourceId,RecordingInfo.Source.SourceId);
	strcpy(pSource->Name,RecordingInfo.Source.Name);
	strcpy(pSource->Location,RecordingInfo.Source.Location);
	strcpy(pSource->Description,RecordingInfo.Source.Description);
	strcpy(pSource->Address,RecordingInfo.Source.Address);


	if(RecordingInfo.Track_num)
	{
		int i=0;
		int index =0;
		for(i=0;i<RecordingInfo.Track_num;i++)
		{	
			if(RecordingInfo.Track[i].List[0] == 1)
			{
				int index = p_res->RecordingInformation.sizeTrack;
				
				strcpy(p_res->RecordingInformation.Track[index].TrackToken, RecordingInfo.Track[i].TrackToken);
				p_res->RecordingInformation.Track[index].TrackType = RecordingInfo.Track[i].TrackType;
				strcpy(p_res->RecordingInformation.Track[index].Description, RecordingInfo.Track[i].List[2]);//list是较大，放不下
				p_res->RecordingInformation.Track[index].DataFrom = RecordingInfo.Track[i].DataFrom;
				p_res->RecordingInformation.Track[index].DataTo = RecordingInfo.Track[i].DataTo;
				
				p_res->RecordingInformation.sizeTrack++;
				
				if (p_res->RecordingInformation.sizeTrack >= 5)
				{
					break;
				}
			}
		}
	}
	p_res->RecordingInformation.RecordingStatus = RecordingInfo.RecordingStatus;

	return ONVIF_OK; 
}



ONVIF_RET my_onvif_GetMediaAttributes(GetMediaAttributes_REQ * p_req, GetMediaAttributes_RES * p_res)
{
	int i, j, k;	
	ONVIF_TRACK * p_track;
	ONVIF_RECORDING * p_recording;
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	
	for (i = 0; i < p_req->sizeRecordingTokens; i++)
	{
		vz_onvif_Recording recordinfo;
		memset(&recordinfo,0,sizeof(recordinfo));
		SdkResult = LocalSDK_find_recording(p_req->RecordingTokens[i],&recordinfo);
		if (VZ_LOCALAPI_FAILED == SdkResult)
		{
			return ONVIF_ERR_NO_RECORDING;
		}

		strcpy(p_res->MediaAttributes[i].RecordingToken, recordinfo.RecordingToken);

		p_res->MediaAttributes[i].From = time(NULL);
		p_res->MediaAttributes[i].Until = time(NULL);

		if(recordinfo.TrackNum)
		{
			for(k = 0; k < recordinfo.TrackNum; k++)
			{
				j = p_res->MediaAttributes[i].sizeTrackAttributes;
				
				strcpy(p_res->MediaAttributes[i].TrackAttributes[j].TrackInformation.TrackToken, recordinfo.Tracks[k].TrackToken);
				p_res->MediaAttributes[i].TrackAttributes[j].TrackInformation.TrackType = recordinfo.Tracks[k].Configuration.TrackType;
				strcpy(p_res->MediaAttributes[i].TrackAttributes[j].TrackInformation.Description, recordinfo.Tracks[k].Configuration.Description);
				p_res->MediaAttributes[i].TrackAttributes[j].TrackInformation.DataFrom = time(NULL);
				p_res->MediaAttributes[i].TrackAttributes[j].TrackInformation.DataTo = time(NULL);
				
				// todo : fill video, audio, metadata information ...
				p_res->MediaAttributes[i].sizeTrackAttributes++;
				if (p_res->MediaAttributes[i].sizeTrackAttributes >= 5)
				{
					break;
				}
			}
		}
		
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_FindRecordings(FindRecordings_REQ * p_req, FindRecordings_RES * p_res)
{ 
	LocalSDK__RecordingTimeSearchFilter TimeSearch;
	memset(&TimeSearch,0x0,sizeof(LocalSDK__RecordingTimeSearchFilter));
	if(p_req->MaxMatchesFlag)
	{
		TimeSearch.TotalMaxMatches = p_req->MaxMatches;
	}
	
	sprintf(TimeSearch.DurationTime,"PT%ds",p_req->KeepAliveTime);
	
	if(p_req->Scope.RecordingInformationFilterFlag)
	{
		int i=0;
		TimeSearch.RecordingsNum=p_req->Scope.sizeIncludedRecordings;
		for(i=0;i<TimeSearch.RecordingsNum;i++)
		{
			strcpy(TimeSearch.Recordings[i],(char *)p_req->Scope.IncludedRecordings[i]);
		}
	}
/*
	if(p_req->Scope->Extension)//__anyAttribute carry the filter time (start and end)
	{
		if(p_req->Scope->Extension->__any.size()){
			char *temp=NULL;
			temp=strstr(p_req->Scope->Extension->__any[0],"start:");
			if(temp)
			{
				char *end=NULL;
				end=strstr(temp,"end:");
				if(end)
				{
					strncpy(TimeSearch.StartTime,temp+6,(end-temp-6));
					strcpy(TimeSearch.EndTime,end+4);
				}
			}
		}
	}
	*/
	LocalSDK__SetRecordingTimeFilter(&TimeSearch);
	strncpy(p_res->SearchToken,TimeSearch.TimeSearchToken,sizeof(p_res->SearchToken));
	return ONVIF_OK; 

}

ONVIF_RET my_onvif_GetRecordingSearchResults(GetRecordingSearchResults_REQ * p_req, GetRecordingSearchResults_RES * p_res)
{ 
	LocalSDK__RecordingTimeSearchResults Results;
	memset(&Results,0x0,sizeof(LocalSDK__RecordingTimeSearchResults));
	strcpy(Results.TimeSearchToken,(char *)p_req->SearchToken);
	if(p_req->MinResultsFlag)
	{
		Results.MinResults = p_req->MinResults;
	}
	if(p_req->MaxResultsFlag)
	{
		Results.MaxResults = p_req->MaxResults;
	}

	if(p_req->WaitTimeFlag)
	{
		sprintf(Results.WaitTime,"PT%ds",p_req->WaitTime);
	}
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	SdkResult = LocalSDK__GetRecordingTimeList(&Results);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}	

	if(Results.ResultNum)
	{
		int i=0;
		p_res->ResultList.SearchState=Results.state;
		if(Results.state == VZ__SearchState__Completed)
		{
			LocalSDK__EndSearch_Info Info;
			memset(&Info,0,sizeof(Info));
			strcpy(Info.SearchToken,Results.TimeSearchToken);
			LocalSDK__EndSearch(&Info);
		}	
		for(i=0;i<Results.ResultNum;i++)
		{
			ONVIF_RECINF * p_info = p_res->ResultList.RecordInformation;
			ONVIF_RECINF * p_temp = (ONVIF_RECINF *)get_one_item(ONVIF_TYPE_RECINF);// malloc(sizeof(ONVIF_RECINF));
			if (NULL == p_temp)
			{
				return ONVIF_OK;
			}
			memset(p_temp,0,sizeof(ONVIF_RECINF));
			p_temp->next = NULL;
			
			onvif_RecordingInformation	*PR_Information = &p_temp->RecordingInformation;
			strncpy(PR_Information->RecordingToken,Results.TimeSearchResult[i].RecordingToken,sizeof(PR_Information->RecordingToken));
			strncpy(PR_Information->Content,Results.TimeSearchResult[i].url,sizeof(PR_Information->Content));
			PR_Information->RecordingStatus=RecordingStatus_Stopped;
			strncpy(PR_Information->Source.Location,Results.TimeSearchResult[i].Source.Token,sizeof(PR_Information->Source.Location));
			strncpy(PR_Information->Source.Name,Results.TimeSearchResult[i].Source.Token,sizeof(PR_Information->Source.Name));
			strncpy(PR_Information->Source.Description,Results.TimeSearchResult[i].Source.Name,sizeof(PR_Information->Source.Description));
			strcpy(PR_Information->Source.Address,"");
			strcpy(PR_Information->Source.SourceId,"");
			PR_Information->EarliestRecordingFlag=0;
			PR_Information->LatestRecordingFlag=0;
			
			if(Results.TimeSearchResult[i].Track_num)
			{
				int j=0;
				int index = 0;
				for(j=0;j<Results.TimeSearchResult[i].Track_num&& j< 5;j++)
				{	
					if(Results.TimeSearchResult[i].Track[j].List[0]==1)
					{
						strcpy(PR_Information->Track[index].TrackToken,Results.TimeSearchResult[i].Track[j].TrackToken);
						strcpy(PR_Information->Track[index].Description,Results.TimeSearchResult[i].Track[j].TrackToken);
						/*char *list=NULL;
						soap_new_char(list,strlen(Results.TimeSearchResult[i].Track[j].List)+32);
						sprintf(list,"<tt:timelist>%s</tt:timelist>",Results.TimeSearchResult[i].Track[j].List[2]);
						Track->__any.push_back(list);*/
						PR_Information->Track[index].TrackType=Results.TimeSearchResult[i].Track[j].TrackType;
						PR_Information->Track[index].DataFrom=Results.TimeSearchResult[i].Track[j].DataFrom;
						PR_Information->Track[index].DataTo=Results.TimeSearchResult[i].Track[j].DataTo;
						index++;
					}
				}
				PR_Information->sizeTrack = index;
			}
			if (NULL == p_info)
			{
				p_res->ResultList.RecordInformation = p_temp;
			}
			else
			{
				while (p_info && p_info->next) p_info = p_info->next;
				p_info->next = p_temp;
			}
			
		}
	}
	
	return ONVIF_OK; 

}

ONVIF_RET my_onvif_FindEvents(FindEvents_REQ * p_req, FindEvents_RES * p_res)
{
	return ONVIF_OK;
}
ONVIF_RET my_onvif_GetEventSearchResults(GetEventSearchResults_REQ * p_req, GetEventSearchResults_RES * p_res)
{
	return ONVIF_OK;
}
ONVIF_RET my_onvif_EndSearch(EndSearch_REQ * p_req, EndSearch_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__EndSearch_Info Info;
	memset(&Info,0,sizeof(Info));
	strcpy(Info.SearchToken,p_req->SearchToken);
	SdkResult = LocalSDK__EndSearch(&Info);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	p_res->Endpoint=Info.Endpoint;

	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetSearchState(GetSearchState_REQ * p_req, GetSearchState_RES * p_res)
{
	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetReplayUri(GetReplayUri_REQ * p_req, GetReplayUri_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	char uri[128]={0};
	
	SdkResult = LocalSDK__GetReplayUri(p_req->RecordingToken,uri);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	strcpy(p_res->Uri,uri);
	
	return	ONVIF_OK; 
}

ONVIF_RET my_onvif_GetReplayConfiguration(GetReplayConfiguration_RES * p_res)
{ 
	//char Timeout[8]="PT10S";
	p_res->SessionTimeout = 10;
	return ONVIF_OK; 
}

ONVIF_RET my_onvif_SetReplayConfiguration(SetReplayConfiguration_REQ * p_req)
{
	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetRecordings(GetRecordings_RES * p_res)
{
	Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();
	int i=0;
	for(i=0;i<pOnvifInfo->Recordings.Recording_num;i++)
	{
		ONVIF_RECORDING * p_record = p_res->p_recording;
		ONVIF_RECORDING * p_temp_record = (ONVIF_RECORDING *)get_one_item(ONVIF_TYPE_RECORDING);// malloc(sizeof(ONVIF_RECORDING));
		if (NULL == p_temp_record)
		{
			return ONVIF_OK;
		}
		memset(p_temp_record,0,sizeof(ONVIF_RECORDING));
		p_temp_record->next = NULL;
		onvif_Recording *PRecording = &p_temp_record->Recording;
		vz_onvif_Recording *vz_Record = &pOnvifInfo->Recordings.Recording[i];
		strncpy(PRecording->RecordingToken,vz_Record->RecordingToken,sizeof(PRecording->RecordingToken));
		PRecording->Configuration.MaximumRetentionTimeFlag = vz_Record->Config.MaximumRetentionTimeFlag;
		PRecording->Configuration.MaximumRetentionTime = vz_Record->Config.MaximumRetentionTime;
		strncpy(PRecording->Configuration.Content,vz_Record->Config.Content,sizeof(PRecording->Configuration.Content));
		onvif_RecordingSourceInformation *p_Source = &PRecording->Configuration.Source;
		vz_onvif_RecordingSourceInfo *vz_Source = &vz_Record->Config.Source;
		strncpy(p_Source->SourceId,vz_Source->SourceId,sizeof(p_Source->SourceId));
		strncpy(p_Source->Name,vz_Source->Name,sizeof(p_Source->Name));
		strncpy(p_Source->Location,vz_Source->Location,sizeof(p_Source->Location));
		strncpy(p_Source->Description,vz_Source->Description,sizeof(p_Source->Description));
		strncpy(p_Source->Address,vz_Source->Address,sizeof(p_Source->Address));
		PRecording->Tracks = NULL;
		if(vz_Record->TrackNum)
		{
			int j = 0;
			for(j = 0;j<vz_Record->TrackNum;j++)
			{
				ONVIF_TRACK * p_Track = PRecording->Tracks;
				ONVIF_TRACK * p_temp_Track = (ONVIF_TRACK *)get_one_item(ONVIF_TYPE_TRACK);// malloc(sizeof(ONVIF_TRACK));
				if (NULL == p_temp_Track)
				{
					return ONVIF_OK;
				}
				memset(p_temp_Track,0,sizeof(ONVIF_TRACK));
				p_temp_Track->next = NULL;
				
				onvif_Track *PTrack = &p_temp_Track->Track;
				vz_onvif_Track *vz_Track = &vz_Record->Tracks[j];
				
				strncpy(PTrack->TrackToken,vz_Track->TrackToken,sizeof(PTrack->TrackToken));
				PTrack->Configuration.TrackType = (onvif_TrackType)vz_Track->Configuration.TrackType;
				strncpy(PTrack->Configuration.Description,vz_Track->Configuration.Description,sizeof(PTrack->Configuration.Description));
				if (NULL == p_Track)
				{
					PRecording->Tracks = p_temp_Track;
				}
				else
				{
					while (p_Track && p_Track->next) p_Track = p_Track->next;
					p_Track->next = p_temp_Track;
				}
			}
		}

		if (NULL == p_record)
		{
			p_res->p_recording = p_temp_record;
		}
		else
		{
			while (p_record && p_record->next) p_record = p_record->next;
			p_record->next = p_temp_record;
		}
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetRecordingConfiguration(GetRecordingConfiguration_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	vz_onvif_Recording recordinfo;
	memset(&recordinfo,0,sizeof(recordinfo));
	SdkResult = LocalSDK_find_recording(p_res->RecordingToken,&recordinfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	p_res->Configuration.MaximumRetentionTimeFlag = recordinfo.Config.MaximumRetentionTimeFlag;
	p_res->Configuration.MaximumRetentionTime = recordinfo.Config.MaximumRetentionTime;
	strncpy(p_res->Configuration.Content,recordinfo.Config.Content,sizeof(p_res->Configuration.Content));
	onvif_RecordingSourceInformation *p_Source = &p_res->Configuration.Source;
	vz_onvif_RecordingSourceInfo *vz_Source = &recordinfo.Config.Source;
	strncpy(p_Source->SourceId,vz_Source->SourceId,sizeof(p_Source->SourceId));
	strncpy(p_Source->Name,vz_Source->Name,sizeof(p_Source->Name));
	strncpy(p_Source->Location,vz_Source->Location,sizeof(p_Source->Location));
	strncpy(p_Source->Description,vz_Source->Description,sizeof(p_Source->Description));
	strncpy(p_Source->Address,vz_Source->Address,sizeof(p_Source->Address));

	return ONVIF_OK;
}	

ONVIF_RET my_onvif_GetTrackConfiguration(GetTrackConfiguration_REQ * p_req,GetTrackConfiguration_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	vz_onvif_Track Trackinfo;
	memset(&Trackinfo,0,sizeof(Trackinfo));
	
	SdkResult = LocalSDK_find_track(p_req->RecordingToken,p_req->TrackToken,&Trackinfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	p_res->Configuration.TrackType = Trackinfo.Configuration.TrackType;
	strncpy(p_res->Configuration.Description,Trackinfo.Configuration.Description,sizeof(p_res->Configuration.Description));

	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetRecordingJobs(GetRecordingJobs_RES * p_res)
{
	Onvif_Info* pOnvifInfo = LocalSDK_GetOnvifInfo();
	int i=0;
	for(i=0;i<pOnvifInfo->RecordingJobs.RecordingJob_num;i++)
	{
		ONVIF_RECORDINGJOB * p_JOB = p_res->recording_jobs;
		ONVIF_RECORDINGJOB * p_temp_job = (ONVIF_RECORDINGJOB *)get_one_item(ONVIF_TYPE_RECORDINGJOB);// malloc(sizeof(ONVIF_RECORDINGJOB));
		if (NULL == p_temp_job)
		{
			return ONVIF_OK;
		}
		memset(p_temp_job,0,sizeof(ONVIF_RECORDINGJOB));
		p_temp_job->next = NULL;
		onvif_RecordingJob *PRecordjob = &p_temp_job->RecordingJob;
		vz_onvif_RecordingJob *vz_job = &pOnvifInfo->RecordingJobs.RecordingJob[i];
		
		strncpy(PRecordjob->JobToken,vz_job->RecordingJobToken,sizeof(PRecordjob->JobToken));
		strncpy(PRecordjob->JobConfiguration.RecordingToken,vz_job->RecordingToken,sizeof(PRecordjob->JobConfiguration.RecordingToken));
		strncpy(PRecordjob->JobConfiguration.Mode,vz_job->Mode,sizeof(PRecordjob->JobConfiguration.Mode));
		PRecordjob->JobConfiguration.Priority = vz_job->Priority;
		PRecordjob->JobConfiguration.sizeSource = vz_job->sizeSource;
		if(vz_job->sizeSource)
		{
			int j =0;
			for(j =0;j<vz_job->sizeSource && j < 5;j++)
			{
				onvif_RecordingJobSource *p_source=&PRecordjob->JobConfiguration.Source[j];
				vz_onvif_RecordingJobSource *vz_source=&vz_job->JobSource[j];
				p_source->SourceTokenFlag = vz_source->SourceTokenFlag;
				p_source->AutoCreateReceiverFlag = vz_source->AutoCreateReceiverFlag;
				p_source->AutoCreateReceiver = vz_source->AutoCreateReceiver;
				p_source->sizeTracks = vz_source->sizeTracks;
				if(p_source->SourceTokenFlag)
				{
					p_source->SourceToken.TypeFlag = vz_source->SourceToken.TypeFlag;
					if(p_source->SourceToken.TypeFlag)
						strncpy(p_source->SourceToken.Type, vz_source->SourceToken.Type,sizeof(p_source->SourceToken.Type));
					strncpy(p_source->SourceToken.Token, vz_source->SourceToken.Token,sizeof(p_source->SourceToken.Token));
				}
				if(p_source->sizeTracks)
				{
					int k =0;
					for(k =0;k<p_source->sizeTracks && k < 5;k++)
					{
						strncpy(p_source->Tracks[k].SourceTag, vz_source->Tracks[k].SourceTag,sizeof(p_source->Tracks[k].SourceTag));
						strncpy(p_source->Tracks[k].Destination, vz_source->Tracks[k].Destination,sizeof(p_source->Tracks[k].Destination));
					}
				}
			}
		}

		if (NULL == p_JOB)
		{
			p_res->recording_jobs = p_temp_job;
		}
		else
		{
			while (p_JOB && p_JOB->next) p_JOB = p_JOB->next;
		
			p_JOB->next = p_temp_job;
		}
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetRecordingJobConfiguration(GetRecordingJobConfiguration_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	vz_onvif_RecordingJob Jobinfo;
	memset(&Jobinfo,0,sizeof(Jobinfo));
	
	SdkResult = LocalSDK_find_recordingjob(p_res->JobToken,&Jobinfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	strncpy(p_res->JobConfiguration.RecordingToken,Jobinfo.RecordingToken,sizeof(p_res->JobConfiguration.RecordingToken));
	strncpy(p_res->JobConfiguration.Mode,Jobinfo.Mode,sizeof(p_res->JobConfiguration.Mode));
	p_res->JobConfiguration.Priority = Jobinfo.Priority;
	p_res->JobConfiguration.sizeSource = Jobinfo.sizeSource;
	if(Jobinfo.sizeSource)
	{
		int j =0;
		for(j =0;j<Jobinfo.sizeSource && j < 5;j++)
		{
			onvif_RecordingJobSource *p_source=&p_res->JobConfiguration.Source[j];
			vz_onvif_RecordingJobSource *vz_source=&Jobinfo.JobSource[j];
			p_source->SourceTokenFlag = vz_source->SourceTokenFlag;
			p_source->AutoCreateReceiverFlag = vz_source->AutoCreateReceiverFlag;
			p_source->AutoCreateReceiver = vz_source->AutoCreateReceiver;
			p_source->sizeTracks = vz_source->sizeTracks;
			if(p_source->SourceTokenFlag)
			{
				p_source->SourceToken.TypeFlag = vz_source->SourceToken.TypeFlag;
				if(p_source->SourceToken.TypeFlag)
					strncpy(p_source->SourceToken.Type, vz_source->SourceToken.Type,sizeof(p_source->SourceToken.Type));
				strncpy(p_source->SourceToken.Token, vz_source->SourceToken.Token,sizeof(p_source->SourceToken.Token));
			}
			if(p_source->sizeTracks)
			{
				int k =0;
				for(k =0;k<p_source->sizeTracks && k < 5;k++)
				{
					strncpy(p_source->Tracks[k].SourceTag, vz_source->Tracks[k].SourceTag,sizeof(p_source->Tracks[k].SourceTag));
					strncpy(p_source->Tracks[k].Destination, vz_source->Tracks[k].Destination,sizeof(p_source->Tracks[k].Destination));
				}
			}
		}
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetSupportedRules_ex(GetSupportedRules_REQ * p_req, GetSupportedRules_RES * p_res)
{ 
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int i = 0;
	LocalSDK__SupportedRules SupportedRules;
	memset(&SupportedRules,0x0,sizeof(LocalSDK__SupportedRules));
	
	SdkResult = LocalSDK__GetSupportedRules(p_req->ConfigurationToken,&SupportedRules);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	if(SupportedRules.sizeSchemaLocation)
	{
		for(i=0;i<SupportedRules.sizeSchemaLocation && i < 10;i++)
		{
			strcpy(p_res->SupportedRules.RuleContentSchemaLocation[i],SupportedRules.SchemaLocation[i]);
		}
	}
	p_res->SupportedRules.sizeRuleContentSchemaLocation = SupportedRules.sizeSchemaLocation;
	
	if(SupportedRules.sizeRuleDescription)
	{
		for(i=0;i<SupportedRules.sizeRuleDescription;i++)
		{
			ONVIF_CFG_DESC * p_RuleDescription = p_res->SupportedRules.RuleDescription;
			ONVIF_CFG_DESC * p_temp = (ONVIF_CFG_DESC *)get_one_item(ONVIF_TYPE_CFG_DESC);//malloc(sizeof(ONVIF_CFG_DESC));
			if (NULL == p_temp)
			{
				return ONVIF_OK;
			}
			memset(p_temp,0,sizeof(ONVIF_CFG_DESC));
			p_temp->next = NULL;
			
			onvif_ItemListDescription *Parameters =&p_temp->ConfigDescription.Parameters;
			if((SupportedRules.RuleDescription[i].Parameters.__sizeElementItem != 0) || (SupportedRules.RuleDescription[i].Parameters.__sizeSimpleItem != 0))
			{
				int j =0;
				if(SupportedRules.RuleDescription[i].Parameters.__sizeSimpleItem)
				{
					for(j=0;j<SupportedRules.RuleDescription[i].Parameters.__sizeSimpleItem;j++)
					{
						ONVIF_SIMPLE_ITEM_DESC * p_SimpleItem = Parameters->SimpleItemDescription;
						ONVIF_SIMPLE_ITEM_DESC * p_SimpleItem_temp = (ONVIF_SIMPLE_ITEM_DESC *)get_one_item(ONVIF_TYPE_SITEM_DESC);//malloc(sizeof(ONVIF_SIMPLE_ITEM_DESC));
						if (NULL == p_SimpleItem_temp)
						{
							return ONVIF_OK;
						}
						memset(p_SimpleItem_temp,0,sizeof(ONVIF_SIMPLE_ITEM_DESC));
						p_SimpleItem_temp->next = NULL;
						onvif_SimpleItemDescription *p_SIDes = &p_SimpleItem_temp->SimpleItemDescription;
						strncpy(p_SIDes->Name,SupportedRules.RuleDescription[i].Parameters.SimpleItem[j].Name,sizeof(p_SIDes->Name));
						strncpy(p_SIDes->Type,SupportedRules.RuleDescription[i].Parameters.SimpleItem[j].Value,sizeof(p_SIDes->Type));
						if (NULL == p_SimpleItem)
						{
							Parameters->SimpleItemDescription = p_SimpleItem_temp;
						}
						else
						{
							while (p_SimpleItem && p_SimpleItem->next) p_SimpleItem = p_SimpleItem->next;
						
							p_SimpleItem->next = p_SimpleItem_temp;
						}
					}
				}
				else
				{
					Parameters->SimpleItemDescription = NULL;
				}
				
				if(SupportedRules.RuleDescription[i].Parameters.__sizeElementItem)
				{
					for(j=0;j<SupportedRules.RuleDescription[i].Parameters.__sizeElementItem;j++)
					{
						ONVIF_SIMPLE_ITEM_DESC * p_EleItem = Parameters->ElementItemDescription;
						ONVIF_SIMPLE_ITEM_DESC * p_EleItem_temp = (ONVIF_SIMPLE_ITEM_DESC *)get_one_item(ONVIF_TYPE_SITEM_DESC);//malloc(sizeof(ONVIF_SIMPLE_ITEM_DESC));
						if (NULL == p_EleItem_temp)
						{
							return ONVIF_OK;
						}
						memset(p_EleItem_temp,0,sizeof(ONVIF_SIMPLE_ITEM_DESC));
						p_EleItem_temp->next = NULL;
	
						onvif_SimpleItemDescription *p_EIDes = &p_EleItem_temp->SimpleItemDescription;
						strncpy(p_EIDes->Name,SupportedRules.RuleDescription[i].Parameters.ElementItem[j].Name,sizeof(p_EIDes->Name));
						strncpy(p_EIDes->Type,SupportedRules.RuleDescription[i].Parameters.ElementItem[j].Value,sizeof(p_EIDes->Type));
						//free(SupportedRules.RuleDescription[i].Parameters.ElementItem[j].Value);
						if (NULL == p_EleItem)
						{
							Parameters->ElementItemDescription = p_EleItem_temp;
						}
						else
						{
							while (p_EleItem && p_EleItem->next) p_EleItem = p_EleItem->next;
						
							p_EleItem->next = p_EleItem_temp;
						}
					
					}
				}
				else
				{
					Parameters->ElementItemDescription=NULL;
				}

			}
			else
			{
				Parameters->SimpleItemDescription = NULL;
				Parameters->ElementItemDescription = NULL;
			}
				
			if(SupportedRules.RuleDescription[i].__sizeMessages)
			{
				int j = 0;
				for(j=0;j<SupportedRules.RuleDescription[i].__sizeMessages;j++)
				{
					ONVIF_CFG_DESC_MSG * p_Messages = p_temp->ConfigDescription.Messages;
					ONVIF_CFG_DESC_MSG * p_Mgtemp = (ONVIF_CFG_DESC_MSG *)get_one_item(ONVIF_TYPE_CFGDESC_MSG);//malloc(sizeof(ONVIF_CFG_DESC_MSG));
					if (NULL == p_Mgtemp)
					{
						return ONVIF_OK;
					}
					memset(p_Mgtemp,0,sizeof(ONVIF_CFG_DESC_MSG));
					p_Mgtemp->next = NULL;
					
					onvif_ConfigDescription_Messages *Parameters =&p_Mgtemp->Messages;
					Parameters->SourceFlag = 0;
					Parameters->KeyFlag = 0;
					Parameters->DataFlag = 0;
					Parameters->IsPropertyFlag = 0;
					if (NULL == p_Messages)
					{
						p_temp->ConfigDescription.Messages = p_Mgtemp;
					}
					else
					{
						while (p_Messages && p_Messages->next) p_Messages = p_Messages->next;
					
						p_Messages->next = p_Mgtemp;
					}
				}
			}
			else
			{
				p_temp->ConfigDescription.Messages=NULL;
			}
			strncpy(p_temp->ConfigDescription.Name,SupportedRules.RuleDescription[i].Name,sizeof(p_temp->ConfigDescription.Name));

			if (NULL == p_RuleDescription)
			{
				p_res->SupportedRules.RuleDescription = p_temp;
			}
			else
			{
				while (p_RuleDescription && p_RuleDescription->next) p_RuleDescription = p_RuleDescription->next;
			
				p_RuleDescription->next = p_temp;
			}
		}
	}

	return ONVIF_OK;
}

ONVIF_RET my_onvif_CreateRules_ex(CreateRules_REQ * p_req)
{
	int i = 0;
	int j = 0;
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__OneSetOfRule RulesInfo;
	memset(&RulesInfo,0x0,sizeof(RulesInfo));
	RulesInfo.token = (char *)p_req->ConfigurationToken;
	ONVIF_CONFIG * p_rules = p_req->Rule;
	while(p_rules)
	{
		strcpy(RulesInfo.Rule[i].Name,p_rules->Config.Name); 
		strcpy(RulesInfo.Rule[i].Type,p_rules->Config.Type); 
		//ignor tan__CreateRules->Rule[i]->Type
		ONVIF_SIMPLEITEM	* P_Simple = p_rules->Config.Parameters.SimpleItem;
		ONVIF_ELEMENTITEM	* P_Element = p_rules->Config.Parameters.ElementItem;
		while(P_Simple)
		{
			strcpy(RulesInfo.Rule[i].Parameters.SimpleItem[j].Name,P_Simple->SimpleItem.Name);
			strcpy(RulesInfo.Rule[i].Parameters.SimpleItem[j].Value,P_Simple->SimpleItem.Value);
			P_Simple = P_Simple->next;
			j++;
		}
		RulesInfo.Rule[i].Parameters.__sizeSimpleItem = j;
		j = 0;
		while(P_Element)
		{
			strcpy(RulesInfo.Rule[i].Parameters.ElementItem[j].Name,P_Element->ElementItem.Name);
      int len = strlen(P_Element->ElementItem.Any)+1;
      len = len > VZ_LIST_MAX_NUM ? VZ_LIST_MAX_NUM : len;
			strncpy(RulesInfo.Rule[i].Parameters.ElementItem[j].Value, P_Element->ElementItem.Any,len);
			P_Element = P_Element->next;
			j++;
		}
		RulesInfo.Rule[i].Parameters.__sizeElementItem = j;
		p_rules = p_rules->next;
		i++;
	}
	RulesInfo.sizeRule = i;
	
	SdkResult = LocalSDK__CreateRules(&RulesInfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;
}	

ONVIF_RET my_onvif_DeleteRules_ex(DeleteRules_REQ * p_req)
{
	int i = 0;
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__OneSetOfRule RulesInfo;
	memset(&RulesInfo,0x0,sizeof(RulesInfo));
	RulesInfo.sizeRule = p_req->sizeRuleName;
	RulesInfo.token = p_req->ConfigurationToken;
	for(i=0;i<RulesInfo.sizeRule;i++)
	{
		strcpy(RulesInfo.Rule[i].Name,p_req->RuleName[i]);
	}
	SdkResult = LocalSDK__DeleteRules(&RulesInfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_GetRules_ex(GetRules_REQ * p_req, GetRules_RES * p_res)
{
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	int i =0;
	LocalSDK__OneSetOfRule OneSetOfRules;
	memset(&OneSetOfRules,0x0,sizeof(LocalSDK__OneSetOfRule));
	OneSetOfRules.token = p_req->ConfigurationToken;
	SdkResult = LocalSDK__GetRules(&OneSetOfRules);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	if(OneSetOfRules.sizeRule)
	{
		for(i=0;i<OneSetOfRules.sizeRule;i++)
		{
			ONVIF_CONFIG * p_Rules = p_res->Rule;
			ONVIF_CONFIG * p_temp = (ONVIF_CONFIG *)get_one_item(ONVIF_TYPE_CONFIG);//malloc(sizeof(ONVIF_CONFIG));
			if (NULL == p_temp)
			{
				return ONVIF_OK;
			}
			memset(p_temp,0,sizeof(ONVIF_CONFIG));
			p_temp->next = NULL;
			strncpy(p_temp->Config.Name,OneSetOfRules.Rule[i].Name,sizeof(p_temp->Config.Name));
			strncpy(p_temp->Config.Type,OneSetOfRules.Rule[i].Name,sizeof(p_temp->Config.Type));
			onvif_ItemList	*Parameters = &p_temp->Config.Parameters;
			if((OneSetOfRules.Rule[i].Parameters.__sizeElementItem != 0) || (OneSetOfRules.Rule[i].Parameters.__sizeSimpleItem != 0))
			{
				int j =0;
				if(OneSetOfRules.Rule[i].Parameters.__sizeSimpleItem)
				{
					for(j=0;j<OneSetOfRules.Rule[i].Parameters.__sizeSimpleItem;j++)
					{
						ONVIF_SIMPLEITEM * p_SimpleItem = Parameters->SimpleItem;
						ONVIF_SIMPLEITEM * p_SimpleItem_temp = (ONVIF_SIMPLEITEM *)get_one_item(ONVIF_TYPE_SIMPLEITEM);//malloc(sizeof(ONVIF_SIMPLEITEM));
						if (NULL == p_SimpleItem_temp)
						{
							return ONVIF_OK;
						}
						memset(p_SimpleItem_temp,0,sizeof(ONVIF_SIMPLEITEM));
						p_SimpleItem_temp->next = NULL;
						onvif_SimpleItem *p_SIDes = &p_SimpleItem_temp->SimpleItem;
						strncpy(p_SIDes->Name,OneSetOfRules.Rule[i].Parameters.SimpleItem[j].Name,sizeof(p_SIDes->Name));
						strncpy(p_SIDes->Value,OneSetOfRules.Rule[i].Parameters.SimpleItem[j].Value,sizeof(p_SIDes->Value));
						if (NULL == p_SimpleItem)
						{
							Parameters->SimpleItem = p_SimpleItem_temp;
						}
						else
						{
							while (p_SimpleItem && p_SimpleItem->next) p_SimpleItem = p_SimpleItem->next;
						
							p_SimpleItem->next = p_SimpleItem_temp;
						}
					}
				}
				else
				{
					Parameters->SimpleItem = NULL;
				}
				if(OneSetOfRules.Rule[i].Parameters.__sizeElementItem)
				{
					for(j=0;j<OneSetOfRules.Rule[i].Parameters.__sizeElementItem;j++)
					{
						ONVIF_ELEMENTITEM * p_EleItem = Parameters->ElementItem;
						ONVIF_ELEMENTITEM * p_EleItem_temp = (ONVIF_ELEMENTITEM *)get_one_item(ONVIF_TYPE_ELEMENTITEM);//malloc(sizeof(ONVIF_ELEMENTITEM));
						if (NULL == p_EleItem_temp)
						{
							return ONVIF_OK;
						}
						memset(p_EleItem_temp,0,sizeof(ONVIF_ELEMENTITEM));
						p_EleItem_temp->next = NULL;

						onvif_ElementItem *p_EIDes = &p_EleItem_temp->ElementItem;
						strncpy(p_EIDes->Name,OneSetOfRules.Rule[i].Parameters.ElementItem[j].Name,sizeof(p_EIDes->Name));
						p_EIDes->Any = get_idle_net_buf();//(char *)malloc(strlen(OneSetOfRules.Rule[i].Parameters.ElementItem[j].Value)+1);
						if (NULL == p_EIDes->Any)
						{
							return ONVIF_OK;
						}
						strcpy(p_EIDes->Any,OneSetOfRules.Rule[i].Parameters.ElementItem[j].Value);
						//free(OneSetOfRules.Rule[i].Parameters.ElementItem[j].Value);

						if (NULL == p_EleItem)
						{
							Parameters->ElementItem = p_EleItem_temp;
						}
						else
						{
							while (p_EleItem && p_EleItem->next) p_EleItem = p_EleItem->next;
						
							p_EleItem->next = p_EleItem_temp;
						}
					
					}
				}
				else
				{
					Parameters->ElementItem = NULL;
				}
			}
			else
			{
				Parameters->SimpleItem = NULL;
				Parameters->ElementItem = NULL;
			}
			
			if (NULL == p_Rules)
			{
				p_res->Rule = p_temp;
			}
			else
			{
				while (p_Rules && p_Rules->next) p_Rules = p_Rules->next;
			
				p_Rules->next = p_temp;
			}
		}
	}
	
	return ONVIF_OK;
}

ONVIF_RET my_onvif_ModifyRules_ex(ModifyRules_REQ * p_req)
{
	int i = 0;
	int j = 0;
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__OneSetOfRule RulesInfo;
	memset(&RulesInfo,0x0,sizeof(RulesInfo));
	RulesInfo.token = (char *)p_req->ConfigurationToken;

	ONVIF_CONFIG * p_rules = p_req->Rule;
	while(p_rules)
	{
		strcpy(RulesInfo.Rule[i].Name,p_rules->Config.Name); 
		//ignor tan__CreateRules->Rule[i]->Type
		ONVIF_SIMPLEITEM	* P_Simple = p_rules->Config.Parameters.SimpleItem;
		ONVIF_ELEMENTITEM	* P_Element = p_rules->Config.Parameters.ElementItem;
		while(P_Simple)
		{
			strcpy(RulesInfo.Rule[i].Parameters.SimpleItem[j].Name,P_Simple->SimpleItem.Name);
			strcpy(RulesInfo.Rule[i].Parameters.SimpleItem[j].Value,P_Simple->SimpleItem.Value);
			P_Simple = P_Simple->next;
			j++;
		}
		RulesInfo.Rule[i].Parameters.__sizeSimpleItem = j;
		j = 0;
		while(P_Element)
		{
			strcpy(RulesInfo.Rule[i].Parameters.ElementItem[j].Name,P_Element->ElementItem.Name);
      int len = strlen(P_Element->ElementItem.Any)+1;
      len = len > VZ_LIST_MAX_NUM ? VZ_LIST_MAX_NUM : len;
			strncpy(RulesInfo.Rule[i].Parameters.ElementItem[j].Value, P_Element->ElementItem.Any,len);
			P_Element = P_Element->next;
			j++;
		}
		RulesInfo.Rule[i].Parameters.__sizeElementItem = j;
		p_rules = p_rules->next;
		i++;
	}
	RulesInfo.sizeRule = i;
	
	SdkResult = LocalSDK__ModifyRules(&RulesInfo);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;
}

//set run_mode /ivs_drawmode/preset_num
ONVIF_RET my_onvif_ModifyAnalyticsModules_ex(ModifyAnalyticsModules_REQ * p_req)
{ 
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__Analytics ModifyAnalytics;
	memset(&ModifyAnalytics,0x0,sizeof(LocalSDK__Analytics));
	ModifyAnalytics.token = (char *)p_req->ConfigurationToken;
	ONVIF_CONFIG * p_cfg = p_req->AnalyticsModule;
	if(p_cfg)
	{
		ONVIF_SIMPLEITEM	* Simple = p_cfg->Config.Parameters.SimpleItem;	
		ONVIF_ELEMENTITEM	* Element = p_cfg->Config.Parameters.ElementItem;
		int index =0;
		while(Simple)
		{
			strcpy(ModifyAnalytics.Analytics.Parameters.SimpleItem[index].Name,Simple->SimpleItem.Name);
			strcpy(ModifyAnalytics.Analytics.Parameters.SimpleItem[index].Value,Simple->SimpleItem.Value);
			Simple = Simple->next;
			index++;
		}
		ModifyAnalytics.Analytics.Parameters.__sizeSimpleItem = index;
		index =0;
		while(Element)
		{
			strcpy(ModifyAnalytics.Analytics.Parameters.ElementItem[index].Name,Element->ElementItem.Name);
      int len = strlen(Element->ElementItem.Any)+1;
      len = len > VZ_LIST_MAX_NUM ? VZ_LIST_MAX_NUM : len;
			strncpy(ModifyAnalytics.Analytics.Parameters.ElementItem[index].Value, Element->ElementItem.Any,len);
			Element = Element->next;
			index++;
		}
		ModifyAnalytics.Analytics.Parameters.__sizeElementItem = index;
	}
	SdkResult = LocalSDK__ModifyAnalyticsModules(&ModifyAnalytics);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}
	
	return ONVIF_OK;
}	

//get run_mode /ivs_drawmode/preset_num/EventAbility

ONVIF_RET my_onvif_GetAnalyticsModules_ex(GetAnalyticsModules_REQ * p_req, GetAnalyticsModules_RES * p_res)
{ 
	int SdkResult = VZ_LOCALAPI_SUCCESS;
	LocalSDK__Analytics AnalyticsModule;
	memset(&AnalyticsModule,0x0,sizeof(LocalSDK__Analytics));
	AnalyticsModule.token = p_req->ConfigurationToken;

	SdkResult = LocalSDK__GetAnalyticsModules(&AnalyticsModule);
	if(SdkResult != VZ_LOCALAPI_SUCCESS)
	{
		return LocalSDK_ErrHandle();
	}

	char EventAbility[64]={0};
	char type[8]={0};
	LocalSDK__GetEventAbility(EventAbility,type);
	if((AnalyticsModule.Analytics.Parameters.__sizeElementItem != 0)
		|| (AnalyticsModule.Analytics.Parameters.__sizeSimpleItem != 0)
		||(EventAbility[0] == 1))
	{
		ONVIF_CONFIG * p_cfg = (ONVIF_CONFIG *)get_one_item(ONVIF_TYPE_CONFIG);// malloc(sizeof(ONVIF_CONFIG));
		if (NULL == p_cfg)
		{
			return ONVIF_OK;
		}
		memset(p_cfg,0,sizeof(ONVIF_CONFIG));
		p_cfg->next = NULL;
		p_res->AnalyticsModule = p_cfg;
		onvif_ItemList	*Parameters = &p_cfg->Config.Parameters;
		if(EventAbility[0] == 1)
		{
			ONVIF_ELEMENTITEM * p_EleItem = Parameters->ElementItem;
			ONVIF_ELEMENTITEM * p_EleItem_temp = (ONVIF_ELEMENTITEM *)get_one_item(ONVIF_TYPE_ELEMENTITEM);//malloc(sizeof(ONVIF_ELEMENTITEM));
			if (NULL == p_EleItem_temp)
			{
				return ONVIF_OK;
			}
			memset(p_EleItem_temp,0,sizeof(ONVIF_ELEMENTITEM));
			p_EleItem_temp->next = NULL;
			
			onvif_ElementItem *p_EIDes = &p_EleItem_temp->ElementItem;
	
			strncpy(p_EIDes->Name,"EventAbility",sizeof(p_EIDes->Name));
			p_EIDes->Any = get_idle_net_buf();//(char *)malloc(strlen(EventAbility)+10);//strlen(EventAbility) + type + '#'
			if (NULL == p_EIDes->Any)
			{
				return ONVIF_OK;
			}
			sprintf(p_EIDes->Any,"%s#%s",type,EventAbility[2]);
	
			if (NULL == p_EleItem)
			{
				Parameters->ElementItem = p_EleItem_temp;
			}
			else
			{
				while (p_EleItem && p_EleItem->next) p_EleItem = p_EleItem->next;
			
				p_EleItem->next = p_EleItem_temp;
			}
				
		}
		
		int j =0;
		if(AnalyticsModule.Analytics.Parameters.__sizeElementItem)
		{
			for(j=0;j<AnalyticsModule.Analytics.Parameters.__sizeElementItem;j++)
			{
				ONVIF_ELEMENTITEM * p_EleItem = Parameters->ElementItem;
				ONVIF_ELEMENTITEM * p_EleItem_temp = (ONVIF_ELEMENTITEM *)get_one_item(ONVIF_TYPE_ELEMENTITEM);//malloc(sizeof(ONVIF_ELEMENTITEM));
				if (NULL == p_EleItem_temp)
				{
					return ONVIF_OK;
				}
				memset(p_EleItem_temp,0,sizeof(ONVIF_ELEMENTITEM));
				p_EleItem_temp->next = NULL;
				
				onvif_ElementItem *p_EIDes = &p_EleItem_temp->ElementItem;

				strncpy(p_EIDes->Name,AnalyticsModule.Analytics.Parameters.ElementItem[j].Name,sizeof(p_EIDes->Name));
				if(AnalyticsModule.Analytics.Parameters.ElementItem[j].Value){
					p_EIDes->Any = get_idle_net_buf();//(char *)malloc(strlen(AnalyticsModule.Analytics.Parameters.ElementItem[j].Value)+1);
					if (NULL == p_EIDes->Any)
					{
						return ONVIF_OK;
					}
					strcpy(p_EIDes->Any,AnalyticsModule.Analytics.Parameters.ElementItem[j].Value);
					//free(AnalyticsModule.Analytics.Parameters.ElementItem[j].Value);
				}

				if (NULL == p_EleItem)
				{
					Parameters->ElementItem = p_EleItem_temp;
				}
				else
				{
					while (p_EleItem && p_EleItem->next) p_EleItem = p_EleItem->next;
				
					p_EleItem->next = p_EleItem_temp;
				}
					
			}
		}
		else
		{
			Parameters->ElementItem = NULL;
		}
		
		if(AnalyticsModule.Analytics.Parameters.__sizeSimpleItem)
		{
			for(j=0;j<AnalyticsModule.Analytics.Parameters.__sizeSimpleItem;j++)
			{
				ONVIF_SIMPLEITEM * p_SimpleItem = Parameters->SimpleItem;
				ONVIF_SIMPLEITEM * p_SimpleItem_temp = (ONVIF_SIMPLEITEM *)get_one_item(ONVIF_TYPE_SIMPLEITEM);//malloc(sizeof(ONVIF_SIMPLEITEM));
				if (NULL == p_SimpleItem_temp)
				{
					return ONVIF_OK;
				}
				memset(p_SimpleItem_temp,0,sizeof(ONVIF_SIMPLEITEM));
				p_SimpleItem_temp->next = NULL;
				onvif_SimpleItem *p_SIDes = &p_SimpleItem_temp->SimpleItem;
				strncpy(p_SIDes->Name,AnalyticsModule.Analytics.Parameters.SimpleItem[j].Name,sizeof(p_SIDes->Name));
				strncpy(p_SIDes->Value,AnalyticsModule.Analytics.Parameters.SimpleItem[j].Value,sizeof(p_SIDes->Value));
				if (NULL == p_SimpleItem)
				{
					Parameters->SimpleItem = p_SimpleItem_temp;
				}
				else
				{
					while (p_SimpleItem && p_SimpleItem->next) p_SimpleItem = p_SimpleItem->next;
				
					p_SimpleItem->next = p_SimpleItem_temp;
				}
			}
		}
		else
		{
			Parameters->SimpleItem =NULL;
		}
	}
	else
	{
		 p_res->AnalyticsModule =NULL;
	}

	return ONVIF_OK;
}







