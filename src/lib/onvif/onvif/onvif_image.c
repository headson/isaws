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
#include "onvif/onvif/onvif_image.h"

/***************************************************************************************/


/***************************************************************************************/
ONVIF_RET onvif_SetImagingSettings(SetImagingSettings_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetImagingSettings(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}


ONVIF_RET onvif_Move(Move_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_Imaging_Move(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_ImagingGetStatus(ONVIF_V_SRC * p_v_src, onvif_ImagingStatus * p_status)
{
	// todo : add get imaging status code ...


	p_status->FocusStatusFlag = 1;
	p_status->FocusStatus.Position = 0.0;
	p_status->FocusStatus.MoveStatus = MoveStatus_IDLE;

	return ONVIF_OK;
}




