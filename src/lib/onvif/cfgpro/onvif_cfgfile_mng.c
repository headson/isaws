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
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
//#include <json/json.h>
//#include "json_sysinfo.h"
//#include "onvif_cfgfile_db.h"
//#include "vznetdp/netdp/dpclient_c.h"
//#include "onvif_dispatcher.h"
//#include "onvif_server_ctrl.h"
#include <sys_env_type.h>
#include "onvif/cfgpro/onvif_cfgfile_mng.h"
#include <system_default.h>
//#include "vz_hwi_sharemem.h"
#include "onvif/cfgpro/onvif_kvdb.h"
#include <ipc_comm/SystemServerComm.h>
#include "onvif/onvif/onvif_api.h"
#include "vzlogging/logging/vzlogging.h"

/***************************************************************************************/
////////////////////////////rebuild start///////////////////////////////////////////

#define ONVIF_REMOTE_SERVER_CONNECT_INFO_DEFAULT \
{\
	{\
		"192.168.2.200",/*char hostname[32];*/\
		80,/*int port;*/\
		"admin",/*char user[32];*/\
		"12345",/*char pass[32];*/\
		0,\
	},/*vz_onvif_RemoteServerLoginConfig onvif_remote_server;*/\
	554, /*remote_rtsp_port*/\
}

#define RECORDING_INFO_DEFAULT \
{\
	MAX_CHANNEL,\
	0,\
	"",\
	VZ_RecordingMaxNum,\
	0,\
	{\
		{0, "", FMT_MAX_NUM, VZ_REC_MAX_NUM},\
		{0, "", FMT_MAX_NUM, VZ_REC_MAX_NUM},\
		{0, "", FMT_MAX_NUM, VZ_REC_MAX_NUM},\
		{0, "", FMT_MAX_NUM, VZ_REC_MAX_NUM},\
	}\
}

#ifdef NEW_ONVIF
#define ONVIF_ONE_PROFILE_INFO_DEFAULT \
{\
	"",/*char Name[32]*/\
	"",/*char token[32]*/\
	"",/*char oldtoken[32]*/\
	{\
		0,/*UserCount*/\
		"",/*char Name[32]*/\
		"",/*char token[32]*/\
		"",/*char oldtoken[32]*/\
		{0,0,0,0,},\
	},/*vz_onvif_VideoSourceConfig*/\
	{\
		"",/*char Name[32]*/\
		"",/*char token[32]*/\
		"",/*char oldtoken[32]*/\
		0,\
		{0,0,},\
		0,\
		{0,0,0,},\
		{0,0,},\
	},/*vz_onvif_VideoEncoderConfig*/\
	-1,/*VA_ID*/\
	{\
		"",/*char token[32]*/\
		0,/*bEnable*/\
		0,/*eRecID*/\
	},/*vz_onvif_VideoRecordingConfig*/\
	{\
		"",/*char Name*/\
		"",/*token*/\
		"",/*oldtoken*/\
		"",/*oldSourceToken*/\
	},/*vz_onvif_AudioSourceConfig*/\
	{\
		"",/*char Name*/\
		"",/*token*/\
		"",/*oldtoken*/\
	},/*vz_onvif_AudioEncoderConfig*/\
	-1,/*PTZCfg_id*/\
	Profile_Type_Max,/*Profile_Type Type;*/\
	1,/*enum vz_boolean fixed*/\
	"",/*char out_uri[MAX_URI_LEN];*/\
	0,/*int  frameFmt;*/\
}
#else
#define ONVIF_ONE_PROFILE_INFO_DEFAULT \
{\
	"",/*char Name[32]*/\
	"",/*char token[32]*/\
	"",/*char oldtoken[32]*/\
	{\
		0,/*UserCount*/\
		"",/*char Name[32]*/\
		"",/*char token[32]*/\
		"",/*char oldtoken[32]*/\
		{0,0,0,0,},\
	},/*vz_onvif_VideoSourceConfig*/\
	{\
		"",/*char Name[32]*/\
		"",/*char token[32]*/\
		"",/*char oldtoken[32]*/\
		0,\
		{0,0,},\
		0,\
		{0,0,0,},\
		{0,0,},\
	},/*vz_onvif_VideoEncoderConfig*/\
	{\
		"",/*char Name[32]*/\
		"",/*char token[32]*/\
	},/*vz_onvif_VideoAnalyticsConfig*/\
	{\
		"",/*char token[32]*/\
		0,/*bEnable*/\
		0,/*eRecID*/\
	},/*vz_onvif_VideoRecordingConfig*/\
	{\
		"",/*char Name*/\
		"",/*token*/\
		"",/*oldtoken*/\
		"",/*oldSourceToken*/\
	},/*vz_onvif_AudioSourceConfig*/\
	{\
		"",/*char Name*/\
		"",/*token*/\
		"",/*oldtoken*/\
	},/*vz_onvif_AudioEncoderConfig*/\
	Profile_Type_Max,/*Profile_Type Type;*/\
	1,/*enum vz_boolean fixed*/\
	"",/*char out_uri[MAX_URI_LEN];*/\
	0,/*int  frameFmt;*/\
}
#endif

#define ONVIF_ONE_CHANNEL_INFO_DEFAULT \
{\
	{\
		ONVIF_ONE_PROFILE_INFO_DEFAULT,\
		ONVIF_ONE_PROFILE_INFO_DEFAULT,\
		ONVIF_ONE_PROFILE_INFO_DEFAULT,\
		ONVIF_ONE_PROFILE_INFO_DEFAULT,\
	},\
}

#define ONVIF_REMOTESTREAMCONFIG_DEFAULT \
{\
	{\
		{\
			0,/*vz_boolean				main_stream_supported;*/\
			0,/*vz_boolean				main_stream_enable;*/\
			"",/*char profile_token[256];*/\
			"",/*char stream_uri[1024];*/\
		},\
	},/*main_stream_config*/\
	{\
		{\
			0,/*vz_boolean				main_stream_supported;*/\
			0,/*vz_boolean				main_stream_enable;*/\
			"",/*char profile_token[256];*/\
			"",/*char stream_uri[1024];*/\
		},\
		{\
			"",/*char profile_name[256];*/\
			"",/*char video_encoder_config_token[256];	*/\
			0,/*int encoding; */\
			0,/*int width;  */\
			0,/*int height;*/\
			0,/*int framerate;*/\
		},\
	},/*IvsStreamConfig;*/\
	{\
		0,/*vz_boolean				alarm_stream_supported;*/\
		0,/*vz_boolean				alarm_stream_enable;*/\
		"",/*char profile_token[256];*/\
		"",/*char stream_uri[1024];*/\
	},\
}

#define ONVIF_SERVICE_SHORT_INFO_DEFAULT \
{\
	{\
		{\
			"http://www.onvif.org/ver10/device/wsdl",/* Namespac*/\
			"",/* XAddr[STR_SHORT_LEN];*/\
			{\
				0,/* Major;*/\
				3,/* Minor;*/\
			},/*  Version;*/\
		},/*device_service;*/\	
		{\
			"http://www.onvif.org/ver10/media/wsdl",/* Namespac*/\
			"",/*XAddr[STR_SHORT_LEN];*/\
			{\
				0,/* Major;*/\
				3,/* Minor;*/\
			},/* Version;*/\		
		},/*media;*/\
		{\
			"http://www.onvif.org/ver10/events/wsdl",/*Namespac*/\
			"",/* XAddr[STR_SHORT_LEN];*/\
			{\
				0,/*Major;*/\
				3,/*Minor;*/\
			},/* Version;*/\
		},/*events;*/\
		{\
			"http://www.onvif.org/ver20/ptz/wsdl",/* Namespac*/\
			"",/* XAddr[STR_SHORT_LEN];*/\
			{\
				0,/*Major;*/\
				3,/* Minor;*/\
			},/*Version;*/\
		},/*ptz;*/\
		{\
			"http://www.onvif.org/ver20/imaging/wsdl",/*Namespac*/\
			"",/* XAddr[STR_SHORT_LEN];*/\
			{\
				0,/*Major;*/\
				3,/* Minor;*/\
			},/*Version;*/\
		},/*imaging;*/\
		{\
			"http://www.onvif.org/ver10/deviceIO/wsdl",/*Namespac*/\
			"",/*XAddr[STR_SHORT_LEN];*/\
			{\
				0,/* Major;*/\
				3,/* Minor;*/\
			},/*Version;*/\
		},/*deviceIO;*/\
		{\
			"http://www.onvif.org/ver20/analytics/wsdl",/*Namespac*/\
			"",/*XAddr[STR_SHORT_LEN];*/\
			{\
				0,/* Major;*/\
				3,/* Minor;*/\
			},/* Version;*/\
		},/*analytics;*/\
		{\
			"http://www.onvif.org/ver10/display/wsdl",/*Namespac*/\
			"",/*XAddr[STR_SHORT_LEN];*/\
			{\
				0,/*Major;*/\
				3,/*Minor;*/\
			},/* Version;*/\
		},/*display;*/\
		{\
			"http://www.onvif.org/ver10/recording/wsdl",/*Namespac*/\
			"",/*XAddr[STR_SHORT_LEN];*/\
			{\
				0,/* Major;*/\
				3,/*Minor;*/\
			},/* Version;*/\
		},/*recording;*/\
		{\
			"http://www.onvif.org/ver10/search/wsdl",/*Namespac*/\
			"",/* XAddr[STR_SHORT_LEN];*/\
			{\
				0,/*Major;*/\
				3,/* Minor;*/\
			},/* Version;*/\
		},/*search;*/\
		{\
			"http://www.onvif.org/ver10/replay/wsdl",/*Namespac*/\
			"",/*XAddr[STR_SHORT_LEN];*/\
			{\
				0,/*Major;*/\
				3,/* Minor;*/\
			},/* Version;*/\
		},/*replay;*/\
		{\
			"http://www.onvif.org/ver10/receiver/wsdl",/* Namespac*/\
			"",/* XAddr[STR_SHORT_LEN];*/\
			{\
				0,/*Major;*/\
				3,/* Minor;*/\
			},/* Version;*/\
		},/*receiver;*/\
		{\
			"http://www.onvif.org/ver10/analyticsdevice/wsdl",/*Namespac*/\
			"",/* XAddr[STR_SHORT_LEN];*/\
			{\
				0,/* Major;*/\
				3,/*Minor;*/\
			},/* Version;*/\
		},/*analyticsdevice;*/\
	}\
}

