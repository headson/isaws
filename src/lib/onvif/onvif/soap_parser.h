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

#ifndef _SOAP_PARSER_H_
#define _SOAP_PARSER_H_

/***************************************************************************************/
#include "onvif/onvif/xml_node.h"
#include "onvif/onvif/onvif.h"
#include "onvif/onvif/onvif_ptz.h"
#include "onvif/onvif/onvif_device.h"
#include "onvif/onvif/onvif_media.h"
#include "onvif/onvif/onvif_event.h"
#include "onvif/onvif/onvif_image.h"
#ifdef VIDEO_ANALYTICS
#include "onvif/onvif/onvif_analytics.h"
#endif
#ifdef PROFILE_G_SUPPORT
#include "onvif/onvif/onvif_recording.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif

/***************************************************************************************/
BOOL parse_Bool(const char * pdata);

ONVIF_RET parse_AddScopes(XMLN * p_AddScopes, ONVIF_SCOPE * p_scope, int scope_max);
ONVIF_RET parse_SetScopes(XMLN * p_AddScopes, ONVIF_SCOPE * p_scope, int scope_max);
ONVIF_RET parse_SetDiscoveryMode(XMLN * p_SetDiscoveryMode, SetDiscoveryMode_REQ * p_req);
ONVIF_RET parse_Subscribe(XMLN * p_node, Subscribe_REQ * p_req);
ONVIF_RET parse_Renew(XMLN * p_node, Renew_REQ * p_req);
ONVIF_RET parse_CreatePullPointSubscription(XMLN * p_node, CreatePullPointSubscription_REQ * p_req);
ONVIF_RET parse_PullMessages(XMLN * p_node, PullMessages_REQ * p_req);
ONVIF_RET parse_ContinuousMove(XMLN * p_node, ContinuousMove_REQ * p_req);
ONVIF_RET parse_PTZ_Stop(XMLN * p_node, PTZ_Stop_REQ * p_req);
ONVIF_RET parse_AbsoluteMove(XMLN * p_node, AbsoluteMove_REQ * p_req);
ONVIF_RET parse_RelativeMove(XMLN * p_node, RelativeMove_REQ * p_req);
ONVIF_RET parse_SetPreset(XMLN * p_node, SetPreset_REQ * p_req);
ONVIF_RET parse_RemovePreset(XMLN * p_node, RemovePreset_REQ * p_req);
ONVIF_RET parse_GotoPreset(XMLN * p_node, GotoPreset_REQ * p_req);
ONVIF_RET parse_GotoHomePosition(XMLN * p_node, GotoHomePosition_REQ * p_req);
ONVIF_RET parse_SetDNS(XMLN * p_node, SetDNS_REQ * p_req);
ONVIF_RET parse_SetNTP(XMLN * p_node, SetNTP_REQ * p_req);
ONVIF_RET parse_SetNetworkProtocols(XMLN * p_node, SetNetworkProtocols_REQ * p_req);
ONVIF_RET parse_SetNetworkDefaultGateway(XMLN * p_node, SetNetworkDefaultGateway_REQ * p_req);
ONVIF_RET parse_SetSystemDateAndTime(XMLN * p_node, SetSystemDateAndTime_REQ * p_req);
ONVIF_RET parse_CreateProfile(XMLN * p_node, CreateProfile_REQ * p_req);
ONVIF_RET parse_AddVideoSourceConfiguration(XMLN * p_node, AddVideoSourceConfiguration_REQ * p_req);
ONVIF_RET parse_AddVideoEncoderConfiguration(XMLN * p_node, AddVideoEncoderConfiguration_REQ * p_req);
ONVIF_RET parse_AddAudioSourceConfiguration(XMLN * p_node, AddAudioSourceConfiguration_REQ * p_req);
ONVIF_RET parse_AddAudioEncoderConfiguration(XMLN * p_node, AddAudioEncoderConfiguration_REQ * p_req);
ONVIF_RET parse_AddPTZConfiguration(XMLN * p_node, AddPTZConfiguration_REQ * p_req);
ONVIF_RET parse_GetStreamUri(XMLN * p_node, GetStreamUri_REQ * p_req);
ONVIF_RET parse_SetNetworkInterfaces(XMLN * p_node, SetNetworkInterfaces_REQ * p_req);
ONVIF_RET parse_GetVideoSourceConfigurationOptions(XMLN * p_node, GetVideoSourceConfigurationOptions_REQ * p_req);
ONVIF_RET parse_SetVideoSourceConfiguration(XMLN * p_node, SetVideoSourceConfiguration_REQ * p_req);
ONVIF_RET parse_GetVideoEncoderConfigurationOptions(XMLN * p_node, GetVideoEncoderConfigurationOptions_REQ * p_req);
ONVIF_RET parse_SetVideoEncoderConfiguration(XMLN * p_SetVideoEncoderConfiguration, SetVideoEncoderConfiguration_REQ * p_req);
ONVIF_RET parse_GetAudioSourceConfigurationOptions(XMLN * p_node, GetAudioSourceConfigurationOptions_REQ * p_req);
ONVIF_RET parse_SetAudioSourceConfiguration(XMLN * p_node, SetAudioSourceConfiguration_REQ * p_req);
ONVIF_RET parse_GetAudioEncoderConfigurationOptions(XMLN * p_node, GetAudioEncoderConfigurationOptions_REQ * p_req);
ONVIF_RET parse_SetAudioEncoderConfiguration(XMLN * p_node, SetAudioEncoderConfiguration_REQ * p_req);
ONVIF_RET parse_SetImagingSettings(XMLN * p_node, SetImagingSettings_REQ * p_req);
ONVIF_RET parse_Move(XMLN * p_node, Move_REQ * p_req);
ONVIF_RET parse_CreateUsers(XMLN * p_AddScopes, ONVIF_USER * p_user, int user_max);
ONVIF_RET parse_DeleteUsers(XMLN * p_node, ONVIF_USER * p_user, int user_max);
ONVIF_RET parse_GetOSDs(XMLN * p_node, GetOSDs_REQ * p_req);
ONVIF_RET parse_GetOSD(XMLN * p_node, GetOSD_REQ * p_req);
ONVIF_RET parse_SetOSD(XMLN * p_node, SetOSD_REQ * p_req);
ONVIF_RET parse_CreateOSD(XMLN * p_node, CreateOSD_REQ * p_req);
ONVIF_RET parse_DeleteOSD(XMLN * p_node, DeleteOSD_REQ * p_req);
ONVIF_RET parse_SetConfiguration(XMLN * p_node, SetConfiguration_REQ * p_req);
ONVIF_RET parse_GetMetadataConfigurationOptions(XMLN * p_node, GetMetadataConfigurationOptions_REQ * p_req);
ONVIF_RET parse_SetMetadataConfiguration(XMLN * p_node, SetMetadataConfiguration_REQ * p_req);
ONVIF_RET parse_AddMetadataConfiguration(XMLN * p_node, AddMetadataConfiguration_REQ * p_req);

