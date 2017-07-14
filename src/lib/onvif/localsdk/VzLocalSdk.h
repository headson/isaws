#ifndef VZLOCALSDK_H
#define VZLOCALSDK_H


#include "VzLocalSdk_DataDefine.h"
//暂时包含soapH以后去掉
#include "onvif_env.h"
#include <Msg_Def.h>
#include <sys_env_type.h>
//#include "file_msg_drv.h"
#include "time.h"
//#include "sysctrl.h"
#include "dpinterface/interface/interface_c.h"
#include "sysctrl_struct.h"
#ifdef __cplusplus
extern "C"{
#endif

#define ControlSystemData ControlSystemData_Ext
#define ControlSystemDataChannel ControlSystemDataChannel_Ext
#define ControlSystemDataLocalChannel ControlSystemDataLocalChannel_Ext
#define ControlSystemDataRemoteChannel ControlSystemDataRemoteChannel_Ext
#define ControlSystemDataALLChannel ControlSystemDataALLChannel_Ext
#define ControlSystemDataALGChannel ControlSystemDataALGChannel_Ext
#define ControlSystemDataIpc ControlSystemData_Ext_IO

/**
*	DHCP_SUPPORT 
*  now dhcp can get the dns server,can not get the host name and NTP server 
*  but the dhcp can return a local clock for NTP
*/
//#define DHCP_SUPPORT 
/**
*	@brief 初始化SDK
*  @return VZ_LOCALAPI_SUCCESS 或 VZ_LOCALAPI_FAILED 中一种
*/
int LocalSDK_init();

/**
*	@brief 释放SDK
*  @return VZ_LOCALAPI_SUCCESS 或 VZ_LOCALAPI_FAILED 中一种
*/
int LocalSDK_CleanUp();
int LocalSDK_GetErr();
Onvif_Info* LocalSDK_GetOnvifInfo();
//SysInfo * LocalSDK_GetSysInfo();

int LocalSDK_SetSystemDateAndTime(LocalSDK_SystemDateAndTime *SystemDateAndTime);
void LocalSDK__GetSystemDateAndTime(LocalSDK_SystemDateAndTime *SystemDateAndTime);
int LocalSDK__SetSystemFactoryDefault(__u8 value);
int LocalSDK__SetScopes(LocalSDK_ScopesInfo* ScopesInfo);
void LocalSDK__SystemReboot(void);
int LocalSDK__AddScopes(LocalSDK_ScopesInfo* ScopesInfo);
int LocalSDK__RemoveScopes(LocalSDK_ScopesInfo* ScopesInfo);
int LocalSDK__CreateUsers( LocalSDK_UsersInfo *CreateUsers);
int LocalSDK__DeleteUsers( LocalSDK_UsersInfo *DeleteUsers);
int LocalSDK__SetUser( LocalSDK_UsersInfo *SetUsers);
int LocalSDK__GetDNS(LocalSDK_DNSInfo *DNSInfo);
int LocalSDK__SetDNS(LocalSDK_DNSInfo *DNSInfo);
int LocalSDK__GetNetworkInterfaces(LocalSDK__NetworkInterfaceList *NetworkInterfaceList);
int LocalSDK__SetNetworkInterfaces(LocalSDK__NetworkInterface *NetworkInterface);
int  LocalSDK__GetNetworkProtocols(LocalSDK__NetworkProtocolList *NetworkProtocolList);
int  LocalSDK__SetNetworkProtocols(LocalSDK__NetworkProtocolList *NetworkProtocolList);
void LocalSDK__GetNetworkDefaultGateway( LocalSDK__NetworkGateway *NetworkGateway);
int LocalSDK__SetNetworkDefaultGateway(LocalSDK__NetworkGateway *NetworkGateway);
int LocalSDK__GetSupportedRules(char * Token,LocalSDK__SupportedRules *SupportedRules);
int LocalSDK__GetRules(LocalSDK__OneSetOfRule *OneSetOfRules);
int LocalSDK__CreateRules(LocalSDK__OneSetOfRule *CreatRules);
int LocalSDK__DeleteRules(LocalSDK__OneSetOfRule *DeleteRules);
int LocalSDK__ModifyRules(LocalSDK__OneSetOfRule *ModifyRules);
int LocalSDK__GetMediaUriResponse(LocalSDK__MediaUri *MediaUri);
//int LocalSDK__SetMainStreamUri(const char* main_stream_uri,const char* profile_token);
int LocalSDK__GetStreamUri(LocalSDK__MediaUri *MediaUri);
int LocalSDK__GetMetadataConfiguration(char *ConfigToken,LocalSDK__MetadataConfig *GetMetadataConfig);
int LocalSDK__GetNTP(LocalSDK_NTPInfo *GetNTP);
int LocalSDK__SetNTP(LocalSDK_SetNTP *SetNTP);
int LocalSDK__SetHostname(char *Name);
void LocalSDK__GetEventAbility(char *EventAbility,char *type);
int LocalSDK__GetAnalyticsModules(LocalSDK__Analytics *Analytics);
//int LocalSDK__SetAnalyticsModules(LocalSDK__Analytics *Analytics);
int LocalSDK__SetPresetNum(LocalSDK__Analytics *Analytics);
int LocalSDK__SetRunMode(LocalSDK__Analytics *Analytics);
int LocalSDK__SetDrawMode(LocalSDK__Analytics *Analytics);
int LocalSDK__ModifyAnalyticsModules(LocalSDK__Analytics *Analytics);
//int LocalSDK__SaveRelayCapabilities(vz_onvif_RelayServicesCapabilities *relaycap,int channel);
//void LocalSDK__ChangeXAddr(char *oldaddr,char *newaddr);
//int LocalSDK__SaveRelayCapabilitieAddrs(vz_onvif_ServiceShortInfo *Xaddr,int channel);
//void LocalSDK__GetServiceCapabilities(LocalSDK__Service_Capabilities *ServiceCap);
int LocalSDK__CreateRecording(vz_onvif_Recording *Recording);
int LocalSDK__DeleteRecording(char * p_RecordingToken);
int LocalSDK__CreateRecordingJob(vz_onvif_RecordingJob *RecordingJob);
int LocalSDK__DeleteRecordingJob(char *JobToken);
int LocalSDK__SetRecordingJobMode(LocalSDK__SetRecordingJob *RecordingJobMode);
int LocalSDK__GetRecordingJobState(vz_onvif_RecordingJob *JobState);
//int LocalSDK__SendSerial(LocalSDK__SerialPort *SerialPort);
//void LocalSDK__GetSerialPorts(LocalSDK__SerialPorts *SerialPorts);
//int LocalSDK__GetPresets(LocalSDK__PTZPresets *Presets);
int LocalSDK__GotoPreset(LocalSDK__PTZPreset *Preset);
int LocalSDK__SetPreset(LocalSDK__PTZPreset *Preset);
int LocalSDK__RemovePreset(LocalSDK__PTZPreset *Preset);
//int LocalSDK__GetPresetTours(LocalSDK__PresetTours *PresetTours);
//int LocalSDK__GetPresetTour(LocalSDK__PresetTours *PresetTours);
//int LocalSDK__ModifyPresetTour(LocalSDK__PresetTours *PresetTours);
//int LocalSDK__PresetTour_Enable(LocalSDK__PresetTours *PresetTours);
int LocalSDK__SetImagingSettings(LocalSDK__ImagingSetting *ImagingSetting);
int LocalSDK__GetImagingSettings(LocalSDK__ImagingSetting *ImagingSetting);
int LocalSDK__CreatePullPointSubscription(LocalSDK__PullPointSubscription *Subscription );
//int LocalSDK__Unsubscribe(const char * token);
int LocalSDK__GetProfile(LocalSDK__Profiles *meta_profiles);
int LocalSDK__GetRemoteProfiles(LocalSDK__Profiles *meta_profiles,int channel);
int LocalSDK__GetLocalProfiles(LocalSDK__Profiles *meta_profiles);
//int LocalSDK_SetMainProfileToken(int channel,const char* profile_token);
int LocalSDK__GetVideoEncoderConfiguration(LocalSDK__OneVideoEncoderConfig *VideoEncoderConfig);
int LocalSDK__GetVideoEncoderConfigurations(LocalSDK__VideoEncoderConfigs *EncoderConfigs);
int LocalSDK__SetVideoEncoderConfiguration(LocalSDK__OneVideoEncoderConfig *VideoEncoderConfig);
int LocalSDK__GetVideoEncoderConfigurationOptions(LocalSDK__VideoEncoderConfigurationOption *Option);
int LocalSDK__GetVideoSources(LocalSDK__VideoSources *Sources);
int LocalSDK__GetStreamUriRemote(LocalSDK__MediaUri *MediaUri);
int LocalSDK__PTZMove(LocalSDK__ptz__ContinuousMove *move);
int LocalSDK__PTZStopMove(LocalSDK__ptz__StopMove *move);
int LocalSDK__GetRecordingList(LocalSDK__RecordingInfo *RecordingInfo);
int LocalSDK__GetOSDs(LocalSDK__OSDList *OSDList);
int LocalSDK__SetOSD(LocalSDK__OSDConfig *OSDConfig);
int LocalSDK__AddVideoAnalyticsConfiguration(LocalSDK__AddVideoAnalyticsConfig *Config);
int LocalSDK__RemoveVideoAnalyticsConfiguration(char *ProfileToken);
int LocalSDK__SetVideoAnalyticsConfiguration(LocalSDK__SetVideoAnalyticsConfig *Config);
int LocalSDK__GetVideoSourceConfigurationOptions(LocalSDK__VideoSourceConfigurationOption *Option);
int LocalSDK__GetProfileResponse(LocalSDK__Profiles *meta_profiles,int remote_channel);
int LocalSDK__GetRecordingTimeList(LocalSDK__RecordingTimeSearchResults *Results);
void LocalSDK__SetRecordingTimeFilter(LocalSDK__RecordingTimeSearchFilter *TimeFilter);
int LocalSDK__UpdateProfiles(vz_onvif_Profile *updateProfile);
int LocalSDK__GetRemoteChannel(char *ProfileToken,LocalSDK__RemoteChannel_Info *Info);
//int LocalSDK__AddVideoSourceConfiguration(LocalSDK__AddVideoSourceConfig *Config);
int LocalSDK__RemoveVideoSourceConfiguration(char *ProfileToken);
int LocalSDK__GetVideoAnalyticsConfiguration(vz_onvif_VideoAnalytics *VideoAnalyticsCfg);
int LocalSDK__GetVideoSourceChannel(char *SourceToken,int *RemoteChannel);
int LocalSDK__GetSpecifiedVideoSource(LocalSDK__GetVideoSourceConfig *VideoSourceConfig);
int LocalSDK__SetVideoSource(LocalSDK__GetVideoSourceConfig *VideoSourceConfig);
int LocalSDK__GetLocalVideoSourceConfigs(LocalSDK__VideoSourceConfigs *SourceConfigs);
int LocalSDK__GetRemoteVideoSourceConfigs(LocalSDK__VideoSourceConfigs *SourceConfigs,int channel);
int LocalSDK__GetRemoteVideoSources(LocalSDK__VideoSources *Sources,int channel);
int LocalSDK__GetGuaranteedNumberOfVideoEncoderInstances(char *VideoSourceToken,int *Number);
int LocalSDK__GetSnapshotUri(LocalSDK__SnapshotUri *SnapshotUri);
int LocalSDK__GetSnapshotData(char * buff, int * rlen, char * ProfileToken);
int LocalSDK__GetVideoSourceConfigResponse(LocalSDK__VideoSourceConfigs *SourceConfigs,int remote_channel);
int LocalSDK__GetVideoEncoderConfigResponse(LocalSDK__VideoEncoderConfigs *Configs,int remote_channel);
int LocalSDK__GetRemoteVideoEncoderConfigs(LocalSDK__VideoEncoderConfigs *Configs,int channel);
int LocalSDK__GetLocalProfiles_OneChannel(LocalSDK__Profiles *meta_profiles,int Channel);
int LocalSDK__GetVideoEncoderConfig_OneChannel(LocalSDK__VideoEncoderConfigs *EncoderConfigs,int Channel);
int LocalSDK__GetLocalVideoSourceConfigs_OneChannel(LocalSDK__VideoSourceConfigs *SourceConfigs,int Channel);
int LocalSDK__GetImagingOptions(LocalSDK__ImagingOption *ImagingOption);
int LocalSDK__GetChannelNum(int *ChannelNum);
int  LocalSDK_gen_new_uri_without_port(char* old_uri);
int LocalSDK__GetReceiver(vz_onvif_Receiver *Response);
int LocalSDK__CreateReceiver(vz_onvif_Receiver *Receiver);
int LocalSDK__DeleteReceiver(const char *Token);
int LocalSDK__ModifyReceiver(vz_onvif_Receiver *Receiver);
int LocalSDK__SetReceiverMode(const char *Token,int mode);
int LocalSDK__GetReceiverState(const char *Token,int *State);
int LocalSDK__GetReplayUri(const char* RecordingToken, char *ReplayUri);
int LocalSDK__GetVideoEncoderConfigOptionsResponse(LocalSDK__SUbIPCVideoEncoderConfigOption *Option);
int LocalSDK__GetRemoteAudioSourcesConfig_OneChannel(LocalSDK__AudioSourceConfigs *SourceConfigs,int channel);
int LocalSDK__GetRemoteAudioEncoderConfigs_OneChannel(LocalSDK__AudioEncoderConfigs *EncoderConfigs,int channel);
void LocalSDK__Get_RealProfileConfig(LocalSDK__RemoteChannel_ProfileConfig_Info *Info);
int LocalSDK__Get_RemoteConfig(char *config ,int ConfigType,LocalSDK__RemoteChannel_Info *Info);
int LocalSDK__GetAudioSourceConfigResponse(LocalSDK__AudioSourceConfigs *SourceConfigs,int remote_channel);
int LocalSDK__GetAudioEncoderConfigResponse(LocalSDK__AudioEncoderConfigs *EncoderConfigs,int remote_channel);
int LocalSDK__UpdateRemoteAudioEncoder(vz_onvif_AudioEncoderConfig *Config,int Channel);
int LocalSDK__UpdateRemoteAudioSource(vz_onvif_AudioSourceConfig *Config,int Channel);
//int LocalSDK__GetRecordingConfiguration(vz_onvif_Recording *Config);
int LocalSDK__SetRecordingConfiguration(vz_onvif_Recording *Config);
//int LocalSDK__GetRecordingJobConfiguration(vz_onvif_RecordingJob *Config);
int LocalSDK__SetRecordingJobConfiguration(vz_onvif_RecordingJob *Config);
//int LocalSDK__GetTrackConfiguration(LocalSDK__TrackConfig_Info *Config);
int LocalSDK__EndSearch(LocalSDK__EndSearch_Info *Info);
int LocalSDK__GetRecordingSummary(LocalSDK__RecordingSummary_Info *Info);
int LocalSDK__GetNodes(LocalSDK__PTZNodes *Nodes);
int LocalSDK__GetSerialPortConfiguration(LocalSDK__SerialPortConfig *Config);
int LocalSDK__SetSerialPortConfiguration(LocalSDK__SerialPortConfig *Config);
int LocalSDK__GetSerialPortCfgOptions(LocalSDK__GetSerialPortConfigOptions *pOptions);
int LocalSDK__CreateOSD(VZ_ONVIF_OSD *OSD);
int LocalSDK__DeleteOSD(char * p_OSDToken,int *Channel);
int LocalSDK__GetOSD_ex(char	*OSDToken,VZ_ONVIF_OSD *OSDCfg);

int LocalSDK__GetOSD(LocalSDK__OSDConfig *OSDCfg);
int LocalSDK__SetRelayOSDToken(int channel,LocalSDK__OSDRelayTokens *OSDTokens);
int LocalSDK__GetSpecifiedLocalOSDs(LocalSDK__OSDList *OSDList,int channel);
int LocalSDK__Find_Profile(char *ProfileToken);
int LocalSDK__Find_Video_Source(char *VideoSourceToken);
int LocalSDK__CreateTrack(char *p_RecordingToken,vz_onvif_Track *p_Track);
int LocalSDK__DeleteTrack(char *p_RecordingToken,char *p_TrackToken);
int LocalSDK__SetTrackConfiguration(char *p_RecordingToken,vz_onvif_Track *p_Track);
int LocalSDK__Find_Video_Analytics(char *AnalyticsToken,int *index);
int LocalSDK__UpdateRemoteVSrcCfgOptions(int curchannel,vz_onvif_IntRectangleRange * p_option);


#ifdef NEW_ONVIF
int LocalSDK__SetDiscoveryMode(vz_onvif_DiscoveryMode * DiscoveryMode);
int LocalSDK__GetNode(char *PTZNodetoken,int * NodeID);
int LocalSDK__GetPTZConfigurationOptions(char *ptzCfgToken,int * NodeID);
int LocalSDK__SetPTZConfiguration(vz_onvif_PTZConfiguration *pCFG);
int LocalSDK__GetPTZConfiguration(char *PTZCFGtoken,int * CFGID);
int LocalSDK__AddCFG2Profile(LocalSDK__AddCFG *cfginfo);
int LocalSDK__DelCFGFromProfile(LocalSDK__DelCFG *cfginfo);
int LocalSDK_GetRecordingOptions(LocalSDK__RecordingOptions * Option);
int LocalSDK_find_recording(char *RecordingToken,vz_onvif_Recording * RecordInfo);
int LocalSDK_find_track(char *RecordingToken,char *TrackToken,vz_onvif_Track * TrackInfo);
int LocalSDK_find_recordingjob(char *RecordingjobToken,vz_onvif_RecordingJob * RecordjobInfo);
#else
//int LocalSDK__PullMessages(LocalSDK__EventPullMessages *Messages );

#endif

#ifdef __cplusplus
}
#endif


#endif