#define ONVIF_SERVER_SERVICE_DEFAULT \
{\
	ONVIF_SERVICE_SHORT_INFO_DEFAULT,\
	{\
		{\
			0,/*enum xsd__boolean_ IPFilter */\
			0,/*enum xsd__boolean_ ZeroConfiguration*/\
			0,/*enum xsd__boolean_ IPVersion6*/\
			0,/*enum xsd__boolean_ DynDNS*/\
			0,/*enum xsd__boolean_ Dot11Configuration*/\
			0,/*int Dot1XConfigurations*/\
			0,/*enum xsd__boolean_ HostnameFromDHCP*/\
			0,/*int NTP*/\
			0,/*enum xsd__boolean_ DHCPv6*/\
		},/*struct vz_tds__NetworkCapabilities Network;*/\
		{\
			0,/*enum xsd__boolean_ TLS1_x002e0*/\
			0,/*enum xsd__boolean_ TLS1_x002e1*/\
			0,/*enum xsd__boolean_ TLS1_x002e2*/\
			0,/*enum xsd__boolean_ OnboardKeyGeneration*/\
			0,/*enum xsd__boolean_ AccessPolicyConfig*/\
			0,/*enum xsd__boolean_ DefaultAccessPolicy*/\
			0,/*enum xsd__boolean_ Dot1X;*/\
			0,/*enum xsd__boolean_ RemoteUserHandling*/\
			0,/*enum xsd__boolean_ X_x002e509Token*/\
			0,/*enum xsd__boolean_ SAMLToken*/\
			0,/*enum xsd__boolean_ KerberosToken*/\
			0,/*enum xsd__boolean_ UsernameToken*/\
			0,/*enum xsd__boolean_ HttpDigest*/\
			0,/*enum xsd__boolean_ RELToken*/\
			"",/*char SupportedEAPMethods[STR_SHORT_LEN]*/\
		},/*struct vz_tds__SecurityCapabilities Security*/\
		{\
			0,/*enum xsd__boolean_ DiscoveryResolve*/\
			0,/*enum xsd__boolean_ DiscoveryBye*/\
			0,/*enum xsd__boolean_ RemoteDiscovery*/\
			0,/*enum xsd__boolean_ SystemBackup*/\
			0,/*enum xsd__boolean_ SystemLogging*/\
			0,/*enum xsd__boolean_ FirmwareUpgrade*/\
			0,/*enum xsd__boolean_ HttpFirmwareUpgrade*/\
			0,/*enum xsd__boolean_ HttpSystemBackup*/\
			0,/*enum xsd__boolean_ HttpSystemLogging*/\
			0,/*enum xsd__boolean_ HttpSupportInformation*/\
		},/*struct vz_tds__SystemCapabilities System*/\
		{\
			"",/*char AuxiliaryCommands[STR_SHORT_LEN]*/\
		},/*struct vz_tds__MiscCapabilities Misc*/\
	},/*						device_service_cap;*/\
	{\
		{\
			0,\
			1,\
			1,\
		},\
		{\
			24,\
		},\
	},/*vz_tds__MediaCapabilities*/\
	{\
		0,\
		0,\
	},/*vz_tds__AnalyticsCapabilities*/\
	{\
		0,\
		0,\
		0,\
	},/*vz_tds__EventCapabilities*/\
	{\
		{\
			0,\
			0,\
			0,\
			0,\
			0,\
		},/*vz_tds__DeviceIOCapabilities*/\
		{\
			0,\
			0,\
			16384,\
			16384,\
			8,\
			"H264",\
		},/*vz_tds__RecordingCapabilities*/\
		{\
			0,\
			0,\
		},/*vz_tds__SearchCapabilities*/\
		{\
			0,\
			0,\
			"",\
		},/*vz_tds__ReplayCapabilities*/\
		{\
			0,\
			0,\
			0,\
			8,\
			255,\
		},/*vz_tds__ReceiverCapabilities*/\
	},/*vz_tds__CapabilitiesExtension*/\
}

#define ONVIF_REMOTE_SERVER_SERVICE_DEFAULT \
{\
	ONVIF_SERVICE_SHORT_INFO_DEFAULT,\
}

#define ONVIF_REMOTE_SERVER_INFO_DEFAULT \
{\
	ONVIF_REMOTE_SERVER_CONNECT_INFO_DEFAULT,\
	ONVIF_REMOTE_SERVER_SERVICE_DEFAULT,\
	ONVIF_REMOTESTREAMCONFIG_DEFAULT,\
	{\
		"",\
	},/*vz_onvif_JpgMng_url*/\
	{\
		"",\
		"",\
		"",\
		{\
			{\
				0,\
				100,\
			},\
			{\
				0,\
				100,\
			},\
			{\
				320,\
				1280,\
			},\
			{\
				240,\
				720,\
			},\
		},/*RelayVSCfgOptions*/\
	},/*vz_onvif_RemoteVideoInfo*/\
	0,/*audiosupport*/\
	0,/*vz_ReceiverState*/\
}

#define ONVIF_RECEIVER_CONFIG_IPC_0 \
{\
	"rtsp://ipc0@192.168.2.200:80/onvif/receiver",\
	vz_ReceiverMode__AlwaysConnect,\
	vz_StreamType__RTP_Unicast,\
	vz_TransportProtocol__RTSP,\
}
#define ONVIF_RECEIVER_CONFIG_V4L2_0 \
{\
	"rtsp://v4l20/onvif/receiver",\
	vz_ReceiverMode__AlwaysConnect,\
	vz_StreamType__RTP_Unicast,\
	vz_TransportProtocol__RTSP,\
}


#define ONVIF_TRACK_CONFIG \
{\
	"",/*TrackToken*/\
	{\
		-1,/*TrackType*/\
		"",/*Description*/\
	},/*vz_onvif_TrackConfiguration*/\
}

#define ONVIF_RECORDING_CONFIG \
{\
	0,/*channel*/\
	0,/*TrackNum*/\
	"",/*RecordingToken*/\
	{\
		1,/*MaximumRetentionTimeFlag*/\
		0,/*Reserved*/\
		0,/*MaximumRetentionTime*/\
		{\
			"",\
			"",\
			"",\
			"",\
			"",\
		},/*vz_onvif_RecordingSourceInfo*/\
		"",/*Content*/\
	},/*vz_onvif_RecordingCfg*/\
	{\
		ONVIF_TRACK_CONFIG,\
		ONVIF_TRACK_CONFIG,\
		ONVIF_TRACK_CONFIG,\
		ONVIF_TRACK_CONFIG,\
	},/*vz_onvif_Track*/\
}

#define ONVIF_RECORDING_JOB_SOURCE \
{\
	1,/*SourceTokenFlag*/\
	0,/*AutoCreateReceiverFlag*/\
	0,/*Reserved*/\
	0,/*AutoCreateReceiver*/\
	0,/*sizeTracks*/\
	{\
		1,/*TypeFlag*/\
		0,/*Reserved*/\
		"",\
		"",\
	},/*vz_onvif_SourceReference*/\
	{\
		{\
			"",\
			"",\
		},\
		{\
			"",\
			"",\
		},\
		{\
			"",\
			"",\
		},\
		{\
			"",\
			"",\
		},\
	},/*vz_onvif_RecordingJobTrack*/\
}


#define ONVIF_RECORDING_JOB_CONFIG \
{\
	"",/*RecordingJobToken*/\
	"",/*RecordingToken*/\
	"",/*Mode*/\
	{\
		ONVIF_RECORDING_JOB_SOURCE,\
		ONVIF_RECORDING_JOB_SOURCE,\
		ONVIF_RECORDING_JOB_SOURCE,\
		ONVIF_RECORDING_JOB_SOURCE,\
	},/*vz_onvif_RecordingJobSource*/\
	0,/*sizeSource*/\
	0,/*Priority*/\
}


#define ONVIF_EVENT \
{\
	0,\
	"",\
	{\
		{\
			"",\
			"",\
		},\
		{\
			"",\
			"",\
		},\
		{\
			"",\
			"",\
		},\
		{\
			"",\
			"",\
		},\
		{\
			"",\
			"",\
		},\
		{\
			"",\
			"",\
		},\
		{\
			"",\
			"",\
		},\
		{\
			"",\
			"",\
		},\
	},\
}
#define ONVIF_SUBSCRIPTION \
{\
	{\
		{\
			"",\
			"",\
			0,\
		},\
		{\
			"",\
			"",\
			0,\
		},\
		{\
			"",\
			"",\
			0,\
		},\
		{\
			"",\
			"",\
			0,\
		},\
		{\
			"",\
			"",\
			0,\
		},\
		{\
			"",\
			"",\
			0,\
		},\
		{\
			"",\
			"",\
			0,\
		},\
		{\
			"",\
			"",\
			0,\
		},\
	},\
	0,\
}

#define ONVIF_SERIAL_PORT \
{\
	2,\
	2400,\
	0,\
	8,\
	1,\
	"Pelco D",\
}


#define ONVIF_OSD_CFG_DEFAULT \
{\
	0,/*used*/\
	0,/*channel*/\
	{\
		0,/*TextStringFlag*/\
		0,/*ImageFlag*/\
		0,/*ImageEnableFlag*/\
		0,/*Reserved*/\
		0,/*Type*/\
		"",/*token*/\
		"",/*VideoSourceConfigurationToken*/\
		"",/*relaytoken*/\
		"",/*relayVSrcCfgToken*/\
		{\
			0,/*PosFlag*/\
			0,/*Reserved*/\
			0,/*Type*/\
			{\
				0.0,/*X*/\
				0.0,/*Y*/\
			},/*Pos*/\
		},/*vz_onvif_OSDPosConfiguration*/\
		{\
			0,/*DateFormatFlag*/\
			0,/*TimeFormatFlag*/\
			0,/*FontSizeFlag*/\
			0,/*FontColorFlag*/\
			0,/*BackgroundColorFlag*/\
			0,/*PlainTextFlag*/\
			0,/*DateEnableFlag*/\
			0,/*TimeEnableFlag*/\
			0,/*PlainEnableFlag*/\
			0,/*Reserved*/\
			0,/*Type*/\
			0,/*FontSize*/\
			"",/*DateFormat*/\
			"",/*TimeFormat*/\
			{\
				0,/*ColorspaceFlag*/\
				0,/*TransparentFlag*/\
				0,/*Reserved*/\
				0.0,/*X*/\
				0.0,/*Y*/\
				0.0,/*Z*/\
				0,/*Transparent*/\
				"",/*Colorspace*/\
			},/*FontColor*/\
			{\
				0,/*ColorspaceFlag*/\
				0,/*TransparentFlag*/\
				0,/*Reserved*/\
				0.0,/*X*/\
				0.0,/*Y*/\
				0.0,/*Z*/\
				0,/*Transparent*/\
				"",/*Colorspace*/\
			},/*BackgroundColor*/\
			"",/*PlainText*/\
		},/*vz_onvif_OSDTextConfiguration*/\
		{\
			"",/*ImgPath*/\
		},/*vz_onvif_OSDImgConfiguration*/\
	},/*vz_onvif_OSDCfg */\
}


#define ONVIF_VIDEO_ANALY_DEFAULT \
{\
	0,/*used*/\
	{\
		0,/*UseCount*/\
		"",/*token*/\
		"",/*Name*/\
		{\
			{\
				{\
					"",/*Name*/\
					"",/*Type*/\
				},\
				{\
					"",/*Name*/\
					"",/*Type*/\
				},\
			},/*AnalyticsModule*/\
		},/*vz_onvif_AnalyticsEngineCfg*/\
		{\
			{\
				{\
					"",/*Name*/\
					"",/*Type*/\
				},\
				{\
					"",/*Name*/\
					"",/*Type*/\
				},\
				{\
					"",/*Name*/\
					"",/*Type*/\
				},\
				{\
					"",/*Name*/\
					"",/*Type*/\
				},\
				{\
					"",/*Name*/\
					"",/*Type*/\
				},\
				{\
					"",/*Name*/\
					"",/*Type*/\
				},\
				{\
					"",/*Name*/\
					"",/*Type*/\
				},\
				{\
					"",/*Name*/\
					"",/*Type*/\
				},\
			},/*Rule*/\
		},/*vz_onvif_RuleEngineCfg*/\
	},/*vz_onvif_VideoAnalyticsCfg */\
}


#ifdef NEW_ONVIF

