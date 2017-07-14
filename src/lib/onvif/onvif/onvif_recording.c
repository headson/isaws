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

#include "onvif/onvif/onvif.h"
#include "onvif/onvif/onvif_recording.h"
#include "onvif/onvif/xml_node.h"
#include "onvif/onvif/onvif_util.h"
#include "onvif/onvif/onvif_event.h"

#ifdef PROFILE_G_SUPPORT

/***************************************************************************************/
extern ONVIF_CLS g_onvif_cls;


/***************************************************************************************/

/**
 The possible return values:
 	ONVIF_ERR_BAD_CONFIGURATION,
	ONVIF_ERR_MAX_RECORDING,
 */
ONVIF_RET onvif_CreateRecording(CreateRecording_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_CreateRecording(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return values:
 	ONVIF_ERR_NO_RECORDING,
	ONVIF_ERR_CANNOT_DELETE,
 */
ONVIF_RET onvif_DeleteRecording(const char * p_RecordingToken)
{
	// todo : add delete recording code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_DeleteRecording(p_RecordingToken);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}

	return ONVIF_OK;
}

/**
 The possible return values:
 	ONVIF_ERR_BAD_CONFIGURATION,
	ONVIF_ERR_NO_RECORDING,
 */
ONVIF_RET onvif_SetRecordingConfiguration(SetRecordingConfiguration_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetRecordingConfiguration(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return values:
 	ONVIF_ERR_BAD_CONFIGURATION,
	ONVIF_ERR_NO_RECORDING,
	ONVIF_ERR_MAX_TRACKS
 */
ONVIF_RET onvif_CreateTrack(CreateTrack_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_CreateTrack(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return values:
	ONVIF_ERR_NO_RECORDING,
	ONVIF_ERR_NO_TRACK,
	ONVIF_ERR_CANNOT_DELETE
 */
ONVIF_RET onvif_DeleteTrack(DeleteTrack_REQ * p_req)
{
	// todo : add delete track code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_DeleteTrack(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return values:
	ONVIF_ERR_NO_RECORDING,
	ONVIF_ERR_NO_TRACK,
	ONVIF_ERR_BAD_CONFIGURATION
 */
ONVIF_RET onvif_SetTrackConfiguration(SetTrackConfiguration_REQ * p_req)
{
	// todo : add set track configuration code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetTrackConfiguration(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return values:
	ONVIF_ERR_MAX_RECORDING_JOBS,
	ONVIF_ERR_BAD_CONFIGURATION,
	ONVIF_ERR_MAX_RECEIVERS
	ONVIF_ERR_NO_RECORDING
 */
ONVIF_RET onvif_CreateRecordingJob(CreateRecordingJob_REQ  * p_req)
{
	// auto create recording source
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_CreateRecordingJob(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return values:
	ONVIF_ERR_NO_RECORDINGJOB
 */
ONVIF_RET onvif_DeleteRecordingJob(const char * p_JobToken)
{
	// todo : add delete recording job code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_DeleteRecordingJob(p_JobToken);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return values:
	ONVIF_ERR_NO_RECORDINGJOB
	ONVIF_ERR_BAD_CONFIGURATION
	ONVIF_ERR_MAX_RECEIVERS
 */
ONVIF_RET onvif_SetRecordingJobConfiguration(SetRecordingJobConfiguration_REQ * p_req)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_CreateRecordingJob(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return values:
	ONVIF_ERR_NO_RECORDINGJOB
	ONVIF_ERR_BAD_MODE
 */
ONVIF_RET onvif_SetRecordingJobMode(SetRecordingJobMode_REQ * p_req)
{
	if (strcmp(p_req->Mode, "Idle") && strcmp(p_req->Mode, "Active"))
	{
		return ONVIF_ERR_BAD_MODE;
	}

	// todo : add set recording job mode code ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_SetRecordingJobMode(p_req);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return values:
	ONVIF_ERR_NO_RECORDINGJOB
 */
ONVIF_RET onvif_GetRecordingJobState(const char * p_JobToken, onvif_RecordingJobStateInformation * p_res)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_GetRecordingJobState(p_JobToken,p_res);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return values:
	ONVIF_ERR_NO_RECORDING
 */
ONVIF_RET onvif_GetRecordingOptions(const char * p_RecordingToken, onvif_RecordingOptions * p_res)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_GetRecordingOptions(p_RecordingToken,p_res);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_GetRecordingSummary(GetRecordingSummary_RES * p_summary)
{
	// todo : modify the information ...
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_GetRecordingSummary(p_summary);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}

	return ONVIF_OK;
}

/**
 The possible return values:
	ONVIF_ERR_INVALID_TOKEN
 */
ONVIF_RET onvif_GetRecordingInformation(const char * p_RecordingToken, GetRecordingInformation_RES * p_res)
{

	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_GetRecordingInformation(p_RecordingToken,p_res);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

/**
 The possible return values:
	ONVIF_ERR_INVALID_TOKEN
	ONVIF_ERR_NO_RECORDING
 */
ONVIF_RET onvif_GetMediaAttributes(GetMediaAttributes_REQ * p_req, GetMediaAttributes_RES * p_res)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_GetMediaAttributes(p_req,p_res);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_FindRecordings(FindRecordings_REQ * p_req, FindRecordings_RES * p_res)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_FindRecordings(p_req,p_res);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	return ONVIF_OK;
}

ONVIF_RET onvif_GetRecordingSearchResults(GetRecordingSearchResults_REQ * p_req, GetRecordingSearchResults_RES * p_res)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_GetRecordingSearchResults(p_req,p_res);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	return ONVIF_OK;
}

ONVIF_RET onvif_FindEvents(FindEvents_REQ * p_req, FindEvents_RES * p_res)
{
	return ONVIF_OK;
}

ONVIF_RET onvif_GetEventSearchResults(GetEventSearchResults_REQ * p_req, GetEventSearchResults_RES * p_res)
{
	return ONVIF_OK;
}

ONVIF_RET onvif_EndSearch(EndSearch_REQ * p_req, EndSearch_RES * p_res)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_EndSearch(p_req,p_res);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	return ONVIF_OK;
}

ONVIF_RET onvif_GetSearchState(GetSearchState_REQ * p_req, GetSearchState_RES * p_res)
{
	return ONVIF_OK;
}

ONVIF_RET onvif_GetReplayUri(GetReplayUri_REQ * p_req, GetReplayUri_RES * p_res)
{
	int SdkResult = ONVIF_OK;
	SdkResult = my_onvif_GetReplayUri(p_req,p_res);
	if(SdkResult != ONVIF_OK)
	{
		return SdkResult;
	}
	
	return ONVIF_OK;
}

ONVIF_RET onvif_GetReplayConfiguration(GetReplayConfiguration_RES * p_res)
{
	p_res->SessionTimeout = 60;

	return ONVIF_OK;
}

ONVIF_RET onvif_SetReplayConfiguration(SetReplayConfiguration_REQ * p_req)
{
	return ONVIF_OK;
}


#endif // end of PROFILE_G_SUPPORT


