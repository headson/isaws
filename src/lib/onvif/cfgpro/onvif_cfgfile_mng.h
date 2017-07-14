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

#ifndef __ONVIF_CFGFILE_MNG_H__
#define __ONVIF_CFGFILE_MNG_H__

//#include <file_msg_drv.h>
#include "json_onvifinfo.h"

#if defined (__cplusplus)
	extern "C" {
#endif

Onvif_Info *GetOnvifInfo();
NetworkPort *GetNetPort();
NetworkInfo *GetNetworkInfo();

int onvif_init_cfg();

int DoOnvifSetScopes(vz_onvif_Scopes *value);
int DoOnvifSetRecordings(vz_onvif_Recordings *value);
int DoOnvifSetRecordingJobs(vz_onvif_RecordingJobs *value);
int DoOnvifSetSubscription(vz_onvif_Subscriptions *value);
int DoOnvifSetEvent(vz_onvif_Event *value);
int DoOnvifSetProfiles(vz_onvif_Profiles *value,int index);
int DoOnvifSetRVideoInfo(vz_onvif_RemoteVideoInfo *value,int index);
int DoOnvifSetOSD(VZ_ONVIF_OSDs *value);
int DoOnvifSetVideoAnalytics(vz_onvif_VideoAnalytics *value,int index);
int DoOnvifSetReceiver(vz_onvif_Receivers *value);
int DoOnvifSetSerialPortsCfg(vz_onvif_SerialPortsCfg *value,int index);
int DoOnvifSetDiscoveryMode(vz_onvif_DiscoveryMode *value);
int DoOnvifSetPTZCfg(vz_onvif_PTZConfiguration *value,int index);
int DoOnvifSetHostName(vz_tds_Hostname *value);
int DoOnvifSetRemoteRtspPort(int value,int index);
int DoOnvifSetRServiceInfo(vz_onvif_ServiceShortInfo *value,int index);
//int UpdateNetmork_byPara(unsigned int *pdata);
//int UpdateNetport();
int UpdateOnvifState();
void UpdateNewOnvifIPFlag(unsigned int *pdata);
#if defined (__cplusplus)
  }
#endif

/*
int UpdateIPCStreamSupport(StreamInfo *stream_info,VideoSourceCfg *SourceCfg);
int UpdateStreamInfo(int channel, StreamInfo *pInfo);
int UpdateReceiverCfg(int channel, VideoSourceCfg *cfg);
int UpdateRecordingConfig(int channel,int enable);
int Update_OneChannel_OnvifProfileInfo(StreamInfo *stream_info,AVS_CAP_TYPE Type,int channel);
int Update_SerialCfg(Onvif_Info* pOnvifInfo);
int Newonvif_init_VA_cfg(Onvif_Info* pOnvifInfo);
#ifdef NEW_ONVIF
int Newonvif_init_OSD(int channel,vz_onvif_OSDTextType type);
int Newonvif_init_ptz_node(Onvif_Info* pOnvifInfo);
int Newonvif_init_ptz_cfg(Onvif_Info* pOnvifInfo);
int Newonvif_init_EndpointReference();
#endif
*/
/*

#ifdef ONVIF_USB_LAN
int DoOnvifSetRemoteServerCap(vz_onvif_RemoteServices *value,int channel);
int DoOnvifSetRemoteServer(int channel,vz_onvif_RemoteServerLoginConfig *value);
#endif
*/
/*
#ifdef ONVIF_USB_LAN
int DoSetOnvifIvsStreamConfig(OnvifIvsStreamConfig* value,int remote_channel);
int DoSetOnvifMainStreamUri(char* stream_uri,int remote_channel);
int DoSetOnvifMainStreamProfileToken(char* profile_token,int remote_channel);
int DoSetRemoteConnectStatus(int channel,unsigned char *value);
int DoSetOnvifRemoteSnapshotUri(char* uri,int remote_channel);
int DoOnvifSetRemoteServer(int channel,vz_onvif_RemoteServerLoginConfig *value);
int DoOnvifSetRemoteAudioSupport(int channel,int *value);
#endif
int UpdateOnvifShortInfo(void);
int UpdateNewOnvifDeviceInfo(void);
*/
#endif