#define ONVIF_PTZ_CFG_DEFAULT \
{\
	0,/*used*/\
	{\
		1,/*DefaultPTZSpeedFlag*/\
		1,/*DefaultPTZTimeoutFlag*/\
		1,/*PanTiltLimitsFlag*/\
		1,/*ZoomLimitsFlag*/\
		1,/*ExtensionFlag*/\
		0,/*Reserved*/\
		0,/*UseCount*/\
		5,/*DefaultPTZTimeout*/\
		"",/*Name*/\
		"",/*token*/\
		"",/*NodeToken*/\
		{\
			1,/*PanTiltFlag*/\
			1,/*ZoomFlag*/\
			0,/*Reserved*/\
			{\
				0.5,/*X*/\
				0.5,/*Y*/\
			},/*vz_onvif_Vector*/\
			{\
				0.5,/*X*/\
			},/*vz_onvif_Vector1D*/\
		},/*vz_onvif_PTZSpeed*/\
		{\
			{\
				-1.0,/*Min*/\
				1.0,/*Max*/\
			},/*XRange*/\
			{\
				-1.0,/*Min*/\
				1.0,/*Max*/\
			},/*YRange*/\
		},/*vz_onvif_PanTiltLimits*/\
		{\
			{\
				0.0,/*Min*/\
				1.0,/*Max*/\
			},/*XRange*/\
		},/*vz_onvif_ZoomLimits*/\
		{\
			0,/*vz_onvif_EFlipMode*/\
			0,/*vz_onvif_ReverseMode*/\
		},/*vz_onvif_PTControlDirection*/\
	},/*vz_onvif_PTZConfiguration */\
}


#define ONVIF_PTZ_NODE_DEFAULT \
{\
	0,/*used*/\
	{\
		1,/*NameFlag*/\
		0,/*ExtensionFlag*/\
		0,/*FixedHomePositionFlag*/\
		0,/*Reserved*/\
		"",/*token*/\
		"",/*Name*/\
		{\
			1,/*AbsolutePanTiltPositionSpaceFlag*/\
			1,/*AbsoluteZoomPositionSpaceFlag*/\
			1,/*RelativePanTiltTranslationSpaceFlag*/\
			1,/*RelativeZoomTranslationSpaceFlag*/\
			1,/*ContinuousPanTiltVelocitySpaceFlag*/\
			1,/*ContinuousZoomVelocitySpaceFlag*/\
			1,/*PanTiltSpeedSpaceFlag*/\
			1,/*ZoomSpeedSpaceFlag*/\
			0,/*Reserved*/\
			{\
				{\
					-1.0,/*Min*/\
					1.0,/*Max*/\
				},/*XRange*/\
				{\
					-1.0,/*Min*/\
					1.0,/*Max*/\
				},/*YRange*/\
			},/*AbsolutePanTiltPositionSpace*/\
			{\
				{\
					0.0,/*Min*/\
					1.0,/*Max*/\
				},/*XRange*/\
			},/*AbsoluteZoomPositionSpace*/\
			{\
				{\
					-1.0,/*Min*/\
					1.0,/*Max*/\
				},/*XRange*/\
				{\
					-1.0,/*Min*/\
					1.0,/*Max*/\
				},/*YRange*/\
			},/*RelativePanTiltTranslationSpace*/\
			{\
				{\
					-1.0,/*Min*/\
					1.0,/*Max*/\
				},/*XRange*/\
			},/*RelativeZoomTranslationSpace*/\
			{\
				{\
					-1.0,/*Min*/\
					1.0,/*Max*/\
				},/*XRange*/\
				{\
					-1.0,/*Min*/\
					1.0,/*Max*/\
				},/*YRange*/\
			},/*ContinuousPanTiltVelocitySpace*/\
			{\
				{\
					-1.0,/*Min*/\
					1.0,/*Max*/\
				},/*XRange*/\
			},/*ContinuousZoomVelocitySpace*/\
			{\
				{\
					0.0,/*Min*/\
					1.0,/*Max*/\
				},/*XRange*/\
			},/*PanTiltSpeedSpace*/\
			{\
				{\
					0.0,/*Min*/\
					1.0,/*Max*/\
				},/*XRange*/\
			},/*ZoomSpeedSpace*/\
		},/*vz_onvif_PTZSpaces*/\
		128,/*MaximumNumberOfPresets*/\
		1,/*HomeSupported*/\
		{\
			1,/*SupportedPresetTourFlag*/\
			0,/*Reserved*/\
			{\
				1,/*PTZPresetTourOperation_Start*/\
				1,/*PTZPresetTourOperation_Stop*/\
				1,/*PTZPresetTourOperation_Pause*/\
				0,/*PTZPresetTourOperation_Extended*/\
				0,/*Reserved*/\
				1,/*MaximumNumberOfPresetTours*/\
			},/*vz_onvif_PTZPresetTourSupported*/\
		},/*vz_onvif_PTZNodeExtension*/\
		0,/*FixedHomePosition*/\
	},/*vz_onvif_PTZNode */\
}

#define ONVIF_PTZCFG_OPTION_DEFAULT \
{\
	1,/*PTControlDirectionFlag*/\
	0,/*Reserved*/\
	{\
		1,/*min*/\
		100,/*max*/\
	},/*PTZTimeout*/\
	{\
		1,/*EFlipMode_OFF*/\
		1,/*EFlipMode_ON*/\
		0,/*EFlipMode_Extended*/\
		1,/*ReverseMode_OFF*/\
		1,/*ReverseMode_ON*/\
		1,/*ReverseMode_AUTO*/\
		0,/*ReverseMode_Extended*/\
		0,/*Reserved*/\
	},/*PTControlDirection*/\
}

#define ONVIF_COLOR_DEFAULT \
{\
	0,/*ColorspaceFlag*/\
	0,/*Reserved*/\
	0.0,/*X*/\
	0.0,/*Y*/\
	0.0,/*Z*/\
	"",/*Colorspace*/\
}

#define ONVIF_COLOR_SPACE_RANGE_DEFAULT \
{\
	{\
		0.0,/*min*/\
		1.0,/*max*/\
	},/*X*/\
	{\
		0.0,/*min*/\
		1.0,/*max*/\
	},/*Y*/\
	{\
		0.0,/*min*/\
		1.0,/*max*/\
	},/*Z*/\
	"",/*Colorspace*/\
}



#define ONVIF_OSDCOLOR_OPTION_DEFAULT \
{\
	0,/*ColorFlag*/\
	0,/*TransparentFlag*/\
	0,/*Reserved*/\
	{\
		0,/*ColorListSize*/\
		{\
			ONVIF_COLOR_DEFAULT,\
			ONVIF_COLOR_DEFAULT,\
			ONVIF_COLOR_DEFAULT,\
			ONVIF_COLOR_DEFAULT,\
			ONVIF_COLOR_DEFAULT,\
			ONVIF_COLOR_DEFAULT,\
			ONVIF_COLOR_DEFAULT,\
			ONVIF_COLOR_DEFAULT,\
			ONVIF_COLOR_DEFAULT,\
			ONVIF_COLOR_DEFAULT,\
		},/*ColorList*/\
		0,/*ColorspaceRangeSize*/\
		{\
			ONVIF_COLOR_SPACE_RANGE_DEFAULT,\
			ONVIF_COLOR_SPACE_RANGE_DEFAULT,\
			ONVIF_COLOR_SPACE_RANGE_DEFAULT,\
			ONVIF_COLOR_SPACE_RANGE_DEFAULT,\
			ONVIF_COLOR_SPACE_RANGE_DEFAULT,\
			ONVIF_COLOR_SPACE_RANGE_DEFAULT,\
			ONVIF_COLOR_SPACE_RANGE_DEFAULT,\
			ONVIF_COLOR_SPACE_RANGE_DEFAULT,\
			ONVIF_COLOR_SPACE_RANGE_DEFAULT,\
			ONVIF_COLOR_SPACE_RANGE_DEFAULT,\
		},/*ColorspaceRangeSize*/\
	},/*Color*/\
	{\
		1,/*min*/\
		100,/*max*/\
	},/*Transparent*/\
}


#define ONVIF_OSD_OPTION_DEFAULT \
{\
	1,/*OSDType_Text*/\
	0,/*OSDType_Image*/\
	0,/*OSDType_Extended*/\
	1,/*OSDPosType_UpperLeft*/\
	1,/*OSDPosType_UpperRight*/\
	1,/*OSDPosType_LowerLeft*/\
	1,/*OSDPosType_LowerRight*/\
	1,/*OSDPosType_Custom*/\
	1,/*TextOptionFlag*/\
	0,/*ImageOptionFlag*/\
	0,/*Reserved*/\
	{\
		0,/*ImageFlag*/\
		1,/*PlainTextFlag*/\
		1,/*DateFlag*/\
		1,/*TimeFlag*/\
		1,/*DateAndTimeFlag*/\
		0,/*Reserved*/\
		5,/*Total*/\
		0,/*Image*/\
		4,/*PlainText*/\
		1,/*Date*/\
		1,/*Time*/\
		1,/*DateAndTime*/\
	},/*MaximumNumberOfOSDs*/\
	{\
		1,/*OSDTextType_Plain*/\
		1,/*OSDTextType_Date*/\
		0,/*OSDTextType_Time*/\
		1,/*OSDTextType_DateAndTime*/\
		1,/*FontSizeRangeFlag*/\
		0,/*FontColorFlag*/\
		0,/*BackgroundColorFlag*/\
		0,/*Reserved*/\
		{\
			16,/*min*/\
			16,/*max*/\
		},/*FontSizeRange*/\
		3,/*DateFormatSize*/\
		{\
			"MM/DD/YYYY","DD/MM/YYYY","YYYY/MM/DD","","","","","","","",\
		},/*DateFormat*/\
		2,/*TimeFormatSize*/\
		{\
			"hh:mm:ss tt","HH:mm:ss","","","","","","","","",\
		},/*TimeFormat*/\
		ONVIF_OSDCOLOR_OPTION_DEFAULT,/*FontColor*/\
		ONVIF_OSDCOLOR_OPTION_DEFAULT,/*BackgroundColor*/\
	},/*TextOption*/\
	{\
		0,/*ImagePathSize*/\
		{\
			"","","","","","","","","","",\
		},/*ImagePath*/\
	},/*ImageOption*/\
}


#endif

