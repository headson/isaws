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

#ifndef	__H_ONVIF_H__
#define	__H_ONVIF_H__

#include "onvif/bm/sys_inc.h"
#include "onvif/onvif/onvif_cm.h"
#include "onvif/bm/linked_list.h"

/***************************************************************************************/

typedef struct _ONVIF_V_SRC
{
	struct _ONVIF_V_SRC * next;
	
    onvif_VideoSource VideoSource; 
} ONVIF_V_SRC;

typedef struct _ONVIF_V_SRC_CFG
{
	struct _ONVIF_V_SRC_CFG * next;

	onvif_VideoSourceConfiguration VideoSourceConfiguration; 
} ONVIF_V_SRC_CFG;

typedef struct _ONVIF_V_ENC_CFG
{    
	struct _ONVIF_V_ENC_CFG * next;	

	onvif_VideoEncoderConfiguration	VideoEncoderConfiguration;	
} ONVIF_V_ENC_CFG;

typedef struct _ONVIF_A_SRC
{    
	struct _ONVIF_A_SRC * next;
	
    onvif_AudioSource AudioSource;	
} ONVIF_A_SRC;

typedef struct _ONVIF_A_SRC_CFG
{    
	struct _ONVIF_A_SRC_CFG * next;
	
    onvif_AudioSourceConfiguration 	AudioSourceConfiguration;	
} ONVIF_A_SRC_CFG;

typedef struct _ONVIF_A_ENC_CFG
{
	struct _ONVIF_A_ENC_CFG * next;
	
    onvif_AudioEncoderConfiguration AudioEncoderConfiguration;
} ONVIF_A_ENC_CFG;

typedef struct _ONVIF_METADATA_CFG
{
	struct _ONVIF_METADATA_CFG * next;
	
	onvif_MetadataConfiguration		MetadataConfiguration;
} ONVIF_METADATA_CFG;

typedef struct
{
    BOOL    UsedFlag;
    
    onvif_PTZPreset	PTZPreset;
} PTZ_PRESET;

typedef struct _PTZ_CFG
{
	struct _PTZ_CFG * next;
	
	onvif_PTZConfiguration	PTZConfiguration;
} PTZ_CFG;

typedef struct _PTZ_NODE
{
	struct _PTZ_NODE * next;

	onvif_PTZNode	PTZNode;	
} PTZ_NODE;

typedef struct _ONVIF_VACFG
{
	struct _ONVIF_VACFG	* next;

	ONVIF_CONFIG 		* rules;					// video analytics rule configuration
	ONVIF_CONFIG 		* modules;					// video analytics module configuration

	onvif_SupportedRules  SupportedRules;			// supported rules
	
	onvif_VideoAnalyticsConfiguration	VideoAnalyticsConfiguration;
} ONVIF_VACFG;

typedef struct _ONVIF_PROFILE
{ 
	struct _ONVIF_PROFILE * next;
	
    ONVIF_V_SRC_CFG   * v_src_cfg;					// video source configuration
    ONVIF_V_ENC_CFG	  * v_enc_cfg;					// video encoder configuration
	ONVIF_A_SRC_CFG   * a_src_cfg;					// audio source configuration
	ONVIF_A_ENC_CFG	  * a_enc_cfg;					// audio encoder configration
	PTZ_CFG           * ptz_cfg;					// ptz configuration
	ONVIF_METADATA_CFG* metadata_cfg;				// metadata configuration

#ifdef VIDEO_ANALYTICS
	ONVIF_VACFG		  * va_cfg;						// video analytics configuration
#endif

    PTZ_PRESET   	  	presets[MAX_PTZ_PRESETS];	// ptz presets
    
    char name[ONVIF_NAME_LEN];						// profile name
    char token[ONVIF_TOKEN_LEN];					// profile token
    char stream_uri[ONVIF_URI_LEN];					// rtsp stream address
    BOOL fixed;										// fixed profile flag	
    BOOL multicasting;								// sending multicast streaming flag
} ONVIF_PROFILE;

typedef struct _ONVIF_SCOPE
{
	char scope[ONVIF_SCOPE_LEN];
	BOOL fixed;	
} ONVIF_SCOPE;

typedef struct _ONVIF_NET_INF
{
	struct _ONVIF_NET_INF * next;
	
	onvif_NetworkInterface	NetworkInterface; 
} ONVIF_NET_INF;

typedef struct 
{
	onvif_NetworkProtocol		NetworkProtocol;
	onvif_DNSInformation		DNSInformation;
	onvif_NTPInformation		NTPInformation;
	onvif_HostnameInformation	HostnameInformation;
	onvif_NetworkGateway		NetworkGateway;
	onvif_DiscoveryMode			DiscoveryMode;
	
	ONVIF_NET_INF * interfaces;
} ONVIF_NET;

