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
#include "onvif/onvif/onvif_media.h"


/***************************************************************************************/
extern ONVIF_CLS g_onvif_cls;


/***************************************************************************************/
ONVIF_RET onvif_CreateProfile(CreateProfile_REQ * p_req)
{
	return ONVIF_OK;
}

ONVIF_RET onvif_DeleteProfile(const char * token)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_DeleteProfile(token);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_AddVideoSourceConfiguration(AddVideoSourceConfiguration_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_AddVideoSourceConfiguration(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}


ONVIF_RET onvif_AddVideoEncoderConfiguration(AddVideoEncoderConfiguration_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_AddVideoEncoderConfiguration(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}


ONVIF_RET onvif_AddAudioSourceConfiguration(AddAudioSourceConfiguration_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_AddAudioSourceConfiguration(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}


ONVIF_RET onvif_AddAudioEncoderConfiguration(AddAudioEncoderConfiguration_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_AddAudioEncoderConfiguration(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}


ONVIF_RET onvif_RemoveVideoEncoderConfiguration(const char * token)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_RemoveVideoEncoderConfiguration(token);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}


ONVIF_RET onvif_RemoveVideoSourceConfiguration(const char * token)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_RemoveVideoSourceConfiguration(token);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}



ONVIF_RET onvif_RemoveAudioEncoderConfiguration(const char * token)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_RemoveAudioEncoderConfiguration(token);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}


ONVIF_RET onvif_RemoveAudioSourceConfiguration(const char * token)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_RemoveAudioSourceConfiguration(token);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}


ONVIF_RET onvif_AddPTZConfiguration(AddPTZConfiguration_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_AddPTZConfiguration(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}


ONVIF_RET onvif_RemovePTZConfiguration(const char * token)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_RemovePTZConfiguration(token);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}


/************************************************************************************
 *  	
 * Possible error:
 * 	ONVIF_ERR_CONFIG_MODIFY
 *	ONVIF_ERR_NO_CONFIG
 * 	ONVIF_ERR_CONFIGURATION_CONFLICT
 *
*************************************************************************************/
ONVIF_RET onvif_SetVideoEncoderConfiguration(SetVideoEncoderConfiguration_REQ * p_req)
{
	// todo : add set video encoder code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetVideoEncoderConfiguration(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/************************************************************************************
 *  	
 * Possible error:
 * 	ONVIF_ERR_CONFIG_MODIFY
 *	ONVIF_ERR_NO_CONFIG
 * 	ONVIF_ERR_CONFIGURATION_CONFLICT
 *
*************************************************************************************/
ONVIF_RET onvif_SetVideoSourceConfiguration(SetVideoSourceConfiguration_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetVideoSourceConfiguration(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}

	return ONVIF_OK;
}


ONVIF_RET onvif_SetAudioSourceConfiguration(SetAudioSourceConfiguration_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetAudioSourceConfiguration(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}

	return ONVIF_OK;
}

ONVIF_RET onvif_SetAudioEncoderConfiguration(SetAudioEncoderConfiguration_REQ * p_req)
{
	// todo : add set audio encoder code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetAudioEncoderConfiguration(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 *
 * Get snapshot JPEG image data, 
 * rlen [in, out], [in] the buff size, [out] the image data size
 *
**/
ONVIF_RET onvif_GetSnapshot(char * buff, int * rlen, char * profile_token)
{
    int len;
    FILE * fp;
    ONVIF_PROFILE * p_profile;
    
    printf("onvif_GetSnapshot\r\n");

    // todo : here is the test code, just read the image data from file ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_GetSnapshot(buff,rlen,profile_token);
	return SdkResult;
	/*
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	

    fp = fopen("snapshot.jpg", "rb");
	if (NULL == fp)
	{
		return ONVIF_ERR_SERVICE_NOT_SUPPORT;
	}
	
	fseek(fp, 0, SEEK_END);
	
	len = ftell(fp);
	if (len <= 0)
	{
		fclose(fp);
		return ONVIF_ERR_SERVICE_NOT_SUPPORT;
	}
	fseek(fp, 0, SEEK_SET);
	
	if (len > *rlen)
	{
	    fclose(fp);
		return ONVIF_ERR_SERVICE_NOT_SUPPORT;
	}

	len = fread(buff, 1, len, fp);
	
	fclose(fp);

	*rlen = len;
    
    return ONVIF_OK;
    */
}

ONVIF_RET onvif_SetOSD(SetOSD_REQ * p_req)
{
	// todo : add set osd code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetOSD(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_CreateOSD(CreateOSD_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_CreateOSD(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_DeleteOSD(DeleteOSD_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_DeleteOSD(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_StartMulticastStreaming(const char * token)
{
	// todo : start multicast streaming ...
	
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_StartMulticastStreaming(token);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}

	return ONVIF_OK;
}

ONVIF_RET onvif_StopMulticastStreaming(const char * token)
{
	// todo : stop multicast streaming ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_StopMulticastStreaming(token);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_SetMetadataConfiguration(SetMetadataConfiguration_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetMetadataConfiguration(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_AddMetadataConfiguration(AddMetadataConfiguration_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_AddMetadataConfiguration(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}

	return ONVIF_OK;
}

ONVIF_RET onvif_RemoveMetadataConfiguration(const char * profile_token)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_RemoveMetadataConfiguration(profile_token);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}

	return ONVIF_OK;
}