static Onvif_Info Onvif_Info_Default =
{	
	ONVIF_SERVER_SERVICE_DEFAULT,//vz_onvif_Services services;
	{
		"",//		char Manufacturer[STR_SHORT_LEN];
		"",//		char Model[STR_SHORT_LEN];
		"",//		char FirmwareVersion[STR_SHORT_LEN];
		"",//		char SerialNumber[STR_SHORT_LEN];
		"",//		char HardwareId[STR_SHORT_LEN];
	},//vz__tds__DeviceInformation device_information;
	{
		0,// enum tt__SetDateTimeType DateTimeType;
		"",// char TZ[STR_SHORT_LEN];
	},//vz_tds_SystemDateAndTimeConfig time_cfg;
	{
		{
			"onvif://www.onvif.org/type/Network_Video_Transmitter",
			"onvif://www.onvif.org/type/video_encoder",
			"onvif://www.onvif.org/type/ptz",
			"onvif://www.onvif.org/location/city/chengdu",
			"onvif://www.onvif.org/hardware/factory", //需要读入
			"",
			"",
		},//char fixed_scopes[SCOPE_MAX_LEN][FIXED_SCOPE_LEN];
		{"","","","","","","",""},//char addtional_scopes[SCOPE_MAX_LEN][ADDTIONAL_SCOPE_LEN];
		6,//int fixed_len;
		0,//int add_len;
	},//vz_onvif_Scopes scopes;
//add by hm.liang start
	{
		"vision-zenith",// char name[STR_SHORT_LEN];
		0,// vz_boolean FromDHCP;
		0,// RebootNeeded
	},//vz_tds_Hostname
//add by hm.liang end

	{
		ONVIF_ONE_CHANNEL_INFO_DEFAULT,
		ONVIF_ONE_CHANNEL_INFO_DEFAULT,
		ONVIF_ONE_CHANNEL_INFO_DEFAULT,
		ONVIF_ONE_CHANNEL_INFO_DEFAULT,
		ONVIF_ONE_CHANNEL_INFO_DEFAULT,
		ONVIF_ONE_CHANNEL_INFO_DEFAULT,
		ONVIF_ONE_CHANNEL_INFO_DEFAULT,
		ONVIF_ONE_CHANNEL_INFO_DEFAULT,
	},//vz_onvif_Profiles
	{
		{
			ONVIF_RECORDING_JOB_CONFIG,
			ONVIF_RECORDING_JOB_CONFIG,
			ONVIF_RECORDING_JOB_CONFIG,
			ONVIF_RECORDING_JOB_CONFIG,
			ONVIF_RECORDING_JOB_CONFIG,
			ONVIF_RECORDING_JOB_CONFIG,
			ONVIF_RECORDING_JOB_CONFIG,
			ONVIF_RECORDING_JOB_CONFIG,
		},
		0,
	},//vz_onvif_RecordingJobs
	{	
		{
			ONVIF_RECORDING_CONFIG,
			ONVIF_RECORDING_CONFIG,
			ONVIF_RECORDING_CONFIG,
			ONVIF_RECORDING_CONFIG,
			ONVIF_RECORDING_CONFIG,
			ONVIF_RECORDING_CONFIG,
			ONVIF_RECORDING_CONFIG,
			ONVIF_RECORDING_CONFIG,
		},
		0,
	},//vz_onvif_Recordings
	ONVIF_EVENT,//vz_onvif_Event
	ONVIF_SUBSCRIPTION,//vz_onvif_Subscriptions
	{
		ONVIF_VIDEO_ANALY_DEFAULT,
		ONVIF_VIDEO_ANALY_DEFAULT,
		ONVIF_VIDEO_ANALY_DEFAULT,
		ONVIF_VIDEO_ANALY_DEFAULT,
	},//vz_onvif_VideoAnalytics VideoAnalytics[MAX_VIDEO_ANALYTICS_NUM];
	{
		{
			{
				"Receiver_0",
				0,
				vz_ReceiverState__NotConnected,
				ONVIF_RECEIVER_CONFIG_IPC_0,
			},
			{
				"Receiver_1",
				1,
				vz_ReceiverState__Connected,
				ONVIF_RECEIVER_CONFIG_V4L2_0,
			}
		},
		2,
	},//vz_onvif_Receivers Receivers;
	{
		{
			ONVIF_SERIAL_PORT,
			"SerialPorts1",	
		},
		{
			ONVIF_SERIAL_PORT,
			"SerialPorts2",
		},
	},//vz_onvif_SerialPorts
	{
		{
			ONVIF_OSD_CFG_DEFAULT,
			ONVIF_OSD_CFG_DEFAULT,
			ONVIF_OSD_CFG_DEFAULT,
			ONVIF_OSD_CFG_DEFAULT,
			ONVIF_OSD_CFG_DEFAULT,
			ONVIF_OSD_CFG_DEFAULT,
			ONVIF_OSD_CFG_DEFAULT,
			ONVIF_OSD_CFG_DEFAULT,
		},
		0,
	},//vz_onvif_OSDCfg
#ifdef NEW_ONVIF
	{
		ONVIF_PTZ_NODE_DEFAULT,
		ONVIF_PTZ_NODE_DEFAULT,
		ONVIF_PTZ_NODE_DEFAULT,
		ONVIF_PTZ_NODE_DEFAULT,
	},//VZ_PTZ_NODE
	{
		ONVIF_PTZ_CFG_DEFAULT,
		ONVIF_PTZ_CFG_DEFAULT,
		ONVIF_PTZ_CFG_DEFAULT,
		ONVIF_PTZ_CFG_DEFAULT,
	},//VZ_PTZ_CFG
	0,// Discoverable vz_onvif_DiscoveryMode
	{
		{
			{
				0,
				100,
			},//XRange
			{
				0,
				100,
			},//YRange
			{
				320,
				1280,
			},//WidthRange
			{
				240,
				720,
			},//HeightRange
		},
	},//vz_onvif_VSCfgOptions
	ONVIF_PTZCFG_OPTION_DEFAULT,
	ONVIF_OSD_OPTION_DEFAULT,
	"",//EndpointReference
	1,//need_auth
	1,//evt_sim_flag
	0,//log_enable
	0,//ip_chg_flag
	0,//reserved
	60,//evt_renew_time
	30,//evt_sim_interval
 #endif			
#ifdef ONVIF_USB_LAN
	{
		ONVIF_REMOTE_SERVER_INFO_DEFAULT,
		ONVIF_REMOTE_SERVER_INFO_DEFAULT,
		ONVIF_REMOTE_SERVER_INFO_DEFAULT,
		ONVIF_REMOTE_SERVER_INFO_DEFAULT
	},
#endif
		
};

static char* HwTypeName[] = {
  "AT2201",
  "VZ101",
  "X6446",
  "VZ2",
  "AT2201_INIVS",
  "AT2202",			//油田两路
  "VZ_201D",			//onvif(双网卡)
  "AT2201D",			//onvif(双网卡)
  "ZVAN_IIA_01",		//之维安环保智能视频检测器
  "TXCLZD-1",		//十所的普通版本
  "TXCLZD-3",		//十所的PTZ跟踪器/智能处理终端
  "X47B",			//海洋局的海面检测器
  "ZVAN_IIA_01D",	//之维安环保智能视频检测器(双网卡)
  "VZ-206",		//一体机
  "VZ-206-OD",	//支持开放开发的一体机
  "VZ-2032220SV",	//2路模拟SD卡ONVIF
  "VZ-2032221SV",	//2路模拟1路高清SD卡ONVIF
};



static const char * ONVIF_INFO = "onvif_info";
#define ONVIF_JSON_FILE		"/mnt/usr/onvif.jc"
#define ONVIF_CFG_FILE		"/mnt/usr/onvif.cfg"
#define ONVIF_CFG_SIZE sizeof(Onvif_Info)
Onvif_Info g_OnvifInfo;
NetworkPort g_netport;
NetworkInfo g_network;

unsigned int g_DeviceSupport = 0;

static OSD_config SYS_MEIDA_OSD_DEFAULT =
{
	1,/*dstampenable*/
	1,/*tstampenable*/
	0,/*logo enable*/
	1,/*nTextEnable*/
	DATEFORMAT_DEFAULT,
	TSTAMPFORMAT_DEFAULT,
	0,
	(((20<<16) & 0xFFFF0000) | (20 & 0xFFFF)),/*date position*/
	(((550<<16) & 0xFFFF0000) | (20 & 0xFFFF)),/*time position*/
	(((505<<16) & 0xFFFF0000) | (635 & 0xFFFF)),/*logo position	*/
	(((530<<16) & 0xFFFF0000) | (530 & 0xFFFF)),/*text position*/
	"oOSD Text",
};



////////////////////////////rebuild end///////////////////////////////////////////

extern vz__tds__DeviceInformation Onvif_DevInfo_Default;

static const char * NETWORK_PORT = "NetworkPort_Cfg";
static const char * NETWORK_INTERFACE = "NetworkInterface_Cfg";

Onvif_Info *GetOnvifInfo() {
  return &g_OnvifInfo;
}

NetworkPort *GetNetPort() {
  return &g_netport;
}

NetworkInfo *GetNetworkInfo() {
  return &g_network;
}

int UpdateNewOnvifDeviceInfo(void) {
	Onvif_Info* pOnvifInfo = GetOnvifInfo();
	if(pOnvifInfo == NULL)
	{
		printf("UpdateNewOnvifDeviceInfo GetOnvifInfo error!!!\n");
		return -1;
	}
	sprintf(pOnvifInfo->device_information.Manufacturer,"Vision Zenith");
	
	hw_info_t hwi;
	GetHwi(&hwi);
	int hw_type;
	
	char *p = (char *)&g_DeviceSupport;
	int ChannelMax = p[0];//Get_IvsRunState_channel_num();
	int AnalyTicsMax = p[3];//Get_IvsRunState_max_analytics_channel_num();
	int local_channel_num=p[1];//Get_IvsRunState_local_channel_num();
	int remote_channel_num=p[2];//Get_IvsRunState_remote_channel_num();

	if(hwi.write_status == HW_INFO_WRITTED)
	{
		sprintf(pOnvifInfo->device_information.SerialNumber,"%08x-%08x", hwi.serialno[0],hwi.serialno[1]);
		if(hwi.exdata_checker=='.')
		{
			sprintf(pOnvifInfo->device_information.FirmwareVersion,"%d.%d.%d.%d",hwi.hw_version[0],hwi.hw_version[1],hwi.hw_version[2],hwi.hw_version[3]);
			if(hwi.hw_version[0]==1 && hwi.hw_version[1]==0 && hwi.hw_version[2]==0 && hwi.hw_version[3]==1)
			{
				sprintf(pOnvifInfo->device_information.HardwareId,"%d_%d_%d_%d",ChannelMax,AnalyTicsMax,local_channel_num,remote_channel_num);
				sprintf(pOnvifInfo->device_information.Model,"%04x",hwi.hw_flag);
				hw_type = 0;
			}
			else
			{
				hw_type = hwi.hw_flag;
				sprintf(pOnvifInfo->device_information.HardwareId,"%d_%d_%d_%d",ChannelMax,AnalyTicsMax,local_channel_num,remote_channel_num);
				sprintf(pOnvifInfo->device_information.Model,"%04x",hwi.hw_version[3]);
			}
		}
		else 
		{
			//无ex信息
			hw_type = 0;
			sprintf(pOnvifInfo->device_information.FirmwareVersion,"1.0.0.1");
			sprintf(pOnvifInfo->device_information.HardwareId,"0");
			sprintf(pOnvifInfo->device_information.Model,"1");
		}
	}
	else
	{
		hw_type = -1;
		//工厂版本其他信息为空
		sprintf(pOnvifInfo->device_information.FirmwareVersion,"1.0.0.1");
		//工厂版本勉强填个序列号吧，onvifsdk还要用，但每台都一样了
		sprintf(pOnvifInfo->device_information.SerialNumber,"%08x-%08x", hwi.serialno[0],hwi.serialno[1]);
	}
	printf("hw_type = %d\n", hw_type);
	int typesize = sizeof(HwTypeName)/sizeof(char *);
	if((hw_type>=0)&&(hw_type < typesize)){ 
		printf("hw_type:%d,HwTypeName[hw_type]:%s\n",hw_type,HwTypeName[hw_type]);
		sprintf(pOnvifInfo->scopes.fixed_scopes[4],"onvif://www.onvif.org/hardware/%s-%d.%d.%d.%d",
			HwTypeName[hw_type],hwi.hw_version[0],hwi.hw_version[1],hwi.hw_version[2],hwi.hw_version[3]);
	}
	return 0;
}