#ifdef PROFILE_G_SUPPORT

ONVIF_RET parse_CreateRecording(XMLN * p_node, CreateRecording_REQ * p_req);
ONVIF_RET parse_SetRecordingConfiguration(XMLN * p_node, SetRecordingConfiguration_REQ * p_req);
ONVIF_RET parse_CreateTrack(XMLN * p_node, CreateTrack_REQ * p_req);
ONVIF_RET parse_DeleteTrack(XMLN * p_node, DeleteTrack_REQ * p_req);
ONVIF_RET parse_GetTrackConfiguration(XMLN * p_node, GetTrackConfiguration_REQ * p_req);
ONVIF_RET parse_SetTrackConfiguration(XMLN * p_node, SetTrackConfiguration_REQ * p_req);
ONVIF_RET parse_CreateRecordingJob(XMLN * p_node, CreateRecordingJob_REQ * p_req);
ONVIF_RET parse_SetRecordingJobConfiguration(XMLN * p_node, SetRecordingJobConfiguration_REQ * p_req);
ONVIF_RET parse_SetRecordingJobMode(XMLN * p_node, SetRecordingJobMode_REQ * p_req);
ONVIF_RET parse_GetMediaAttributes(XMLN * p_node, GetMediaAttributes_REQ * p_req);
ONVIF_RET parse_FindRecordings(XMLN * p_node, FindRecordings_REQ * p_req);
ONVIF_RET parse_GetRecordingSearchResults(XMLN * p_node, GetRecordingSearchResults_REQ * p_req);
ONVIF_RET parse_FindEvents(XMLN * p_node, FindEvents_REQ * p_req);
ONVIF_RET parse_GetEventSearchResults(XMLN * p_node, GetEventSearchResults_REQ * p_req);
ONVIF_RET parse_EndSearch(XMLN * p_node, EndSearch_REQ * p_req);
ONVIF_RET parse_GetSearchState(XMLN * p_node, GetSearchState_REQ * p_req);
ONVIF_RET parse_GetReplayUri(XMLN * p_node, GetReplayUri_REQ * p_req);
ONVIF_RET parse_SetReplayConfiguration(XMLN * p_node, SetReplayConfiguration_REQ * p_req);

#endif	// end of PROFILE_G_SUPPORT

#ifdef VIDEO_ANALYTICS

ONVIF_RET parse_GetSupportedRules(XMLN * p_node, GetSupportedRules_REQ * p_req);
ONVIF_RET parse_CreateRules(XMLN * p_node, CreateRules_REQ * p_req);
ONVIF_RET parse_DeleteRules(XMLN * p_node, DeleteRules_REQ * p_req);
ONVIF_RET parse_GetRules(XMLN * p_node, GetRules_REQ * p_req);
ONVIF_RET parse_ModifyRules(XMLN * p_node, ModifyRules_REQ * p_req);
ONVIF_RET parse_CreateAnalyticsModules(XMLN * p_node, CreateAnalyticsModules_REQ * p_req);
ONVIF_RET parse_DeleteAnalyticsModules(XMLN * p_node, DeleteAnalyticsModules_REQ * p_req);
ONVIF_RET parse_GetAnalyticsModules(XMLN * p_node, GetAnalyticsModules_REQ * p_req);
ONVIF_RET parse_ModifyAnalyticsModules(XMLN * p_node, ModifyAnalyticsModules_REQ * p_req);

ONVIF_RET parse_AddVideoAnalyticsConfiguration(XMLN * p_node, AddVideoAnalyticsConfiguration_REQ * p_req);
ONVIF_RET parse_GetVideoAnalyticsConfiguration(XMLN * p_node, GetVideoAnalyticsConfiguration_REQ * p_req);
ONVIF_RET parse_RemoveVideoAnalyticsConfiguration(XMLN * p_node, RemoveVideoAnalyticsConfiguration_REQ * p_req);
ONVIF_RET parse_SetVideoAnalyticsConfiguration(XMLN * p_node, SetVideoAnalyticsConfiguration_REQ * p_req);

#endif	// end of VIDEO_ANALYTICS


#ifdef __cplusplus
}
#endif

#endif


