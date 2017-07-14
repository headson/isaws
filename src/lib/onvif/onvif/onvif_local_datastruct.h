//一些公有的符号定义
#ifndef _ONVIF_LOCAL_DATADEFINE_H_
#define _ONVIF_LOCAL_DATADEFINE_H_


#include <asm/types.h>
#include <netinet/in.h>
#include "onvif/onvif/onvif.h"
//#include "onvif_cm.h"

/************************************************************************

ERROR define

************************************************************************/
#define MAX_LIST_NUM   	8

typedef struct 
{	
    onvif_SystemDateTime	SystemDateTime;		 
    onvif_DateTime 			UTCDateTime;
    onvif_DateTime 			LocalDateTime;
} GetSystemDateAndTime_RES;

typedef struct 
{	
	onvif_HostnameInformation	HostnameInfo; 	 
} GetHostname_RES;

typedef struct 
{	
	onvif_DNSInformation	DNSInfo; 	 
} GetDNS_RES;

typedef struct 
{	
	onvif_NTPInformation		NTPInfo;
} GetNTP_RES;

typedef struct 
{	
	onvif_NetworkProtocol		NetworkProtocol;
} GetNetworkProtocols_RES;

typedef struct 
{	
	onvif_NetworkGateway		NetworkGateway;
} GetNetworkDefaultGateway_RES;

typedef struct 
{	
	ONVIF_NET_INF * interfaces;
} GetNetworkInterfaces_RES;

typedef struct 
{	
	onvif_DiscoveryMode 		DiscoveryMode;
} GetDiscoveryMode_RES;

typedef struct 
{	
	onvif_DeviceInformation 	DeviceInfo;
} GetDeviceInformation_RES;

	
typedef struct 
{	
	onvif_CapabilityCategory  Category;
	onvif_Capabilities  Capabilities;
} GetCapabilities_RES;

typedef struct 
{	
	BOOL bIncludeCapability;
	onvif_Capabilities  Capabilities;
} GetServices_RES;

typedef struct 
{	
    char stream_uri[ONVIF_URI_LEN];					// rtsp stream address
}GetStreamUri_RES;

typedef struct 
{	
	char *token;
    char stream_uri[ONVIF_URI_LEN];					// rtsp stream address
}GetSnapshotUri_RES;

typedef struct 
{	
	ONVIF_SCOPE   	scopes[MAX_SCOPE_NUMS];
}GetScopes_RES;

typedef struct 
{	
	ONVIF_USER		users[MAX_USERS];	
}GetUsers_RES;



typedef struct 
{	
	ONVIF_PROFILE * profiles;
}GetProfiles_RES;

typedef struct 
{
	char * token;
	ONVIF_PROFILE profile;
}GetProfile_RES;

typedef struct 
{	
	ONVIF_V_ENC_CFG * p_v_enc_cfg;
}Get_v_enc_cfgs_RES;

typedef struct 
{	
	char *token;
	ONVIF_V_ENC_CFG * p_v_enc_cfg;
}GetCompatiblev_enc_cfgs_RES;

typedef struct 
{	
	char *ConfigurationToken;
	ONVIF_V_ENC_CFG v_enc_cfg;
}Get_v_enc_cfg_RES;

typedef struct 
{	
	ONVIF_V_SRC_CFG * p_v_src_cfgs;
}Get_v_src_cfgs_RES;

typedef struct 
{	
	char *token;
	ONVIF_V_SRC_CFG * p_v_src_cfgs;
}GetCompatiblev_src_cfgs_RES;

typedef struct 
{
	char *ConfigurationToken;
	ONVIF_V_SRC_CFG v_src_cfg;
}Get_v_src_cfg_RES;

typedef struct 
{
	char 	SourceToken[MAX_LIST_NUM][ONVIF_TOKEN_LEN];
	onvif_VideoSourceConfigurationOptions v_src_option;
}Get_v_src_options_RES;

typedef struct 
{
	onvif_VideoEncoderConfigurationOptions  v_enc_option;
}Get_v_enc_options_RES;

typedef struct 
{
	ONVIF_OSD	* OSDs;
}GetOSDs_RES;

typedef struct 
{
	ONVIF_OSD	struOSD;
}GetOSD_RES;