int UpdateIPCStreamSupport(StreamInfo *stream_info,VideoSourceCfg *SourceCfg) {
  if(!stream_info || !SourceCfg)
    return -1;
  int source_channel=SourceCfg->userID;
  if(SourceCfg->enable && (SourceCfg->streamType == AVS_CAP_IPC)) {
    if(source_channel <0 || source_channel >= MAX_REMOTE_CHANNEL ) return -1;
    Onvif_Info *pOnvif_Info = GetOnvifInfo();
    if(strlen(stream_info->out_main_uri)) {
      pOnvif_Info->remote_server[source_channel].stream.main_stream.basic_config.supported
        = 1;
    } else {
      pOnvif_Info->remote_server[source_channel].stream.main_stream.basic_config.supported
        = 0;
    }

    if(strlen(stream_info->out_sub_uri)) {
      pOnvif_Info->remote_server[source_channel].stream.sub_stream.basic_config.supported
        = 1;
    } else {
      pOnvif_Info->remote_server[source_channel].stream.sub_stream.basic_config.supported
        = 0;
    }

    if(strlen(stream_info->out_alarm_uri)) {
      pOnvif_Info->remote_server[source_channel].stream.alarm_stream.supported = 1;
    } else {
      pOnvif_Info->remote_server[source_channel].stream.alarm_stream.supported = 0;
    }
  }
  return 0;
}


int UpdateStreamInfo(int channel, StreamInfo *pInfo) {
  if(channel <0 || channel >= MAX_CHANNEL ) return -1;
  Onvif_Info *pOnvifInfo = GetOnvifInfo();
  strcpy(pOnvifInfo->Profiles[channel].meta_profile[0].out_uri,
         pInfo->out_main_uri);
  strcpy(pOnvifInfo->Profiles[channel].meta_profile[1].out_uri,
         pInfo->out_sub_uri);
  strcpy(pOnvifInfo->Profiles[channel].meta_profile[3].out_uri,
         pInfo->out_alarm_uri);
  pOnvifInfo->Profiles[channel].meta_profile[0].frameFmt = pInfo->frameFmt;
  pOnvifInfo->Profiles[channel].meta_profile[1].frameFmt = pInfo->frameFmt;
  pOnvifInfo->Profiles[channel].meta_profile[3].frameFmt = pInfo->frameFmt;
  return 0;
}


int UpdateReceiverCfg(int channel, VideoSourceCfg *cfg) {
  if(!cfg)
    return -1;
  Onvif_Info *pOnvifInfo = GetOnvifInfo();

  if(cfg->enable) {
    int i=0;
    int index= pOnvifInfo->Receivers.Receiver_num;
    int modifyflag=0;//modify receiver or creat receiver
    for(i=0; i<pOnvifInfo->Receivers.Receiver_num; i++) {
      if(pOnvifInfo->Receivers.Receiver[i].channel == channel) {
        modifyflag =1;
        index = i;
        break;
      }
    }
    if(modifyflag == 0) { //creat
      int exist =0;
      int j=0;
      char strName[32]= {0};

      //find a available Receiver token
      do {
        exist =0;
        sprintf(strName, "Receiver_%d",j);
        for(i=0; i<pOnvifInfo->Receivers.Receiver_num; i++) {
          if(strcmp(strName,pOnvifInfo->Receivers.Receiver[i].Token)==0) {
            exist =1;
            j++;
            break;
          }
        }
      } while(exist);
      //strcpy(pOnvifInfo->Receivers.Receiver[index].Token,strName);
      pOnvifInfo->Receivers.Receiver[index].channel = channel;
      pOnvifInfo->Receivers.Receiver[index].state= vz_ReceiverState__Connected;
      pOnvifInfo->Receivers.Receiver[index].Config.Mode =
        vz_ReceiverMode__AlwaysConnect;
      pOnvifInfo->Receivers.Receiver[index].Config.StreamType=
        vz_StreamType__RTP_Unicast;
      pOnvifInfo->Receivers.Receiver[index].Config.Protocol=
        vz_TransportProtocol__RTSP;
      pOnvifInfo->Receivers.Receiver_num++;
    }
    strcpy(pOnvifInfo->Receivers.Receiver[index].Token,
           pOnvifInfo->Profiles[channel].meta_profile[0].VideoSourceConfig.token);
    vz_onvif__ReceiverConfig *pConfig
      =&pOnvifInfo->Receivers.Receiver[index].Config;
    memset(pConfig->MediaUri,0,sizeof(pConfig->MediaUri));
    if(cfg->streamType == AVS_CAP_V4L2) {
      sprintf(pConfig->MediaUri,"rtsp://v4l2%d/onvif/receiver",cfg->userID);

    } else if(cfg->streamType == AVS_CAP_IPC) {
      pOnvifInfo->Receivers.Receiver[index].state= vz_ReceiverState__NotConnected;
      vz_onvif_RemoteServerLoginConfig *pRemoteServer=&(pOnvifInfo->remote_server[(unsigned char)cfg->userID].connect.onvif_remote_server);
      sprintf(pConfig->MediaUri,"rtsp://ipc%d@%s:%d/onvif/receiver",cfg->userID,
              pRemoteServer->hostname,pRemoteServer->port);
    }

  } else {//delete receiver
    int i=0;
    vz_onvif_Receivers *pReceivers=&pOnvifInfo->Receivers;
    for(i=0; i<pReceivers->Receiver_num; i++) {
      if(pReceivers->Receiver[i].channel == channel) {
        strcpy(pReceivers->Receiver[i].Token,"");
        break;
      }
    }
    if(i!=pOnvifInfo->Receivers.Receiver_num) {
      //排序
      int last_empty_str_index=pReceivers->Receiver_num;
      int j=0;
      int add_len = 0;
      for(j=0; j<pReceivers->Receiver_num; j++) {
        if(strlen(pReceivers->Receiver[j].Token)!=0) {
          if(last_empty_str_index <j) {
            memcpy(&(pReceivers->Receiver[last_empty_str_index]),&(pReceivers->Receiver[j]),
                   sizeof(vz_onvif_Receiver));
            memset(&(pReceivers->Receiver[j]),0x0,sizeof(vz_onvif_Receiver));
            last_empty_str_index++;
          }
          add_len++;
        } else {
          if(last_empty_str_index >j)
            last_empty_str_index = j;
          memset(&(pReceivers->Receiver[j]),0x0,sizeof(vz_onvif_Receiver));
        }
      }
      //长度修正
      pReceivers->Receiver_num = add_len;
    }
  }
  return 0;
}


int UpdateRecordingConfig(int channel,Onvif_Info* pOnvifInfo,int enable) {
	int i=0;
	int flag=0;
	int index = 0;
	if(enable)//the channel is enable,change this channel recording config
	{
		index = pOnvifInfo->Recordings.Recording_num;
		for(i=0;i<pOnvifInfo->Recordings.Recording_num;i++)
		{
			if(pOnvifInfo->Recordings.Recording[i].channel == channel)
			{
				index = i;
				flag=1;
				break; 
			}
		}
		if(index < MAX_RECORD_NUM)
		{
			memset(&pOnvifInfo->Recordings.Recording[index],0,sizeof(vz_onvif_Recording));
			strcpy(pOnvifInfo->Recordings.Recording[index].RecordingToken,pOnvifInfo->Profiles[channel].meta_profile[0].VideoRecordConfig.token);
			strcpy(pOnvifInfo->Recordings.Recording[index].Config.Source.Name,pOnvifInfo->Profiles[channel].meta_profile[0].VideoSourceConfig.token);
			strcpy(pOnvifInfo->Recordings.Recording[index].Config.Source.Location,pOnvifInfo->Profiles[channel].meta_profile[0].VideoSourceConfig.token);
			pOnvifInfo->Recordings.Recording[index].Config.MaximumRetentionTimeFlag = 1;
			pOnvifInfo->Recordings.Recording[index].Config.MaximumRetentionTime = 10;
			pOnvifInfo->Recordings.Recording[index].channel = channel;
			sprintf(pOnvifInfo->Recordings.Recording[index].Config.Source.Description,"Recording_%d",channel);
			sprintf(pOnvifInfo->Recordings.Recording[index].Config.Content,"Recordingplace:1");
			if(flag==0)
			{
				pOnvifInfo->Recordings.Recording_num+=1;
			}
		}
	}
	else//the channel is disable,remove this channel recording config
	{
		if(pOnvifInfo->Recordings.Recording_num > 0)
		{
			for(i=0;i<pOnvifInfo->Recordings.Recording_num;i++)
			{
				if(pOnvifInfo->Recordings.Recording[i].channel == channel)
				{
					index = i;
					flag=1;
					break; 
				}
			}
			if(flag)
			{
				memset(&pOnvifInfo->Recordings.Recording[index],0,sizeof(vz_onvif_Recording));
				for(i=index;i<(pOnvifInfo->Recordings.Recording_num-1);i++)
				{
					memcpy(&pOnvifInfo->Recordings.Recording[i],&pOnvifInfo->Recordings.Recording[i+1],sizeof(vz_onvif_Recording));
					memset(&pOnvifInfo->Recordings.Recording[i+1],0,sizeof(vz_onvif_Recording));
				}
				pOnvifInfo->Recordings.Recording_num--;
			}
		}
	}
	return 0;
}

static int UpdateOnvifProfileInfo(Onvif_Info* pOnvifInfo,
                                  AVS_CAP_TYPE streamType,int channel,int stream) {
	char string[16]={0};
	if(stream == Stream_Main){
		strcpy(string,"vz_main");
	}
	else if(stream == Stream_Sub){
		strcpy(string,"vz_sub");
	}
	else if(stream == Stream_Jpeg){
		strcpy(string,"vz_jpeg");
	}
	else if(stream == AlarmInfo_Stream){
		strcpy(string,"vz_info");
	}
	
	sprintf(pOnvifInfo->Profiles[channel].meta_profile[stream].token,"%sProfile_%d",string,channel);
	sprintf(pOnvifInfo->Profiles[channel].meta_profile[stream].Name,"%sStream_%d",string,channel);

	pOnvifInfo->Profiles[channel].meta_profile[stream].VideoSourceConfig.Bounds.x=0;
	pOnvifInfo->Profiles[channel].meta_profile[stream].VideoSourceConfig.Bounds.y=0;
	pOnvifInfo->Profiles[channel].meta_profile[stream].VideoSourceConfig.Bounds.width=1280;
	pOnvifInfo->Profiles[channel].meta_profile[stream].VideoSourceConfig.Bounds.height=720;
	sprintf(pOnvifInfo->Profiles[channel].meta_profile[stream].VideoRecordConfig.token,"vz_recording_%d",channel);
	pOnvifInfo->Profiles[channel].meta_profile[stream].VideoRecordConfig.bEnable=0;
	pOnvifInfo->Profiles[channel].meta_profile[stream].VideoRecordConfig.eRecID=0;
	if(stream != AlarmInfo_Stream){
		sprintf(pOnvifInfo->Profiles[channel].meta_profile[stream].VideoEncoderConfig.token,"vz_video_encoder%d_%d",stream,channel);
		sprintf(pOnvifInfo->Profiles[channel].meta_profile[stream].VideoEncoderConfig.Name,"vz_video_encoder%d_%d",stream,channel);
		pOnvifInfo->Profiles[channel].meta_profile[stream].VideoEncoderConfig.Encoding=2;//h264
		pOnvifInfo->Profiles[channel].meta_profile[stream].VideoEncoderConfig.H264.GovLength=30;
		pOnvifInfo->Profiles[channel].meta_profile[stream].VideoEncoderConfig.H264.H264Profile=0;
		sprintf(pOnvifInfo->Profiles[channel].meta_profile[stream].AudioEncoderConfiguration.token,"vz_audio_encoder%d_%d",stream,channel);
		sprintf(pOnvifInfo->Profiles[channel].meta_profile[stream].AudioEncoderConfiguration.Name,"vz_audio_encoder%d_%d",stream,channel);
		sprintf(pOnvifInfo->Profiles[channel].meta_profile[stream].AudioSourceConfiguration.token,"vz_audio_sourcer_%d",channel);
		sprintf(pOnvifInfo->Profiles[channel].meta_profile[stream].AudioSourceConfiguration.Name,"vz_audio_source_%d",channel);
	}
	
	int cfg_id = 0;
	if( pOnvifInfo->VideoAnalytics[channel].used)
	{
		cfg_id = channel;
	}
	pOnvifInfo->Profiles[channel].meta_profile[stream].VA_ID = cfg_id;
	pOnvifInfo->VideoAnalytics[cfg_id].VACfg.UseCount++;
	pOnvifInfo->Profiles[channel].meta_profile[stream].fixed = 1;
	sprintf(pOnvifInfo->Profiles[channel].meta_profile[stream].VideoSourceConfig.token,"vz_video_source_%d",channel);
	sprintf(pOnvifInfo->Profiles[channel].meta_profile[stream].VideoSourceConfig.Name,"vz_video_source_%d",channel);

	if(streamType == AVS_CAP_V4L2){
		pOnvifInfo->Profiles[channel].meta_profile[stream].Type=stream;
	}
	else{
		pOnvifInfo->Profiles[channel].meta_profile[stream].Type=stream+Stream_Type_Max;
	}
#ifdef NEW_ONVIF
	int ptz_cfg_id = 0;
	if( pOnvifInfo->ptz_cfg[channel].used)
	{
		ptz_cfg_id = channel;
	}
	pOnvifInfo->Profiles[channel].meta_profile[stream].PTZCfg_id = ptz_cfg_id;
	pOnvifInfo->ptz_cfg[ptz_cfg_id].PTZConfiguration.UseCount++;
#endif
	return 0;
}


