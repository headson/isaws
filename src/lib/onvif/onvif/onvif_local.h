#ifndef VZONVIF_LOCAL_H
#define VZONVIF_LOCAL_H

#include "onvif/onvif/onvif_device.h"
#include "onvif/onvif/onvif_media.h"
#include "onvif/onvif/onvif_analytics.h"
#include "onvif/onvif/onvif_event.h"
#include "onvif/onvif/onvif_ptz.h"
#include "onvif/onvif/onvif_image.h"
#include "onvif/onvif/onvif_recording.h"
#include "onvif/onvif/onvif_local_datastruct.h"
#ifdef __cplusplus
extern "C" {
#endif

ONVIF_RET my_onvif_SetSystemDateAndTime(SetSystemDateAndTime_REQ * p_req);
ONVIF_RET my_onvif_SetHostname(const char * name);
ONVIF_RET my_onvif_SetDNS(SetDNS_REQ * p_req);
ONVIF_RET my_onvif_SetNTP(SetNTP_REQ * p_req);
ONVIF_RET my_onvif_SetNetworkProtocols(SetNetworkProtocols_REQ * p_req);
ONVIF_RET my_onvif_SetNetworkDefaultGateway(SetNetworkDefaultGateway_REQ * p_req);
ONVIF_RET my_onvif_SystemReboot();
ONVIF_RET my_onvif_SetSystemFactoryDefault(int type /* 0:soft, 1:hard */);
ONVIF_RET my_onvif_SetNetworkInterfaces(SetNetworkInterfaces_REQ * p_req);
ONVIF_RET my_onvif_SetDiscoveryMode(SetDiscoveryMode_REQ * p_req);
ONVIF_RET my_onvif_StartFirmwareUpgrade(StartFirmwareUpgrade_RES * p_res);	
ONVIF_RET my_onvif_FirmwareUpgradeCheck(const char * buff, int len);
ONVIF_RET my_onvif_FirmwareUpgrade(const char * buff, int len);
ONVIF_RET my_onvif_GetSystemDateAndTime(GetSystemDateAndTime_RES * p_res);
ONVIF_RET my_onvif_GetHostname(GetHostname_RES * p_res);
ONVIF_RET my_onvif_GetDNS(GetDNS_RES * p_res);
ONVIF_RET my_onvif_GetNTP(GetNTP_RES * p_res);
ONVIF_RET my_onvif_GetNetworkProtocols(GetNetworkProtocols_RES * p_res);
ONVIF_RET my_onvif_GetNetworkDefaultGateway(GetNetworkDefaultGateway_RES * p_res);
ONVIF_RET my_onvif_GetNetworkInterfaces(GetNetworkInterfaces_RES * p_res);
ONVIF_RET my_onvif_GetDiscoveryMode(GetDiscoveryMode_RES * p_res);
ONVIF_RET my_onvif_GetDeviceInformation(GetDeviceInformation_RES * p_res);
ONVIF_RET my_onvif_GetCapabilities(GetCapabilities_RES * p_res);
ONVIF_RET my_onvif_GetServices(GetServices_RES * p_res);
ONVIF_RET my_onvif_GetStreamUri(GetStreamUri_REQ *p_req,GetStreamUri_RES * p_res);
ONVIF_RET my_onvif_GetScopes(GetScopes_RES * p_res);
ONVIF_RET my_onvif_add_scopes(ONVIF_SCOPE * p_scope, int scope_max);
ONVIF_RET my_onvif_set_scopes(ONVIF_SCOPE * p_scope, int scope_max);
ONVIF_RET my_onvif_remove_scopes(ONVIF_SCOPE * p_scope, int scope_max);
ONVIF_RET my_onvif_GetUsers(GetUsers_RES * p_res);
ONVIF_RET my_onvif_add_users(ONVIF_USER * p_user, int user_max);
ONVIF_RET my_onvif_delete_users(ONVIF_USER * p_user, int user_max);
ONVIF_RET my_onvif_set_users(ONVIF_USER * p_user, int user_max);
const char * my_onvif_get_user_pass(const char * username);
//event
ONVIF_NOTIFYMESSAGE * my_onvif_init_notify_message(EUA * p_eua,void *pEventInfo);


//media
ONVIF_RET my_onvif_CreateProfile(CreateProfile_REQ * p_req,CreateProfile_RES * p_reqs);
ONVIF_RET my_onvif_DeleteProfile(const char * token);
ONVIF_RET my_onvif_AddVideoSourceConfiguration(AddVideoSourceConfiguration_REQ * p_req);
ONVIF_RET my_onvif_AddVideoEncoderConfiguration(AddVideoEncoderConfiguration_REQ * p_req);
ONVIF_RET my_onvif_AddAudioSourceConfiguration(AddAudioSourceConfiguration_REQ * p_req);
ONVIF_RET my_onvif_AddAudioEncoderConfiguration(AddAudioEncoderConfiguration_REQ * p_req);
ONVIF_RET my_onvif_AddPTZConfiguration(AddPTZConfiguration_REQ * p_req);
ONVIF_RET my_onvif_RemoveVideoEncoderConfiguration(const char * token);
ONVIF_RET my_onvif_RemoveVideoSourceConfiguration(const char * token);
ONVIF_RET my_onvif_RemoveAudioEncoderConfiguration(const char * token);
ONVIF_RET my_onvif_RemoveAudioSourceConfiguration(const char * token);
ONVIF_RET my_onvif_RemovePTZConfiguration(const char * token);
ONVIF_RET my_onvif_SetVideoEncoderConfiguration(SetVideoEncoderConfiguration_REQ * p_req);
ONVIF_RET my_onvif_SetVideoSourceConfiguration(SetVideoSourceConfiguration_REQ * p_req);
ONVIF_RET my_onvif_SetAudioSourceConfiguration(SetAudioSourceConfiguration_REQ * p_req);
ONVIF_RET my_onvif_SetAudioEncoderConfiguration(SetAudioEncoderConfiguration_REQ * p_req);
ONVIF_RET my_onvif_GetAudioSources(GetAudioSources_RES * p_res);
ONVIF_RET my_onvif_GetAudioEncoderConfigurations(GetAudioEncoderCfgs_RES * p_res);
ONVIF_RET my_onvif_GetCompatibleAudioEncoderConfigurations(GetCompatibleA_Enc_Cfgs_RES * p_res);
ONVIF_RET my_onvif_GetAudioEncoderConfiguration(GetAudioEncoderCfg_RES * p_res);
ONVIF_RET my_onvif_GetAudioSourceConfigurations(GetAudioSourceCfgs_RES * p_res);
ONVIF_RET my_onvif_GetCompatibleAudioSourceConfigurations(GetCompatibleA_Src_Cfgs_RES * p_res);
ONVIF_RET my_onvif_GetAudioSourceConfigurationOptions(GetAudioSourceConfigurationOptions_REQ *p_req,GetA_Src_Options_RES * p_res);
ONVIF_RET my_onvif_GetAudioSourceConfiguration(GetAudioSourceCfg_RES * p_res);
ONVIF_RET my_onvif_GetAudioEncoderConfigurationOptions(GetAudioEncoderConfigurationOptions_REQ *p_req,GetA_Enc_Options_RES * p_res);
ONVIF_RET my_onvif_GetOSDOptions(GetOSDOptions_RES * p_res);
ONVIF_RET my_onvif_GetMetadataConfigurations(GetMetadataConfigurations_RES * p_res);
ONVIF_RET my_onvif_GetMetadataConfiguration(GetMetadataConfiguration_RES * p_res);
ONVIF_RET my_onvif_GetCompatibleMetadataConfigurations(GetCompatibleM_Cfgs_RES * p_res);
ONVIF_RET my_onvif_GetMetadataConfigurationOptions(GetMetadataConfigurationOptions_REQ *p_req,GetMetadataCfgOptions_RES * p_res);



ONVIF_RET my_onvif_GetSnapshot(char * buff, int * rlen, char * profile_token);
ONVIF_RET my_onvif_SetOSD(SetOSD_REQ * p_req);
ONVIF_RET my_onvif_CreateOSD(CreateOSD_REQ * p_req);
ONVIF_RET my_onvif_DeleteOSD(DeleteOSD_REQ * p_req);
ONVIF_RET my_onvif_StartMulticastStreaming(const char * token);
ONVIF_RET my_onvif_StopMulticastStreaming(const char * token);
ONVIF_RET my_onvif_SetMetadataConfiguration(SetMetadataConfiguration_REQ * p_req);
ONVIF_RET my_onvif_AddMetadataConfiguration(AddMetadataConfiguration_REQ * p_req);
ONVIF_RET my_onvif_RemoveMetadataConfiguration(const char * profile_token);
ONVIF_RET my_onvif_GetProfiles(GetProfiles_RES * p_res);
ONVIF_RET my_onvif_GetProfile(GetProfile_RES * p_res);
ONVIF_RET my_onvif_GetSnapshotUri(GetSnapshotUri_RES * p_res);
ONVIF_RET my_onvif_GetVideoEncoderConfigurations(Get_v_enc_cfgs_RES * p_res);
ONVIF_RET my_onvif_GetCompatibleVideoEncoderConfigurations(GetCompatiblev_enc_cfgs_RES * p_res);
ONVIF_RET my_onvif_GetVideoEncoderConfiguration(Get_v_enc_cfg_RES * p_res);
ONVIF_RET my_onvif_GetVideoSourceConfigurations(Get_v_src_cfgs_RES * p_res);
ONVIF_RET my_onvif_GetCompatibleVideoSourceConfigurations(GetCompatiblev_src_cfgs_RES * p_res);
ONVIF_RET my_onvif_GetVideoSourceConfiguration(Get_v_src_cfg_RES * p_res);
ONVIF_RET my_onvif_GetVideoSourceConfigurationOptions(GetVideoSourceConfigurationOptions_REQ * p_req,Get_v_src_options_RES * p_res);
ONVIF_RET my_onvif_GetVideoEncoderConfigurationOptions(GetVideoEncoderConfigurationOptions_REQ * p_req,Get_v_enc_options_RES * p_res);
ONVIF_RET my_onvif_GetOSDs(GetOSDs_REQ * p_req,GetOSDs_RES * p_res);
ONVIF_RET my_onvif_GetOSD(GetOSD_REQ * p_req,GetOSD_RES * p_res);
ONVIF_RET my_onvif_GetVideoSources(GetVideoSources_RES * p_res);
//analytics
ONVIF_RET my_onvif_AddVideoAnalyticsConfiguration(AddVideoAnalyticsConfiguration_REQ * p_req);
ONVIF_RET my_onvif_RemoveVideoAnalyticsConfiguration(RemoveVideoAnalyticsConfiguration_REQ * p_req);
ONVIF_RET my_onvif_SetVideoAnalyticsConfiguration(SetVideoAnalyticsConfiguration_REQ * p_req);

ONVIF_RET my_onvif_GetSupportedRules(GetSupportedRules_REQ * p_req, GetSupportedRules_RES * p_res);
ONVIF_RET my_onvif_CreateRules(CreateRules_REQ * p_req);
ONVIF_RET my_onvif_DeleteRules(DeleteRules_REQ * p_req);
ONVIF_RET my_onvif_GetRules(GetRules_REQ * p_req, GetRules_RES * p_res);
ONVIF_RET my_onvif_ModifyRules(ModifyRules_REQ * p_req);
ONVIF_RET my_onvif_CreateAnalyticsModules(CreateAnalyticsModules_REQ * p_req);
ONVIF_RET my_onvif_DeleteAnalyticsModules(DeleteAnalyticsModules_REQ * p_req);
ONVIF_RET my_onvif_GetAnalyticsModules(GetAnalyticsModules_REQ * p_req, GetAnalyticsModules_RES * p_res);
ONVIF_RET my_onvif_ModifyAnalyticsModules(ModifyAnalyticsModules_REQ * p_req);
ONVIF_RET my_onvif_GetVideoAnalyticsConfigurations(Get_VA_Configurations_RES * p_res);
ONVIF_RET my_onvif_GetVideoAnalyticsConfiguration(GetVideoAnalyticsConfiguration_REQ* p_req,Get_VA_Configuration_RES * p_res);


//PTZ

ONVIF_RET my_onvif_PTZGetStatus(const char	*ProfileToken, onvif_PTZStatus * p_ptz_status);
ONVIF_RET my_onvif_ContinuousMove(ContinuousMove_REQ * p_req);
ONVIF_RET my_onvif_PTZ_Stop(PTZ_Stop_REQ * p_req);
ONVIF_RET my_onvif_AbsoluteMove(AbsoluteMove_REQ * p_req);
ONVIF_RET my_onvif_RelativeMove(RelativeMove_REQ * p_req);
ONVIF_RET my_onvif_SetPreset(SetPreset_REQ * p_req);
ONVIF_RET my_onvif_RemovePreset(RemovePreset_REQ * p_req);
ONVIF_RET my_onvif_GotoPreset(GotoPreset_REQ * p_req);
ONVIF_RET my_onvif_GotoHomePosition(GotoHomePosition_REQ * p_req);
ONVIF_RET my_onvif_SetHomePosition(const char * token);
ONVIF_RET my_onvif_SetPTZConfiguration(SetConfiguration_REQ * p_req);
ONVIF_RET my_onvif_GetNodes(GetNodes_RES * p_res);
ONVIF_RET my_onvif_GetNode(const char *PTZNodetoken,GetNode_RES * p_res);
ONVIF_RET my_onvif_GetPTZConfigurations(GetPTZCfgs_RES * p_res);
ONVIF_RET my_onvif_GetPTZConfiguration(const char *ptzCfgToken,GetPTZCfg_RES * p_res);
ONVIF_RET my_onvif_GetPTZConfigurationOptions(const char *ptzCfgToken,GetPTZCfgOptions_RES * p_res);
ONVIF_RET my_onvif_GetPresets(const char *ptzProfileToken,GetPresets_RES * p_res);

//IMAGING
ONVIF_RET my_onvif_SetImagingSettings(SetImagingSettings_REQ * p_req);
ONVIF_RET my_onvif_Imaging_Move(Move_REQ * p_req);
ONVIF_RET my_onvif_Imaging_Stop(char *VideoSourceToken);
ONVIF_RET my_onvif_GetMoveOptions(const char * VideoSourceToken);
ONVIF_RET my_onvif_ImagingGetStatus(const char * VideoSourceToken, onvif_ImagingStatus * p_status);
ONVIF_RET my_onvif_GetImagingSettings(GetImagSettings_RES * p_res);
ONVIF_RET my_onvif_GetImagingOptions(GetImagingOptions_RES * p_res);
//recording
ONVIF_RET my_onvif_CreateRecording(CreateRecording_REQ * p_req);
ONVIF_RET my_onvif_DeleteRecording(const char * p_RecordingToken);
ONVIF_RET my_onvif_SetRecordingConfiguration(SetRecordingConfiguration_REQ * p_req);
ONVIF_RET my_onvif_CreateTrack(CreateTrack_REQ * p_req);
ONVIF_RET my_onvif_DeleteTrack(DeleteTrack_REQ * p_req);
ONVIF_RET my_onvif_SetTrackConfiguration(SetTrackConfiguration_REQ * p_req);
ONVIF_RET my_onvif_CreateRecordingJob(CreateRecordingJob_REQ  * p_req);
ONVIF_RET my_onvif_DeleteRecordingJob(const char * p_JobToken);
ONVIF_RET my_onvif_SetRecordingJobConfiguration(SetRecordingJobConfiguration_REQ * p_req);
ONVIF_RET my_onvif_SetRecordingJobMode(SetRecordingJobMode_REQ * p_req);
ONVIF_RET my_onvif_GetRecordingJobState(const char * p_JobToken, onvif_RecordingJobStateInformation * p_res);
ONVIF_RET my_onvif_GetRecordingOptions(const char * p_RecordingToken, onvif_RecordingOptions * p_res);

ONVIF_RET my_onvif_GetRecordingSummary(GetRecordingSummary_RES * p_summary);
ONVIF_RET my_onvif_GetRecordingInformation(const char * p_RecordingToken, GetRecordingInformation_RES * p_res);
ONVIF_RET my_onvif_GetMediaAttributes(GetMediaAttributes_REQ * p_req, GetMediaAttributes_RES * p_res);
ONVIF_RET my_onvif_FindRecordings(FindRecordings_REQ * p_req, FindRecordings_RES * p_res);
ONVIF_RET my_onvif_GetRecordingSearchResults(GetRecordingSearchResults_REQ * p_req, GetRecordingSearchResults_RES * p_res);
ONVIF_RET my_onvif_FindEvents(FindEvents_REQ * p_req, FindEvents_RES * p_res);
ONVIF_RET my_onvif_GetEventSearchResults(GetEventSearchResults_REQ * p_req, GetEventSearchResults_RES * p_res);
ONVIF_RET my_onvif_EndSearch(EndSearch_REQ * p_req, EndSearch_RES * p_res);
ONVIF_RET my_onvif_GetSearchState(GetSearchState_REQ * p_req, GetSearchState_RES * p_res);

ONVIF_RET my_onvif_GetReplayUri(GetReplayUri_REQ * p_req, GetReplayUri_RES * p_res);
ONVIF_RET my_onvif_GetReplayConfiguration(GetReplayConfiguration_RES * p_res);
ONVIF_RET my_onvif_SetReplayConfiguration(SetReplayConfiguration_REQ * p_req);
ONVIF_RET my_onvif_GetRecordings(GetRecordings_RES * p_res);
ONVIF_RET my_onvif_GetRecordingConfiguration(GetRecordingConfiguration_RES * p_res);
ONVIF_RET my_onvif_GetTrackConfiguration(GetTrackConfiguration_REQ * p_req,GetTrackConfiguration_RES * p_res);
ONVIF_RET my_onvif_GetRecordingJobs(GetRecordingJobs_RES * p_res);
ONVIF_RET my_onvif_GetRecordingJobConfiguration(GetRecordingJobConfiguration_RES * p_res);
//analytics_ex
ONVIF_RET my_onvif_GetSupportedRules_ex(GetSupportedRules_REQ * p_req, GetSupportedRules_RES * p_res);
ONVIF_RET my_onvif_CreateRules_ex(CreateRules_REQ * p_req);
ONVIF_RET my_onvif_DeleteRules_ex(DeleteRules_REQ * p_req);
ONVIF_RET my_onvif_GetRules_ex(GetRules_REQ * p_req, GetRules_RES * p_res);
ONVIF_RET my_onvif_ModifyRules_ex(ModifyRules_REQ * p_req);
ONVIF_RET my_onvif_GetAnalyticsModules_ex(GetAnalyticsModules_REQ * p_req, GetAnalyticsModules_RES * p_res);
ONVIF_RET my_onvif_ModifyAnalyticsModules_ex(ModifyAnalyticsModules_REQ * p_req);


#ifdef __cplusplus
}
#endif


#endif
