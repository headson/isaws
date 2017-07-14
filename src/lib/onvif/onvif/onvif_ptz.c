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
#include "onvif/onvif/onvif_ptz.h"
#include "onvif/onvif/onvif_util.h"

/***************************************************************************************/
extern ONVIF_CLS g_onvif_cls;

/***************************************************************************************/
ONVIF_RET onvif_PTZGetStatus(ONVIF_PROFILE * p_profile, onvif_PTZStatus * p_ptz_status)
{
	// todo : add get ptz status code ...
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

ONVIF_RET onvif_ContinuousMove(ContinuousMove_REQ * p_req)
{
	// todo : add continuous move code ... 
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_ContinuousMove(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
    return ONVIF_OK;
}

ONVIF_RET onvif_PTZ_Stop(PTZ_Stop_REQ * p_req)
{
	// todo : add stop PTZ moving code ... 
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_PTZ_Stop(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
    return ONVIF_OK;
}

ONVIF_RET onvif_AbsoluteMove(AbsoluteMove_REQ * p_req)
{	
	// todo : add absolute move code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_AbsoluteMove(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
    return ONVIF_OK;
}

ONVIF_RET onvif_RelativeMove(RelativeMove_REQ * p_req)
{
	// todo : add relative move code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_RelativeMove(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
    return ONVIF_OK;
}

ONVIF_RET onvif_SetPreset(SetPreset_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetPreset(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}

    
    return ONVIF_OK;
}

ONVIF_RET onvif_RemovePreset(RemovePreset_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_RemovePreset(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
    return ONVIF_OK;
}

ONVIF_RET onvif_GotoPreset(GotoPreset_REQ * p_req)
{	
    // todo : add goto preset code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_GotoPreset(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
    return ONVIF_OK;
}

ONVIF_RET onvif_GotoHomePosition(GotoHomePosition_REQ * p_req)
{
    // todo : add goto home position code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_GotoHomePosition(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
    return ONVIF_OK;
}

ONVIF_RET onvif_SetHomePosition(const char * token)
{
    // todo : add set home position code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetHomePosition(token);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
    return ONVIF_OK;
}

ONVIF_RET onvif_SetConfiguration(SetConfiguration_REQ * p_req)
{
	// todo : add set ptz configuration code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetPTZConfiguration(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
    return ONVIF_OK;
}