int Update_OneChannel_OnvifProfileInfo(StreamInfo *stream_info,Onvif_Info* pOnvifInfo,
                                       AVS_CAP_TYPE Type,int channel) {
  int ret = -1;
	if(!stream_info || !pOnvifInfo)
		return ret;
  if(channel <0 || channel >= MAX_CHANNEL)
    return ret;
	
	memset(&pOnvifInfo->Profiles[channel],0,sizeof(vz_onvif_Profiles));

  int count=0;
  if(strlen(stream_info->out_main_uri)) {
    UpdateOnvifProfileInfo(pOnvifInfo,Type,channel,Stream_Main);
    count++;
  }

  if(strlen(stream_info->out_sub_uri)) {
    UpdateOnvifProfileInfo(pOnvifInfo,Type,channel,Stream_Sub);
    count++;
  }

  if(strlen(stream_info->out_alarm_uri)) {
    UpdateOnvifProfileInfo(pOnvifInfo,Type,channel,AlarmInfo_Stream);
    count++;
  }

  int i=0;
  for(i=0; i<MAX_PROFILE_NUM; i++) {
		pOnvifInfo->Profiles[channel].meta_profile[i].VideoSourceConfig.UserCount = count;
  }

  UpdateRecordingConfig(channel,pOnvifInfo,count);

  int type = 0;
  for(type=0; type<3; type++) {
    Newonvif_init_OSD(channel,(vz_onvif_OSDTextType)type);
  }

  return ret;
}

int Update_SerialCfg(Onvif_Info* pOnvifInfo) {
  int i=0;
  int ret =0;
  int base =0;
#ifdef DM3730
  base = 0;
#else
  base = 1;
#endif
  for(i=0; i<MAX_SERIAL_PORTS_NUM; i++) {
    pOnvifInfo->SerialPorts[i].Config.Comm = i+base;
  }

  return ret;
}

int Newonvif_init_VA_cfg(Onvif_Info* pOnvifInfo) {
  int ret =0;
  int i = 0;
  for(i = 0; i < MAX_VIDEO_ANALYTICS_NUM; i++) {
    vz_onvif_VideoAnalyticsCfg * p_cfg = &pOnvifInfo->VideoAnalytics[i].VACfg;
    memset(p_cfg,0,sizeof(vz_onvif_VideoAnalyticsCfg));
    if(i == 0)
      pOnvifInfo->VideoAnalytics[i].used = 1;

    sprintf(p_cfg->Name, "VideoAnalytics%d",i);
    sprintf(p_cfg->token, "VideoAnalyticsToken_%03d",i);

    sprintf(p_cfg->AnalyticsEngineCfg.AnalyticsModule[0].Name,"alg");
    sprintf(p_cfg->AnalyticsEngineCfg.AnalyticsModule[0].Type,"alg_%d",i);
  }

  return ret;
}


#ifdef NEW_ONVIF

int Newonvif_init_OSD(int channel,vz_onvif_OSDTextType type) {
	Onvif_Info *pOnvif_Info = GetOnvifInfo();
	char strtype[8]={0};
	if(pOnvif_Info->OSDS.OSD_num< MAX_OSD_NUM-1)
	{
		//SysInfo *pSysInfo = GetSysInfo();
		VideoSourceCfg testcfg;
		testcfg.enable = 1;
		testcfg.enableAlg = 1; 
		testcfg.userID = 0; 	
		testcfg.algId = 0;	 
		testcfg.streamType = AVS_CAP_V4L2;
		 OSD_config testosd;
		 memcpy(&testosd,&SYS_MEIDA_OSD_DEFAULT,sizeof(testosd));
		
		if(testcfg.streamType == AVS_CAP_V4L2)
		{
			vz_onvif_OSDCfg * p_OSDCfg = &pOnvif_Info->OSDS.OSD[pOnvif_Info->OSDS.OSD_num].OSDCfg;
			pOnvif_Info->OSDS.OSD[pOnvif_Info->OSDS.OSD_num].used =1;
			pOnvif_Info->OSDS.OSD[pOnvif_Info->OSDS.OSD_num].channel =channel;
			p_OSDCfg->TextStringFlag = 1;
			p_OSDCfg->Type = vz_OSDType_Text;
		
			int localID = testcfg.userID;
			OSD_config *p_osd = &testosd;//pSysInfo->media[localID].osd;
			switch(type)
			{
			case vz_OSDTextType_Plain:
				strcpy(strtype,"Plain");
				p_OSDCfg->TextString.PlainTextFlag =1;
				p_OSDCfg->TextString.PlainEnableFlag =p_osd->nTextEnable;
				strcpy(p_OSDCfg->TextString.PlainText,p_osd->overlaytext);
				p_OSDCfg->Position.Type =vz_OSDPosType_Custom;
				p_OSDCfg->Position.PosFlag = 1;
				p_OSDCfg->Position.Pos.x= (p_osd->nTextPosition&0xFFFF0000)>>16;
				p_OSDCfg->Position.Pos.y= (p_osd->nTextPosition&0xFFFF);
				break;
			case vz_OSDTextType_Date:
				strcpy(strtype,"Date");
				p_OSDCfg->TextString.DateEnableFlag=p_osd->dstampenable;
				p_OSDCfg->TextString.DateFormatFlag=1;
				if(p_osd->dateformat == 0)
				{
					strcpy(p_OSDCfg->TextString.DateFormat,"YYYY/MM/DD");
				}
				else if(p_osd->dateformat == 1)
				{
					strcpy(p_OSDCfg->TextString.DateFormat,"MM/DD/YYYY");
				}
				else if(p_osd->dateformat == 2)
				{
					strcpy(p_OSDCfg->TextString.DateFormat,"DD/MM/YYYY");
				}
				else
				{
					strcpy(p_OSDCfg->TextString.DateFormat,"YYYY/MM/DD");
				}
				p_OSDCfg->Position.Type =vz_OSDPosType_Custom;
				p_OSDCfg->Position.PosFlag = 1;
				p_OSDCfg->Position.Pos.x= (p_osd->dateposition&0xFFFF0000)>>16;
				p_OSDCfg->Position.Pos.y= (p_osd->dateposition&0xFFFF);
				break;
			case vz_OSDTextType_Time:
				strcpy(strtype,"Time");
				p_OSDCfg->TextString.TimeEnableFlag=1;
				p_OSDCfg->TextString.TimeFormatFlag=1;
				if(p_osd->tstampformat == 0)
				{
					strcpy(p_OSDCfg->TextString.TimeFormat,"hh:mm:ss tt");
				}
				else
				{
					strcpy(p_OSDCfg->TextString.TimeFormat,"HH:mm:ss");
				}
				p_OSDCfg->Position.Type =vz_OSDPosType_Custom;
				p_OSDCfg->Position.PosFlag = 1;
				p_OSDCfg->Position.Pos.x= (p_osd->timeposition&0xFFFF0000)>>16;
				p_OSDCfg->Position.Pos.y= (p_osd->timeposition&0xFFFF);
				break;
			default:
				break;
			}
			p_OSDCfg->TextString.Type =type;
			
			sprintf(p_OSDCfg->token, "OSD_%d_%s",channel,strtype);
			strcpy(p_OSDCfg->VideoSourceConfigurationToken, pOnvif_Info->Profiles[channel].meta_profile[0].VideoSourceConfig.token);
			pOnvif_Info->OSDS.OSD_num++;
		}
		else
		{//删除OSD.channel = channel的数据，等获取转发的osds 来写入OSD.channel = channel的数据
			int i = 0;
			int numbefore = pOnvif_Info->OSDS.OSD_num;
			for(i = 0;i<numbefore;i++)
			{
				if(pOnvif_Info->OSDS.OSD[i].used
					&&(pOnvif_Info->OSDS.OSD[i].channel == channel))
				{
					memset(&pOnvif_Info->OSDS.OSD[i],0,sizeof(VZ_ONVIF_OSD));
					pOnvif_Info->OSDS.OSD_num --;
				}
			}
		}
	}
	//will save sysinfo & onvif_info the same time
	return 0;
}

int Newonvif_init_ptz_node(Onvif_Info* pOnvifInfo) {
  int ret = 0;
  int i = 0;
  for(i = 0; i < MAX_PTZ_NODE_NUM; i++) {
    vz_onvif_PTZNode * p_node = &pOnvifInfo->ptz_node[i].PTZNode;
    if(i == 0)
      pOnvifInfo->ptz_node[i].used = 1;

    sprintf(p_node->Name, "PTZNode%d",i);
    sprintf(p_node->token, "NODE_%03d",i);
  }

  return ret;
}


/**
 * init ptz configuration
 */
int Newonvif_init_ptz_cfg(Onvif_Info* pOnvifInfo) {
  int i = 0;
  int ret = 0;
  for(i = 0; i < MAX_PTZ_CFG_NUM; i++) {
    vz_onvif_PTZConfiguration * p_cfg = &pOnvifInfo->ptz_cfg[i].PTZConfiguration;
    if(i == 0)
      pOnvifInfo->ptz_cfg[i].used = 1;

    sprintf(p_cfg->Name, "PTZConfig%d",i);
    sprintf(p_cfg->token, "PTZCFGToken_%03d",i);
    if(pOnvifInfo->ptz_node[i].used) {
      strcpy(p_cfg->NodeToken,pOnvifInfo->ptz_node[i].PTZNode.token);
    }

  }
  return ret;
}

int Newonvif_init_EndpointReference(Onvif_Info *pOnvifInfo) {
  static char uuid[100];

  srand((unsigned int)time(NULL));
  sprintf(uuid, "%04x%04x-%04x-%04x-%04x-%04x%04x%04x",
          rand()%0xFFFF, rand()%0xFFFF, rand()%0xFFFF, rand()%0xFFFF,
          rand()%0xFFFF, rand()%0xFFFF, rand()%0xFFFF, rand()%0xFFFF);
  strncpy(pOnvifInfo->EndpointReference, uuid,
          sizeof(pOnvifInfo->EndpointReference)-1);
  return 0;
}