typedef struct
{
	BOOL	fixed;
	char	Username[64];
	char	Password[64];
	
	onvif_UserLevel	UserLevel;
} ONVIF_USER;

typedef struct _ONVIF_OSD
{
	struct _ONVIF_OSD * next;
	
	onvif_OSDConfiguration OSD;
} ONVIF_OSD;

typedef struct _ONVIF_RECORDING
{
	struct _ONVIF_RECORDING * next;

	onvif_Recording	Recording;
} ONVIF_RECORDING;

typedef struct _ONVIF_RECORDINGJOB
{	
	struct _ONVIF_RECORDINGJOB * next;

	onvif_RecordingJob	RecordingJob;
} ONVIF_RECORDINGJOB;

typedef struct _ONVIF_NOTIFYMESSAGE
{
	struct _ONVIF_NOTIFYMESSAGE * next;

	onvif_NotificationMessage	NotificationMessage;
} ONVIF_NOTIFYMESSAGE;

typedef struct 
{
	unsigned int 	need_auth	 	: 1;			// Whether need auth request flag
	unsigned int	evt_sim_flag 	: 1;			// event simulate flag
	unsigned int    log_enable      : 1;			// Whether log enable flag
	unsigned int	reserved     	: 29;

	char 			serv_ip[32];					// The Configed service address
	unsigned short 	serv_port;						// The Configed service port 
	
	ONVIF_USER		users[MAX_USERS];	
	ONVIF_SCOPE   	scopes[MAX_SCOPE_NUMS];
	ONVIF_NET       network;
	
	char			EndpointReference[64];
	
	int  			evt_renew_time;
	int	 			evt_sim_interval;				// event simulate interval 

	/********************************************************/
	ONVIF_V_SRC   		* v_src;	
	ONVIF_V_SRC_CFG		* v_src_cfg;
	ONVIF_V_ENC_CFG		* v_enc_cfg;	
	ONVIF_A_SRC   		* a_src;
	ONVIF_A_SRC_CFG 	* a_src_cfg;
	ONVIF_A_ENC_CFG 	* a_enc_cfg;
		
	ONVIF_PROFILE 		* profiles;	
	PTZ_NODE      		* ptz_node;
	PTZ_CFG	      		* ptz_cfg;
	ONVIF_OSD     		* OSDs;
	ONVIF_METADATA_CFG	* metadata_cfg;

#ifdef VIDEO_ANALYTICS
	ONVIF_VACFG			* va_cfg;	
#endif

#ifdef PROFILE_G_SUPPORT
	ONVIF_RECORDING     * recordings;
	ONVIF_RECORDINGJOB  * recording_jobs;
#endif

	/********************************************************/
	onvif_DeviceInformation					DeviceInformation;
	onvif_ImagingSettings					ImagingSettings;
	onvif_ImagingOptions					ImagingOptions;
	onvif_Capabilities						Capabilities;
	onvif_SystemDateTime					SystemDateTime;
	
	onvif_VideoSourceConfigurationOptions   VideoSourceConfigurationOptions;
	onvif_VideoEncoderConfigurationOptions  VideoEncoderConfigurationOptions;
	onvif_AudioEncoderConfigurationOptions  AudioEncoderConfigurationOptions;	
	onvif_PTZConfigurationOptions           PTZConfigurationOptions;
	onvif_MetadataConfigurationOptions		MetadataConfigurationOptions;
	onvif_OSDConfigurationOptions			OSDConfigurationOptions;
} ONVIF_CFG;


typedef struct
{
	unsigned int	sys_timer_run	: 1;			// timer running flag
	unsigned int    discovery_flag	: 1;			// discovery flag
	
	unsigned int	reserved		: 30;
	
	int	 			v_src_idx;
	int	 			a_src_idx;
	int	 			v_enc_idx;
	int	 			a_enc_idx;
	int  			profile_idx;
	int             preset_idx;
	int				netinf_idx;
	int				osd_idx;

#ifdef PROFILE_G_SUPPORT
	int				recording_idx;
	int				recordingjob_idx;
#endif

	char 			local_ipstr[32];
	int  			local_port;

	PPSN_CTX      * eua_fl;
	PPSN_CTX      * eua_ul;
	LINKED_LIST   * msg_list;						// for poll mode, save notify message to list
	void          * mutex_list;
	
	uint32 	   		timer_id;

	int		   		discovery_fd;					// discovery socket handler
	pthread_t  		discovery_tid;					// discovery task handler
} ONVIF_CLS;