typedef struct 
{
	ONVIF_V_SRC * p_v_src;
}GetVideoSources_RES;

typedef struct 
{
	PTZ_NODE * p_node;
}GetNodes_RES;

typedef struct 
{
	PTZ_NODE struNode;
}GetNode_RES;

typedef struct 
{
	PTZ_CFG struPTZCfg;
}GetPTZCfg_RES;

typedef struct 
{
	PTZ_CFG * p_PTZCfg;
}GetPTZCfgs_RES;

typedef struct 
{
	PTZ_NODE struNode;
	onvif_PTZConfigurationOptions           PTZConfigurationOptions;
}GetPTZCfgOptions_RES;


typedef struct 
{
	int  num;
	PTZ_PRESET			presets[MAX_PTZ_PRESETS];	// ptz presets
}GetPresets_RES;

typedef struct 
{
	char *VideoSourceToken;
	onvif_ImagingSettings	Imaging;
}GetImagSettings_RES;

typedef struct 
{
	char *VideoSourceToken;
	onvif_ImagingOptions	ImagingOptions;
}GetImagingOptions_RES;

typedef struct 
{
	ONVIF_RECORDING * p_recording;
}GetRecordings_RES;

typedef struct 
{
	char *RecordingToken;
	onvif_RecordingConfiguration	Configuration;	// required
}GetRecordingConfiguration_RES;

typedef struct 
{
	onvif_TrackConfiguration 	Configuration;		// required
}GetTrackConfiguration_RES;

typedef struct 
{
	ONVIF_RECORDINGJOB	* recording_jobs;
}GetRecordingJobs_RES;

typedef struct 
{
	char *JobToken;
	onvif_RecordingJobConfiguration	JobConfiguration;	// required
}GetRecordingJobConfiguration_RES;

typedef struct 
{
	ONVIF_A_SRC * a_src;
}GetAudioSources_RES;

typedef struct 
{
	char *ConfigurationToken;
	ONVIF_A_SRC_CFG strua_src_cfg;
}GetAudioSourceCfg_RES;


typedef struct 
{
	ONVIF_A_SRC_CFG 	* a_src_cfg;
}GetAudioSourceCfgs_RES;

typedef struct 
{
	char *ProfileToken;
	ONVIF_A_SRC_CFG 	* a_src_cfg;
}GetCompatibleA_Src_Cfgs_RES;

typedef struct 
{
	char 	SourceToken[MAX_LIST_NUM][ONVIF_TOKEN_LEN];
}GetA_Src_Options_RES;

typedef struct 
{
	onvif_AudioEncoderConfigurationOptions a_enc_option;
}GetA_Enc_Options_RES;

typedef struct 
{
	char *ConfigurationToken;
	ONVIF_A_ENC_CFG 	strua_enc_cfg;
}GetAudioEncoderCfg_RES;

typedef struct 
{
	ONVIF_A_ENC_CFG 	* a_enc_cfg;
}GetAudioEncoderCfgs_RES;

typedef struct 
{
	char *ProfileToken;
	ONVIF_A_ENC_CFG 	* a_enc_cfg;
}GetCompatibleA_Enc_Cfgs_RES;

typedef struct 
{
	onvif_OSDConfigurationOptions			OSDConfigurationOptions;
}GetOSDOptions_RES;

typedef struct 
{
	ONVIF_METADATA_CFG * metadata_cfg;
}GetMetadataConfigurations_RES;

typedef struct 
{
	char *ConfigurationToken;
	ONVIF_METADATA_CFG strumetadata;
}GetMetadataConfiguration_RES;

typedef struct 
{
	char *ProfileToken;
	ONVIF_METADATA_CFG * metadata_cfg;
}GetCompatibleM_Cfgs_RES;

typedef struct 
{
	onvif_MetadataConfigurationOptions		M_Options;
}GetMetadataCfgOptions_RES;

typedef struct 
{
	ONVIF_VACFG * p_va_cfg;
}Get_VA_Configurations_RES;

typedef struct 
{
	ONVIF_VACFG struva_cfg;
}Get_VA_Configuration_RES;

typedef struct 
{
	ONVIF_PROFILE struprofile;
}CreateProfile_RES;



#endif