#endif

//网络启动起来后更新
int UpdateOnvifState() {
  //SysInfo* pSysInfo = GetSysInfo();
  char buffer[512]= {0};
  int returnsize = Kvdb_GetKeyAbsolutelyToBuffer(NETWORK_PORT,strlen(NETWORK_PORT),buffer,
                                       sizeof(buffer));
  printf("%s,%d,buffer=%s,!!!!!!!!!!\n",__FUNCTION__,__LINE__,buffer);
  if(returnsize <= 0) {
    return -1;
  }

  if(ParserNetPort(buffer,returnsize,&g_netport) < 0) {
    return -1;
  }

  returnsize = Kvdb_GetKeyAbsolutelyToBuffer(NETWORK_INTERFACE,strlen(NETWORK_INTERFACE),buffer,
                                       sizeof(buffer));
  if(returnsize <= 0) {
    return -1;
  }

  if(ParserNetInterface(buffer,returnsize,&g_network) < 0) {
    return -1;
  }

  Onvif_Info* pOnvifInfo = GetOnvifInfo();

  char end_point[30];
  if(g_netport.http_port==80) {
    sprintf(end_point,"%s",inet_ntoa(g_network.ip));
  } else {
    sprintf(end_point,"%s:%d",inet_ntoa(g_network.ip),
            g_netport.http_port);
  }
	//sprintf(end_point,"%s:8000",inet_ntoa(g_network.ip));
	
  sprintf(pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_DEVICE_SERVICE].XAddr,
          "http://%s/onvif/device_service",end_point);
  sprintf(pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_MEDIA].XAddr,
          "http://%s/onvif/Media",end_point);
  sprintf(pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_EVENTS].XAddr,
          "http://%s/onvif/Events",end_point);
  sprintf(pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_PTZ].XAddr,
          "http://%s/onvif/PTZ",end_point);
  sprintf(pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_IMAGING].XAddr,
          "http://%s/onvif/Imaging",end_point);
  sprintf(pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_DEVICEIO].XAddr,
          "http://%s/onvif/DeviceIO",end_point);
  sprintf(pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_ANALYTICS].XAddr,
          "http://%s/onvif/Analytics",end_point);
  sprintf(pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_DISPLAY].XAddr,
          "http://%s/onvif/Display",end_point);
  sprintf(pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_RECORDING].XAddr,
          "http://%s/onvif/Recording",end_point);
  sprintf(pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_SEARCH].XAddr,
          "http://%s/onvif/Search",end_point);
  sprintf(pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_REPLAY].XAddr,
          "http://%s/onvif/Replay",end_point);
  sprintf(pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_RECEIVER].XAddr,
          "http://%s/onvif/Receiver",end_point);
  sprintf(pOnvifInfo->services.info.service[ONVIF_SERVICE_TYPE_ANALYTICSDEVICE].XAddr,
          "http://%s/onvif/Analyticsdevice",end_point);
  int ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
                        sizeof(Onvif_Info));
  return ret;
}


//模拟获取AV server StreamInfo
int  GetAvStreamInfo(StreamInfo *info,int ChnId)
{
	if(info == NULL || ChnId < 0)
		return -1;

	memset(info, 0, sizeof(StreamInfo));

	info->frameFmt = FMT_VID_CHNMAIN_0 + ChnId*2;

	const char *pStrPrefix = "rtsp://VisionZenith:147258369@192.168.1.100:8557";

#if 0
	sprintf(info->out_main_uri, "%s/video_channel=%d_stream=0", pStrPrefix, ChnId);
	sprintf(info->out_sub_uri, "%s/video_channel=%d_stream=1", pStrPrefix, ChnId);
	sprintf(info->out_alarm_uri, "%s/vzinfo=%d", pStrPrefix, ChnId);
#else
	if(ChnId == 0)
	{
		sprintf(info->out_main_uri, "%s/h264", pStrPrefix);
//		sprintf(info->out_sub_uri, "%s/video_channel=%d_stream=1", pStrPrefix, ChnId);
//		sprintf(info->out_alarm_uri, "%s/vzinfo=%d", pStrPrefix, ChnId);
	}
	else if(ChnId == 1)
	{
		sprintf(info->out_main_uri, "%s/h264_ch2", pStrPrefix);
		sprintf(info->out_sub_uri, "%s/video_channel=%d_stream=1", pStrPrefix, ChnId);
		sprintf(info->out_alarm_uri, "%s/vzinfo=%d", pStrPrefix, ChnId);
	}

#endif
	return 0;
}

int onvif_init_cfg_byDev(Onvif_Info *pOnvifInfo) {
  int ret = 0;
#ifdef NEW_ONVIF
  Newonvif_init_EndpointReference(pOnvifInfo);
  Newonvif_init_ptz_node(pOnvifInfo);
  Newonvif_init_ptz_cfg(pOnvifInfo);
#endif
  Newonvif_init_VA_cfg(pOnvifInfo);
  Update_SerialCfg(pOnvifInfo);
	//init after system server /av server started
	
	char *p = (char *)&g_DeviceSupport;
	p[0] = 1;//Get_IvsRunState_channel_num();
	p[1] = 1;//Get_IvsRunState_local_channel_num();
	p[2] = 1;//Get_IvsRunState_remote_channel_num();
	p[3] = 1;//Get_IvsRunState_max_analytics_channel_num();
	VideoSourceCfg testcfg;
	testcfg.enable = 1;
	testcfg.enableAlg = 1; 
	testcfg.userID = 0;		
	testcfg.algId = 0; 	 
	testcfg.streamType = AVS_CAP_V4L2;

  int channel = 0;
	//对ipc视频的一些初始化
	for(channel=0;channel<p[2] && channel<MAX_REMOTE_CHANNEL;channel++){
		sprintf(pOnvifInfo->remote_server[channel].video.VideoSourceToken,"vz_Remote_source_%d",channel);
	}
	
  for(channel=0; channel<p[0]&& channel<MAX_CHANNEL; channel++) {
    StreamInfo stream_info;
		//SysInfo *pSysInfo = GetSysInfo();
		VideoSourceCfg* pSourceConfig = &testcfg;//pSysInfo->source_config[channel];
    //更新onvif视频参数
    memset(&stream_info,0x0,sizeof(StreamInfo));
		
    if(GetAvStreamInfo(&stream_info,channel) >= 0) {
      Update_OneChannel_OnvifProfileInfo(&stream_info,pOnvifInfo,
                                          pSourceConfig->streamType,channel);
      UpdateStreamInfo(channel,&stream_info);
      UpdateIPCStreamSupport(&stream_info,pSourceConfig);
      UpdateReceiverCfg(channel,pSourceConfig);
    }
  }

	UpdateNewOnvifDeviceInfo();
	UpdateOnvifState();
  ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)pOnvifInfo,
                    sizeof(Onvif_Info));
  return ret;
}



int onvif_init_cfg() {
  //first read from the kvdb,failed then read from the cfg&write to the kvdb
  int ret = 0;
  //first read from the kvdb
		LOG_INFO("ONVIF_INFO size = %d", strlen(ONVIF_INFO));
  if(Kvdb_GetKeyAbsolutelyToBuffer(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
                         sizeof(Onvif_Info)) != sizeof(Onvif_Info)) {
		LOG_INFO("ONVIF_INFO size = %d", sizeof(Onvif_Info));
    //failed then read from the cfg(json file),write to kvdb
    //set to default if json dont has the value
    memcpy(&g_OnvifInfo, &Onvif_Info_Default, ONVIF_CFG_SIZE);
    ret = JsonRead_OnvifInfo(ONVIF_JSON_FILE,&g_OnvifInfo);
    if(ret < 0) { // read from the cfg (bin file),write to kvdb&delete the bin file
      char buffer[256]= {0};
      sprintf(buffer,"./updatecfg2json\n");
      system(buffer);
      ret = JsonRead_OnvifInfo(ONVIF_JSON_FILE,&g_OnvifInfo);
      if(ret < 0) {
        memcpy(&g_OnvifInfo, &Onvif_Info_Default, ONVIF_CFG_SIZE);
      } else {
        remove(ONVIF_CFG_FILE);
      }
    }/*else {
			remove(ONVIF_JSON_FILE);
    }*/
		LOG_INFO("ONVIF_INFO size = %d", sizeof(Onvif_Info));
    ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
                      sizeof(Onvif_Info));
  }

  ret = onvif_init_cfg_byDev(&g_OnvifInfo);


  return ret;
}

int DoOnvifSetScopes(vz_onvif_Scopes *value)
{
  int ret = 0;
	memcpy(&g_OnvifInfo.scopes,value,sizeof(vz_onvif_Scopes));
	ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
										sizeof(Onvif_Info));
	return ret;
}

int DoOnvifSetRecordings(vz_onvif_Recordings *value)
{
  int ret = 0;
	memcpy(&g_OnvifInfo.Recordings,value,sizeof(vz_onvif_Recordings));
	ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
										sizeof(Onvif_Info));
	return ret;
}

int DoOnvifSetRecordingJobs(vz_onvif_RecordingJobs *value)
{
  int ret = 0;
	memcpy(&g_OnvifInfo.RecordingJobs,value,sizeof(vz_onvif_RecordingJobs));
	ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
										sizeof(Onvif_Info));
	return ret;
}

int DoOnvifSetSubscription(vz_onvif_Subscriptions *value)
{
	int ret = 0;
	memcpy(&g_OnvifInfo.Subscriptions,value,sizeof(vz_onvif_Subscriptions));
	ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
										sizeof(Onvif_Info));
	return ret;
}

int DoOnvifSetProfiles(vz_onvif_Profiles *value,int index)
{
	int ret = 0;
	memcpy(&g_OnvifInfo.Profiles[index],value,sizeof(vz_onvif_Profiles));
	ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
										sizeof(Onvif_Info));
	return ret;
}


int DoOnvifSetEvent(vz_onvif_Event *value)
{
	int ret = 0;
	memcpy(&g_OnvifInfo.Event,value,sizeof(vz_onvif_Event));
	ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
										sizeof(Onvif_Info));
	return ret;
}

int DoOnvifSetRVideoInfo(vz_onvif_RemoteVideoInfo *value,int index)
{
	int ret = 0;
	memcpy(&g_OnvifInfo.remote_server[index].video,value,sizeof(vz_onvif_RemoteVideoInfo));
	ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
										sizeof(Onvif_Info));
	return ret;
}

int DoOnvifSetOSD(VZ_ONVIF_OSDs *value)
{
	int ret = 0;
	memcpy(&g_OnvifInfo.OSDS,value,sizeof(VZ_ONVIF_OSDs));
	ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
										sizeof(Onvif_Info));
	return ret;
}

int DoOnvifSetVideoAnalytics(vz_onvif_VideoAnalytics *value,int index)
{
	int ret = 0;
	memcpy(&g_OnvifInfo.VideoAnalytics[index],value,sizeof(vz_onvif_VideoAnalytics));
	ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
										sizeof(Onvif_Info));
	return ret;
}

int DoOnvifSetReceiver(vz_onvif_Receivers *value)
{
	int ret = 0;
	memcpy(&g_OnvifInfo.Receivers,value,sizeof(vz_onvif_Receivers));
	ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
										sizeof(Onvif_Info));
	return ret;
}

int DoOnvifSetSerialPortsCfg(vz_onvif_SerialPortsCfg *value,int index)
{
	int ret = 0;
	memcpy(&g_OnvifInfo.SerialPorts[index].Config,value,sizeof(vz_onvif_SerialPortsCfg));
	ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
										sizeof(Onvif_Info));
	return ret;
}