#ifdef __cplusplus
extern "C" {
#endif


/***************************************************************************************/
void 				onvif_init();

/***************************************************************************************/
ONVIF_RET 			onvif_add_scopes(ONVIF_SCOPE * p_scope, int scope_max);
ONVIF_RET 			onvif_set_scopes(ONVIF_SCOPE * p_scope, int scope_max);
ONVIF_RET 			onvif_remove_scopes(ONVIF_SCOPE * p_scope, int scope_max);

/***************************************************************************************/
ONVIF_RET 			onvif_add_users(ONVIF_USER * p_user, int user_max);
ONVIF_RET 			onvif_delete_users(ONVIF_USER * p_user, int user_max);
ONVIF_RET 			onvif_set_users(ONVIF_USER * p_user, int user_max);
const char        * onvif_get_user_pass(const char * username);

/***************************************************************************************/

ONVIF_NOTIFYMESSAGE * onvif_add_notify_message(ONVIF_NOTIFYMESSAGE ** p_message);
void 				  onvif_free_notify_message(ONVIF_NOTIFYMESSAGE * p_message);
void				  onvif_free_notify_messages(ONVIF_NOTIFYMESSAGE ** p_message);


ONVIF_SIMPLEITEM	* onvif_add_simple_item(ONVIF_SIMPLEITEM ** p_simpleitem);
void				  onvif_free_simple_items(ONVIF_SIMPLEITEM ** p_simpleitem);

ONVIF_ELEMENTITEM   * onvif_add_element_item(ONVIF_ELEMENTITEM ** p_elementitem);
void				  onvif_free_element_items(ONVIF_ELEMENTITEM ** p_elementitem);


/***************************************************************************************/

#ifdef PROFILE_G_SUPPORT
ONVIF_TRACK       * onvif_add_track(ONVIF_TRACK ** p_tracks);	
void				onvif_free_tracks(ONVIF_TRACK ** p_tracks);
ONVIF_TRACK       * onvif_find_track(ONVIF_TRACK	* p_tracks, const char * token);
int					onvif_get_track_nums_by_type(ONVIF_TRACK * p_tracks, onvif_TrackType);
#endif

/***************************************************************************************/
#ifdef VIDEO_ANALYTICS
ONVIF_CONFIG 	  * onvif_add_config(ONVIF_CONFIG ** p_config);
void				onvif_free_config(ONVIF_CONFIG * p_config);
void				onvif_free_configs(ONVIF_CONFIG ** p_config);
ONVIF_CONFIG 	  * onvif_find_config(ONVIF_CONFIG ** p_config, const char * name);
void				onvif_remove_config(ONVIF_CONFIG ** p_config, ONVIF_CONFIG * p_remove);
ONVIF_CONFIG 	  * onvif_get_config_prev(ONVIF_CONFIG ** p_config, ONVIF_CONFIG * p_found);

#endif
/***************************************************************************************/
void onvif_free_simple_item_descs(ONVIF_SIMPLE_ITEM_DESC ** p_simpleitem);
void onvif_free_cfg_desc_msg(ONVIF_CFG_DESC_MSG * p_cfg);
void onvif_free_cfg_desc_msgs(ONVIF_CFG_DESC_MSG ** p_cfg);
void onvif_free_cfg_desc(ONVIF_CFG_DESC * p_cfgdesc);
void onvif_free_cfg_descs(ONVIF_CFG_DESC ** p_cfgdesc);
void onvif_free_profile_sev(ONVIF_PROFILE * p_profile);
void onvif_free_profiles_sev(ONVIF_PROFILE ** p_profile);

/***************************************************************************************/

typedef enum {
  ONVIF_TYPE_ELEMENTITEM = 0,
  ONVIF_TYPE_SIMPLEITEM,
  ONVIF_TYPE_CONFIG,
  ONVIF_TYPE_NOTIFYMESSAGE,
  ONVIF_TYPE_TRACK,
  ONVIF_TYPE_PROFILE,
  ONVIF_TYPE_V_SRC,
  ONVIF_TYPE_V_ENC,
  ONVIF_TYPE_A_SRC,
  ONVIF_TYPE_A_ENC,
  ONVIF_TYPE_PTZ,
  ONVIF_TYPE_METADATA,
  ONVIF_TYPE_VACFG,
  ONVIF_TYPE_NET_INF,
  ONVIF_TYPE_OSD,
  ONVIF_TYPE_CFG_DESC,
  ONVIF_TYPE_SITEM_DESC,
  ONVIF_TYPE_CFGDESC_MSG,
  ONVIF_TYPE_PTZ_NODE,
  ONVIF_TYPE_RECINF,
  ONVIF_TYPE_RECORDING,
  ONVIF_TYPE_RECORDINGJOB,
  ONVIF_TYPE_LINK_NODE,
  ONVIF_TYPE_LINK_LIST,
}ONVIF_MEM_TYPE;

void * get_one_item(ONVIF_MEM_TYPE type);
void free_one_item(void *node, ONVIF_MEM_TYPE type);




#ifdef __cplusplus
}
#endif

#endif