int DoOnvifSetDiscoveryMode(vz_onvif_DiscoveryMode *value)
{
	int ret = 0;
	g_OnvifInfo.DiscoveryMode = *value;
	ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
										sizeof(Onvif_Info));
	return ret;
}

int DoOnvifSetPTZCfg(vz_onvif_PTZConfiguration *value,int index)
{
	int ret = 0;
	memcpy(&g_OnvifInfo.ptz_cfg[index].PTZConfiguration,value,sizeof(vz_onvif_PTZConfiguration));
	ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
										sizeof(Onvif_Info));
	return ret;
}


int DoOnvifSetHostName(vz_tds_Hostname *value)
{
	int ret = 0;
	memcpy(&g_OnvifInfo.hostname,value,sizeof(vz_tds_Hostname));
	ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
										sizeof(Onvif_Info));
	return ret;
}

int DoOnvifSetRemoteRtspPort(int value,int index)
{
	int ret = 0;
	g_OnvifInfo.remote_server[index].connect.remote_rtsp_port = value;
	ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
										sizeof(Onvif_Info));
	return ret;
}

int DoOnvifSetRServiceInfo(vz_onvif_ServiceShortInfo *value,int index)
{
	int ret = 0;
	memcpy(&g_OnvifInfo.remote_server[index].service.info,value,sizeof(vz_onvif_ServiceShortInfo));
	ret = Kvdb_SetKey(ONVIF_INFO,strlen(ONVIF_INFO),(char *)&g_OnvifInfo,
										sizeof(Onvif_Info));
	return ret;
}

/*
int UpdateNetmork_byPara(unsigned int *pdata) {
  if (pdata == NULL)
    return -1;
  g_network.ip.s_addr = pdata[0];
  g_network.netmask.s_addr = pdata[1];
  g_network.gateway.s_addr = pdata[2];
  return 0;
}

int UpdateNetport() {
  char buffer[512]= {0};
  int returnsize = Kvdb_GetKeyAbsolutelyToBuffer(NETWORK_PORT,strlen(NETWORK_PORT),buffer,
                                       sizeof(buffer));
  if(returnsize <= 0) {
    return -1;
  }

  if(ParserNetPort(buffer,returnsize,&g_netport) < 0) {
    return -1;
  }
  return 0;
}
*/
void UpdateNewOnvifIPFlag(unsigned int *pdata) {
  onvif_modify_localip(pdata);

	g_OnvifInfo.ip_chg_flag = 1;
}


/*
int DoOnvifSetScopes(vz_onvif_Scopes *value) {
  return SetOnvifScopes_todb(value);
}

#ifdef ONVIF_USB_LAN



int DoSetOnvifIvsStream_todb(OnvifIvsStreamConfig *pvalue,int remote_channel) {
  vz_onvif_RemoteStreamConfig Stream;
  if(GetOnvifRemoteServerStream_bydb(&Stream,remote_channel) < 0) {
    return -1;
  }
  memcpy(Stream.sub_stream.basic_config.profile_token, pvalue->profile_token,
         PROFILE_TOKEN_LEN);
  memcpy(Stream.sub_stream.encoder_config.video_encoder_config_token,
         pvalue->video_encoder_config_token,PROFILE_TOKEN_LEN);
  memcpy(Stream.sub_stream.encoder_config.video_encoder_config_name,
         pvalue->video_encoder_config_name,PROFILE_TOKEN_LEN);
  Stream.sub_stream.encoder_config.encoding = pvalue->encoding;
  Stream.sub_stream.encoder_config.framerate = pvalue->framerate;
  Stream.sub_stream.encoder_config.height = pvalue->height;
  Stream.sub_stream.encoder_config.width = pvalue->width;
  memcpy(Stream.sub_stream.basic_config.in_uri,pvalue->stream_uri,MAX_URI_LEN);

  return SetOnvifRemoteServerStream_todb(&Stream,remote_channel);
}

int DoSetOnvifIvsStreamConfig(OnvifIvsStreamConfig* value,int remote_channel) {
  //防止溢出
  value->stream_uri[MAX_URI_LEN-1] = '\0';
  DoSetOnvifIvsStreamToAV(value->stream_uri,remote_channel);
  return DoSetOnvifIvsStream_todb(value,remote_channel);
}

int DoSetOnvifMainStreamUri_todb(char* pvalue,int remote_channel) {
  vz_onvif_RemoteStreamConfig Stream;
  if(GetOnvifRemoteServerStream_bydb(&Stream,remote_channel) < 0) {
    return -1;
  }
  strcpy(Stream.main_stream.basic_config.in_uri, pvalue);

  return SetOnvifRemoteServerStream_todb(&Stream,remote_channel);
}

int DoSetOnvifMainStreamUri(char* stream_uri,int remote_channel) {
  printf("DoSetOnvifMainStreamUri:%s\n",stream_uri);
  DoSetOnvifMainStreamUriToAV(stream_uri,remote_channel);
  return DoSetOnvifMainStreamUri_todb(stream_uri,remote_channel);
}


int DoSetOnvifMainStreamProfileToken_todb(char* pvalue,int remote_channel) {
  vz_onvif_RemoteStreamConfig Stream;
  if(GetOnvifRemoteServerStream_bydb(&Stream,remote_channel) < 0) {
    return -1;
  }
  strcpy(Stream.main_stream.basic_config.profile_token, pvalue);
  return SetOnvifRemoteServerStream_todb(&Stream,remote_channel);
}

//当改变profiletoken，上次的streamUri变为无效
int DoSetOnvifMainStreamProfileToken(char* profile_token,int remote_channel) {
  printf("DoSetOnvifMainStreamProfileToken:%s\n",profile_token);
  DoSetOnvifMainStreamUriToAV("",remote_channel);
  return DoSetOnvifMainStreamProfileToken_todb(profile_token,remote_channel);
}


//重设所有命令状态，重设连接状态
int DoResetRemoteConnectStatus(int channel) {
  printf("DoResetRemoteConnectStatus channel: %d\n",channel);
  if(channel<0 || channel>= MAX_REMOTE_CHANNEL) return -1;
  unsigned char connect_status = vz_ReceiverState__NotConnected;
  int msg[2]= {5,0}; //{MAX_CONFIG_TYPE,CONFIG_STATUS_NEED_UPDATE};
  DpClient_SendDpMessage(SYS_SRV_SET_REMOTE_IVSSTATE,channel, (const char *)msg,
                         sizeof(msg));

  return SetOnvifRemoteServerConnState_todb(&connect_status,channel);
}

int DoSetRemoteConnectStatus(int channel,unsigned char *value) {
  if(channel<0 || channel>= MAX_REMOTE_CHANNEL) return -1;
  printf("DoSetRemoteConnectStatus channel: %d ,%d\n",channel,*value);
  if(*value == vz_ReceiverState__NotConnected) {
    int msg[2]= {2,0}; //{CONFIG_TYPE_CAPABILITIES,CONFIG_STATUS_NEED_UPDATE};
    DpClient_SendDpMessage(SYS_SRV_SET_REMOTE_IVSSTATE,channel, (const char *)msg,
                           sizeof(msg));
  }

  return SetOnvifRemoteServerConnState_todb(value,channel);
}

int DoSetOnvifRemoteSnapshotUri(char* uri,int remote_channel) {
  vz_onvif_JpgMng_url Info;
  strncpy(Info.JpgUrl,uri,256);
  return SetOnvifRemoteServerJpgUrl_todb(&Info, remote_channel);
}

int DoOnvifSetRemoteServer(int channel,
                           vz_onvif_RemoteServerLoginConfig *value) {
  //maybe will relogin the remote server here
  if(channel<0 || channel>= MAX_REMOTE_CHANNEL) return -1;
  //reset the ivs_stream_config
  OnvifIvsStreamConfig streamConfig;
  memset(&streamConfig,0,sizeof(OnvifIvsStreamConfig));
  DoSetOnvifIvsStreamConfig(&streamConfig,channel);
  DoSetOnvifMainStreamProfileToken("",channel);
  DoResetRemoteConnectStatus(channel);
  DoSetIPCTimeDiffEnableToAV(channel,value->TimeDiffEnable);
  return SetOnvifRemoteServerConnect_todb(value,channel);
}

int DoOnvifSetRemoteAudioSupport(int channel,int *value) {
  return SetOnvifRemoteServerAudioSupport_todb(value,channel);
}



#endif


void GetKvdbCallback(const char *key,int key_size, const char *value,
                     int value_size,void* user_data) {
  if((user_data == NULL)||(key ==NULL)||(value==NULL)||(key_size <= 0)
     ||(value_size <= 0))
    return;

  if(strcmp(key,NETWORK_PORT)==0) {

  }
}




int UpdateOnvifShortInfo(void) {
  __u16 http_port;
  if(OnvifDisp_SendRequestSync(SYS_SRV_GET_HTTPPORT, NULL, 0,&http_port,
                               sizeof(http_port))< 0)
    return -1;
  struct in_addr	ip;
  if(OnvifDisp_SendRequestSync(SYS_SRV_GET_IP, NULL, 0,&ip, sizeof(ip))< 0)
    return -1;

  char end_point[30];

  if(http_port==80) {
    sprintf(end_point,"%s",inet_ntoa(ip));
  } else {
    sprintf(end_point,"%s:%d",inet_ntoa(ip),http_port);
  }
  vz_onvif_ServiceShortInfo info;
  sprintf(info.service[ONVIF_SERVICE_TYPE_DEVICE_SERVICE].XAddr,
          "http://%s/onvif/device_service",end_point);
  sprintf(info.service[ONVIF_SERVICE_TYPE_MEDIA].XAddr,"http://%s/onvif/Media",
          end_point);
  sprintf(info.service[ONVIF_SERVICE_TYPE_EVENTS].XAddr,"http://%s/onvif/Events",
          end_point);
  sprintf(info.service[ONVIF_SERVICE_TYPE_PTZ].XAddr,"http://%s/onvif/PTZ",
          end_point);
  sprintf(info.service[ONVIF_SERVICE_TYPE_IMAGING].XAddr,
          "http://%s/onvif/Imaging",end_point);
  sprintf(info.service[ONVIF_SERVICE_TYPE_DEVICEIO].XAddr,
          "http://%s/onvif/DeviceIO",end_point);
  sprintf(info.service[ONVIF_SERVICE_TYPE_ANALYTICS].XAddr,
          "http://%s/onvif/Analytics",end_point);
  sprintf(info.service[ONVIF_SERVICE_TYPE_DISPLAY].XAddr,
          "http://%s/onvif/Display",end_point);
  sprintf(info.service[ONVIF_SERVICE_TYPE_RECORDING].XAddr,
          "http://%s/onvif/Recording",end_point);
  sprintf(info.service[ONVIF_SERVICE_TYPE_SEARCH].XAddr,"http://%s/onvif/Search",
          end_point);
  sprintf(info.service[ONVIF_SERVICE_TYPE_REPLAY].XAddr,"http://%s/onvif/Replay",
          end_point);
  sprintf(info.service[ONVIF_SERVICE_TYPE_RECEIVER].XAddr,
          "http://%s/onvif/Receiver",end_point);
  sprintf(info.service[ONVIF_SERVICE_TYPE_ANALYTICSDEVICE].XAddr,
          "http://%s/onvif/Analyticsdevice",end_point);
  //DpClient_SendDpRequest(ONVIF_SRV_SET_SHORTINFO, 0,(const char *)&info,sizeof(info),NULL,NULL, 5);
  return SetOnvifServiceShortInfo_todb(&info);
  //return 0;
}

*/





