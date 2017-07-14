//一些公有的符号定义
#ifndef _VZ_LOCALSDK_DATADEFINE_H_
#define _VZ_LOCALSDK_DATADEFINE_H_


#include <asm/types.h>
#include <netinet/in.h>
#include "onvif_env.h"

#define VZ_LOCALAPI_SUCCESS			0
#define VZ_LOCALAPI_FAILED			-1

/************************************************************************

ERROR define

************************************************************************/
#define  E_SUCCESS   0		//Success
#define  E_INVAL     1		//Invalid parameters
#define  E_NOENT     2		//No such file or directory
#define  E_IO        3		//I/O Error
#define  E_NOMEM     4		//Out of memory

#define HOST_NAME_LEN 33
#define VZ_MANUAL_IP_MAX_NUM 8 /*manual ip max number*/
#define VZ_NETWORK_INTERFACE_MAX_NUM  8/*max Network Interfaces number*/
#define VZ_IP_ADDR_LEN 20		/*IPv4Address string length*/
#define VZ_DNS_MAX_NUM 2		/*max DNS number*/
#define VZ_MAC_STR_LEN 20		/*mac string length*/
#define VZ_LIST_MAX_NUM 8    /*all kinds of list max number*/
#define VZ_PASSWORD_MIN_NUM 6    /*user password min length*/
#define VZ_STR_SHORT_LEN 128    /*short string length*/
#define VZ_TOKEN_STR_LEN 64    /*token string length*/
#define VZ_TZ_STR_LEN 64    /*TimeZone string length*/
#define VZ_PTZ_PRESET_NUM                       128
#define VZ_PTZ_PRESET_NAMELEN                   8
#define VZ_PTZ_CRUISE_NUM 4
#define VZ_OSD_TEXT_LEN				16
#define VZ_IMAGING_OPTION_MIN				0
#define VZ_IMAGING_OPTION_MAX				100
#define VZ_RESOLUTION_MAX_NUM VZ_LIST_MAX_NUM*2    
#define VZ_EITEM_ANY_MAX_SIZE 2048    


typedef  enum 
{
	VZ_LOCALAPI_TZ_ERR =1,					/*InvalidTimeZone*/
	VZ_LOCALAPI_DATETIME_ERR,			/*:InvalidDateTime*/
	VZ_LOCALAPI_NTP_ERR,		/*NtpServerUndefined*/
	VZ_LOCALAPI_ACTION_FAIL,					/*operate failed*/
	VZ_LOCALAPI_ACTION_UNSUPPORT,					/*operate unsupported*/
	VZ_LOCALAPI_SCOPE_OVERWRITE,		/*Scope parameter overwrites*/
 	VZ_LOCALAPI_SCOPELIST_EXCEED,			/*TooManyScopes*/
	VZ_LOCALAPI_SCOPE_NONEXIST,			/*scope which does not exist*/
	VZ_LOCALAPI_SCOPE_EXCEED,			/*ONE Scope string is too long*/
	VZ_LOCALAPI_USERNAME_CLASH,		/*Username already exists*/
	VZ_LOCALAPI_USERNAME_LONG,			/*The username is too long*/
	VZ_LOCALAPI_USERNAME_SHORT,		/*The username is too short*/
	VZ_LOCALAPI_USER_EXCEED,			/*Maximum number of supported users exceeded.*/
	VZ_LOCALAPI_USER_ANONYMOUS,			/*User level anonymous is not allowed.*/
	VZ_LOCALAPI_PASSWORD_LONG,			/*The password is too long*/
	VZ_LOCALAPI_PASSWORD_SHORT,			/*The password is too SHORT*/
	VZ_LOCALAPI_USERNAME_MISS, 	/*Username not recognized*/
	VZ_LOCALAPI_USERNAME_FIXED,			/*Username may not be deleted*/
	VZ_LOCALAPI_VALUE_INVALID, 		/*Argument Value Invalid*/
	VZ_LOCALAPI_NETWORK_INTERFACE_EXCEED,			/*Maximum number of network interfaces exceeded.*/
	VZ_LOCALAPI_NETWORK_INTERFACE_INVALID,			/*Invalid Network Interface*/
	VZ_LOCALAPI_IPV4_INVALID,			/*Invalid IPv4 Address*/
	VZ_LOCALAPI_ENABLE_TLS_FAIL,			/*Enabling Tls Failed */
	VZ_LOCALAPI_RULE_EXCEED,			/*TooManyRules */
	VZ_LOCALAPI_RULE_CLASH,			/*RuleAlreadyExistent */
	VZ_LOCALAPI_RULE_NONEXIST, 		/*RuleNotExistent */
	VZ_LOCALAPI_RULE_INVALID,		/*InvalidRule */
	VZ_LOCALAPI_CONFIG_TOKEN_NONEXIST,		/*ConfigurationToken not exist*/
	VZ_LOCALAPI_PROFILE_NONEXIST,		/*profile token does not exist*/
	VZ_LOCALAPI_DNS_INVALID,		/*Invalid Dns Name*/
	VZ_LOCALAPI_HOSTNAME_INVALID,		/*Invalid host name */
	VZ_LOCALAPI_RECORDINGJOB_EXCEED,			/*MaxRecordingJobs */
	VZ_LOCALAPI_RECORDING_EXCEED,			/*MaxRecording */
	VZ_LOCALAPI_SERIALPORT_INVALID,			/*InvalidSerialPort */
	VZ_LOCALAPI_PRESET_EXCEED, 		/*TooManyPresets*/
	VZ_LOCALAPI_PRESETNAME_INVALID,   /* InvalidPresetName*/
	VZ_LOCALAPI_VIDEOANALYTICS_CHANNEL_EXCEED,			/*too many channls in one video ananlytics */
	VZ_LOCALAPI_IMAGEVALUE_INVALID,
	VZ_LOCALAPI_RECEIVER_EXCEED,
	VZ_LOCALAPI_NO_PTZNODE,/*No such PTZNode on the device*/
	VZ_LOCALAPI_NO_VSOURCE,/*No such video source token on the device*/
	VZ_LOCALAPI_TRACK_EXCEED,			/*TooMany recording tracks */
	VZ_LOCALAPI_NO_TRACK,/*No such track token on the device*/
	VZ_LOCALAPI_NO_RECORDING,/*No such recording token on the device*/
	VZ_LOCALAPI_NO_RECORDINGJOB,/*No such recordingjob token on the device*/
	VZ_LOCALAPI_BAD_MODE,/* recordingjob mode error*/
	VZ_LOCALAPI_OSDS_EXCEED,/* TooMany OSD */
}LocalApi_Err_Type;

LocalApi_Err_Type gLocalErr;


/* tt:SetDateTimeType */
enum VZ__SetDateTimeType { Vz__SetDateTimeType__Manual = 0, Vz__SetDateTimeType__NTP = 1 };
/* tt:IPType */
enum VZ__IPType { Vz__IPType__IPv4 = 0, Vz__IPType__IPv6 = 1 };
/* tt:NetworkProtocolType */
enum VZ__NetworkProtocolType { Vz__NetworkProtocolType__HTTP = 0, Vz__NetworkProtocolType__HTTPS = 1, Vz__NetworkProtocolType__RTSP = 2 };
/* tt:NetworkHostType */
enum VZ__NetworkHostType { Vz__NetworkHostType__IPv4 = 0, Vz__NetworkHostType__IPv6 = 1, Vz__NetworkHostType__DNS = 2 };
/* tt:H264Profile */
enum VZ__H264Profile { VZ__H264Profile__Baseline = 0, VZ__H264Profile__Main = 1, VZ__H264Profile__Extended = 2, VZ__H264Profile__High = 3 };
/* tt:TrackType */
enum VZ__TrackType { VZ__TrackType__Video = 0, VZ__TrackType__Audio = 1, VZ__TrackType__Metadata = 2, VZ__TrackType__Extended = 3 };
/* tt:RecordingStatus */
enum VZ__RecordingStatus { VZ__RecordingStatus__Initiated = 0, VZ__RecordingStatus__Recording = 1, VZ__RecordingStatus__Stopped = 2, VZ__RecordingStatus__Removing = 3, VZ__RecordingStatus__Removed = 4, VZ__RecordingStatus__Unknown = 5 };
/* tt:OSDType */
enum VZ__OSDType { vz__OSDType__Text = 0, vz__OSDType__Image = 1, vz__OSDType__Extended = 2 };
/* tt:Mpeg4Profile */
enum VZ__Mpeg4Profile { vz__Mpeg4Profile__SP = 0, vz__Mpeg4Profile__ASP = 1 };
/* tt:VideoEncoding */
enum VZ__VideoEncoding { vz__VideoEncoding__JPEG = 0, vz__VideoEncoding__MPEG4 = 1, vz__VideoEncoding__H264 = 2 };

/* tt:VideoEncoding */
enum VZ__ConfigType { vz_Config_VideoEncoder = 0, vz_Config_VideoSource = 1, vz_Config_AudioEncoder = 2 ,vz_Config_AudioSource = 3 };
/* tt:SearchState */
enum VZ__SearchState { VZ__SearchState__Queued = 0, VZ__SearchState__Searching = 1, VZ__SearchState__Completed = 2, VZ__SearchState__Unknown = 3 };

/* tt:CapabilityCategory */
enum VZ__CapabilityCategory 
{ 
	Vz__CapabilityCategory__All = 0,
	Vz__CapabilityCategory__Analytics = 1, 
	Vz__CapabilityCategory__Device = 2, 
	Vz__CapabilityCategory__Events = 3, 
	Vz__CapabilityCategory__Imaging = 4, 
	Vz__CapabilityCategory__Media = 5, 
	Vz__CapabilityCategory__PTZ = 6, 
};

enum VZ__PTZPresetTourState { VZ__PTZPresetTourState__Idle = 0, VZ__PTZPresetTourState__Touring = 1, VZ__PTZPresetTourState__Paused = 2, VZ__PTZPresetTourState__Extended = 3 };
enum VZ__PTZPresetTourOperation { VZ__PTZPresetTourOperation__Start = 0, VZ__PTZPresetTourOperation__Stop = 1, VZ__PTZPresetTourOperation__Pause = 2, VZ__PTZPresetTourOperation__Extended = 3 };

/* tmd:SerialPortType */
enum VZ__SerialPortType { VZ__SerialPortType__RS232 = 0, VZ__SerialPortType__RS422HalfDuplex = 1, VZ__SerialPortType__RS422FullDuplex = 2, VZ__SerialPortType__RS485HalfDuplex = 3, VZ__SerialPortType__RS485FullDuplex = 4, VZ__SerialPortType__Generic = 5 };
/* tmd:ParityBit */
enum VZ__ParityBit { VZ__ParityBit__None = 0, VZ__ParityBit__Even = 1, VZ__ParityBit__Odd = 2, VZ__ParityBit__Mark = 3, VZ__ParityBit__Space = 4, VZ__ParityBit__Extended = 5 };

#ifdef NEW_ONVIF


typedef enum
{
	vz_CFGType_VideoSource = 0,
	vz_CFGType_VideoEncoder,
	vz_CFGType_AudioSource,
	vz_CFGType_AudioEncoder,
	vz_CFGType_PTZ,
	vz_CFGType_VideoAnalytics,
} LocalSDK_CFGType;

typedef struct
{ 
	LocalSDK_CFGType type;
	char 	ProfileToken[VZ_TOKEN_STR_LEN];		
	char 	ConfigurationToken[VZ_TOKEN_STR_LEN];
}LocalSDK__AddCFG;

typedef struct
{ 
	LocalSDK_CFGType type;
	char 	ProfileToken[VZ_TOKEN_STR_LEN];		
}LocalSDK__DelCFG;

typedef struct
{
	int 	Spare;									// optional, Number of spare jobs that can be created for the recording
	char 	CompatibleSources[160];					// optional, A device that supports recording of a restricted set of Media Service Profiles returns the list of profiles that can be recorded on the given Recording
} LocalSDK_JobOptions;

typedef struct
{
	int 	SpareTotal;								// optional, Total spare number of tracks that can be added to this recording
	int 	SpareVideo;								// optional, Number of spare Video tracks that can be added to this recording
	int 	SpareAudio;								// optional, Number of spare Aduio tracks that can be added to this recording
	int 	SpareMetadata;							// optional, Number of spare Metadata tracks that can be added to this recording
} LocalSDK_TrackOptions;

typedef struct
{ 
	LocalSDK_JobOptions	Job;						// required, 
	LocalSDK_TrackOptions	Track;						// required, 
	char	RecordingToken[VZ_TOKEN_STR_LEN]; 
}LocalSDK__RecordingOptions;

#endif

typedef struct {
	char	hostname[HOST_NAME_LEN];	///< 主机名
	struct in_addr	ip; ///< IP address in static IP mode
	struct in_addr	netmask; ///< netmask in static IP mode
	struct in_addr	gateway; ///< gateway in static IP mode
	struct in_addr	dns; ///< DNS IP in static IP mode
	int httpport;
	int rtspport;
	char	mac[HOST_NAME_LEN]; ///< hardware MAC address
}LocalSDK_NetBaseConfig;

typedef struct {
	char	Manufacturer[HOST_NAME_LEN];	///设备商 VZ
	//char	Version[];		//软件版本
	//char	HardwareID[];	//硬件ID
	//char	SerialNumber[];	//序列号
}LocalSDK_DeviceInfo;


typedef  struct
{
	int Year;	/* required element of type xsd:int */
	int Month;	/* required element of type xsd:int */
	int Day;	/* required element of type xsd:int */
}LocalSDK_Date;
typedef  struct
{
	int Hour;	/* required element of type xsd:int */
	int Minute;	/* required element of type xsd:int */
	int Second;	/* required element of type xsd:int */
}LocalSDK_Time;
typedef struct
{
	LocalSDK_Time Time;	/* required element of type tt:Time */
	LocalSDK_Date Date;	/* required element of type tt:Date */
}LocalSDK_DateTime;


/* tds:SetSystemDateAndTime */
typedef  struct {
	__u8 DateTimeType;	/* used by set and get*/
	__u8 DaylightSavings;	/*used by set and get*/
	char TimeZone[VZ_TZ_STR_LEN];	/* used by set and get*/
	LocalSDK_DateTime UTCDateTime;	/*used by set and get */
	LocalSDK_DateTime LocalDateTime;	/* only used by get */
}LocalSDK_SystemDateAndTime;

/* tds:SetScopes */
typedef struct
{
		int sizeScopes;	/* sequence of elements <Scopes> */
		char *Scopes[VZ_LIST_MAX_NUM];	/* required element of type xsd:string */
}LocalSDK_ScopesInfo;

/*one uer info*/
typedef struct
{
	char *Username;	/* required element of type xsd:string */
	char *Password;	/* optional element of type xsd:string */
	__u8 UserLevel;	/* required element of type tt:UserLevel */
}LocalSDK_User;

/* tds:CreateUsers */
typedef struct
{
	int sizeUser;	/* sequence of elements <User> */
	LocalSDK_User UserInfo[VZ_LIST_MAX_NUM];
}LocalSDK_UsersInfo;


/* tt:IPAddress */
typedef  struct 
{
	__u8 Type;	/* required element of type tt:IPType */
	char IPv4Address[VZ_IP_ADDR_LEN];	/* optional element of type tt:IPv4Address */
	char IPv6Address[VZ_IP_ADDR_LEN];	/* optional element of type tt:IPv6Address */
}LocalSDK_IPAddress;

typedef struct
{
	int __size;	/* sequence of elements <-any> */
	char *__any;
} LocalSDK__ComExtension;

/* tt:DNSInformation */
typedef struct
{
	__u8 FromDHCP;	/* required element of type xsd:boolean */
	int sizeSearchDomain;	/* sequence of elements <SearchDomain> */
	char **SearchDomain;	/* optional element of type xsd:token */
	int sizeDNSFromDHCP;	/* sequence of elements <DNSFromDHCP> */
	LocalSDK_IPAddress DNSFromDHCP[VZ_DNS_MAX_NUM];	/* optional element of type tt:IPAddress */
	int sizeDNSManual;	/* sequence of elements <DNSManual> */
	LocalSDK_IPAddress DNSManual[VZ_DNS_MAX_NUM];	/* optional element of type tt:IPAddress */
	LocalSDK__ComExtension Extension;
	char *__anyAttribute;	/* optional attribute of type xsd:anyType */
}LocalSDK_DNSInfo;



/* tt:PrefixedIPv4Address */
typedef struct
{
	char Address[VZ_IP_ADDR_LEN];	/* required element of type tt:IPv4Address */
	int PrefixLength;	/* required element of type xsd:int */
} LocalSDK__PrefixIPv4Addr;
typedef enum 
{
	//PREFIX_IPV4_ADDR_MANUAL,
	PREFIX_IPV4_ADDR_LINKLOCAL,
	PREFIX_IPV4_ADDR_FROMDHCP,
	PREFIX_IPV4_ADDR_MAX,
}IPV4_ADDR_TYPE;

/* tt:IPv4Configuration */
typedef struct
{
	int sizeManual;	/* sequence of elements <Manual> */
	LocalSDK__PrefixIPv4Addr PreIPv4Add[PREFIX_IPV4_ADDR_MAX];	/* optional element of type tt:PrefixedIPv4Address */
	LocalSDK__PrefixIPv4Addr Manual[VZ_MANUAL_IP_MAX_NUM];	/* optional element of type tt:PrefixedIPv4Address */
	__u8 DHCP;	/* required element of type xsd:boolean */
	__u8 Enabled;/*only used by set */
	int size;	/* sequence of elements <-any> */
	char *any;
	char *anyAttribute;	/* optional attribute of type xsd:anyType */
}LocalSDK__IPv4Config;

/* tt:IPv4NetworkInterface */
typedef struct
{
	__u8 Enabled;	/* required element of type xsd:boolean */
	LocalSDK__IPv4Config Config;	/* required element of type tt:IPv4Configuration */
} LocalSDK__IPv4NetworkInterface;

/* tt:NetworkInterface */
typedef  struct
{
	char *token;	
	__u8 Enabled;	
	/*info*/
	char Name[VZ_MAC_STR_LEN];	
	char HwAddress[VZ_MAC_STR_LEN];	
	int MTU;	
	/*IPv4*/
	LocalSDK__IPv4NetworkInterface IPv4;	
	char *anyAttribute;	/* optional attribute of type xsd:anyType */
	/*IPv6*/
	//struct tt__IPv6NetworkInterface *IPv6;	
	/*link*/
	//struct tt__NetworkInterfaceLink *Link;
	/*Extension*/
	//struct tt__NetworkInterfaceExtension *Extension;	
} LocalSDK__NetworkInterface;
/* tds:GetNetworkInterfacesResponse */
typedef struct
{
	int sizeNetworkInterfaces;	/*Interfaces number*/	/* sequence of elements <NetworkInterfaces> */
	LocalSDK__NetworkInterface NetworkInterfaces[VZ_NETWORK_INTERFACE_MAX_NUM];	/* required element of type tt:NetworkInterface */
}LocalSDK__NetworkInterfaceList;

typedef enum 
{ 
	NETWORK_PROTOCOL_TYPE_HTTP = 0,
	NETWORK_PROTOCOL_TYPE_HTTPS, 
	NETWORK_PROTOCOL_TYPE_RTSP, 
	NETWORK_PROTOCOL_TYPE_MAX, 
}LocalSDK__NetworkProtocolType ;

/* tt:NetworkProtocol */
typedef struct 
{
	LocalSDK__NetworkProtocolType Name;	/* required element of type tt:NetworkProtocolType */
	__u8 Enabled;	/* required element of type xsd:boolean */
	int sizePort;	/* sequence of elements <Port> */
	int Port;	/* required element of type xsd:int */
	LocalSDK__ComExtension Extension;	/* optional element of type tt:NetworkProtocolExtension */
	char *anyAttribute;	/* optional attribute of type xsd:anyType */
}LocalSDK__NetworkProtocol;

typedef struct
{
	int sizeNetworkProtocols;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* sequence of elements <NetworkProtocols> */
	LocalSDK__NetworkProtocol NetworkProtocols[NETWORK_PROTOCOL_TYPE_MAX];	/* optional element of type tt:NetworkProtocol */
} LocalSDK__NetworkProtocolList;

/* tt:NetworkGateway */
typedef struct 
{
	int sizeIPv4Address;	/* sequence of elements <IPv4Address> */
	char IPv4Address[VZ_LIST_MAX_NUM][VZ_IP_ADDR_LEN];	/* optional element of type tt:IPv4Address */
	int sizeIPv6Address;	/* sequence of elements <IPv6Address> */
	char IPv6Address[VZ_LIST_MAX_NUM][VZ_IP_ADDR_LEN];	/* optional element of type tt:IPv6Address */
}LocalSDK__NetworkGateway;


typedef struct 
{
	char Name[HOST_NAME_LEN];	/* required attribute of type xsd:string */
	char Value[HOST_NAME_LEN];	/* required attribute of type xsd:anySimpleType */
}LocalSDK_RuleSimpleItem;
typedef struct 
{
	char Name[HOST_NAME_LEN];	/* required attribute of type xsd:string */
	char Value[VZ_EITEM_ANY_MAX_SIZE];	/* required attribute of type xsd:anySimpleType */
}LocalSDK_RuleElementItem;

typedef struct
{
	int __sizeSimpleItem;	/* sequence of elements <SimpleItem> */
	LocalSDK_RuleSimpleItem SimpleItem[VZ_LIST_MAX_NUM];	/* optional element of type tt:ItemList-SimpleItem */
	int __sizeElementItem;	/* sequence of elements <ElementItem> */
	LocalSDK_RuleElementItem ElementItem[VZ_LIST_MAX_NUM];	/* optional element of type tt:ItemList-ElementItem */
} LocalSDK__RuleItemList;

typedef struct 
{
	LocalSDK__RuleItemList *Source;	/* optional element of type tt:ItemListDescription */
	LocalSDK__RuleItemList *Key;	/* optional element of type tt:ItemListDescription */
	LocalSDK__RuleItemList *Data;	/* optional element of type tt:ItemListDescription */
	__u8 IsProperty;	/* optional attribute of type xsd:boolean */
	char *__anyAttribute;	/* optional attribute of type xsd:anyType */
	char *ParentTopic;	/* required element of type xsd:string */
}LocalSDK__RuleMsg;
/* tt:ConfigDescription */
typedef struct 
{
	LocalSDK__RuleItemList Parameters;	/* required element of type tt:ItemListDescription */
	int __sizeMessages;	/* sequence of elements <Messages> */
	LocalSDK__RuleMsg *Messages;	/* optional element of type tt:ConfigDescription-Messages */
	char Name[HOST_NAME_LEN];	/* required attribute of type xsd:QName */
}LocalSDK__ConfigDescription;

typedef struct
{
	LocalSDK__RuleItemList Parameters;	/* required element of type tt:ItemList */
	char Type[HOST_NAME_LEN];	/* required attribute of type xsd:QName */
	char Name[HOST_NAME_LEN];	/* required attribute of type xsd:QName */
}LocalSDK__RuleConfig;
typedef LocalSDK__RuleConfig LocalSDK__AnalyticsConfig; 
typedef struct 
{
	char *token;	
	int sizeRule;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* sequence of elements <Rule> */
	LocalSDK__RuleConfig Rule[VZ_LIST_MAX_NUM];	/* optional element of type tt:Config */
}LocalSDK__OneSetOfRule;

/* tt:SupportedRules */
typedef struct 
{
	int sizeSchemaLocation;	/* sequence of elements <RuleContentSchemaLocation> */
	char SchemaLocation[VZ_LIST_MAX_NUM][VZ_STR_SHORT_LEN];	/* optional element of type xsd:string */
	int sizeRuleDescription;	/* sequence of elements <RuleDescription> */
	LocalSDK__ConfigDescription RuleDescription[VZ_LIST_MAX_NUM];	/* optional element of type tt:ConfigDescription */
}LocalSDK__SupportedRules;

typedef struct
{
	char profiletoken[VZ_TOKEN_STR_LEN];
	char Uri[VZ_STR_SHORT_LEN];	/* required element of type xsd:string */
	int channel;
	__u8 PortMapFlag;	/* required element of type xsd:boolean */
	__u8 InvalidAfterConnect;	/* required element of type xsd:boolean */
	__u8 InvalidAfterReboot;	/* required element of type xsd:boolean */
} LocalSDK__MediaUri;

 typedef struct 
{
	int Filtersize;	/* sequence of elements Filterany */
	char Filterany[VZ_LIST_MAX_NUM][VZ_STR_SHORT_LEN];
	int SubscriptionPolicysize;	/* sequence of elements SubscriptionPolicyany */
	char SubscriptionPolicyany[VZ_LIST_MAX_NUM][VZ_STR_SHORT_LEN];

}LocalSDK__EventSubscription;
typedef  struct
{
	LocalSDK_IPAddress Address;	/* required element of type tt:IPAddress */
	int Port;	/* required element of type xsd:int */
	int TTL;	/* required element of type xsd:int */
	__u8 AutoStart;	/* required element of type xsd:boolean */
}LocalSDK__MulticastConfig;
typedef  struct
{
	char Name[VZ_TOKEN_STR_LEN];	/* required element of type tt:Name */
	int UseCount;	/* required element of type xsd:int */
	char token[VZ_TOKEN_STR_LEN];	/* required attribute of type tt:ReferenceToken */
	LocalSDK__MulticastConfig Multicast;	/* required element of type tt:MulticastConfiguration */
	LocalSDK__EventSubscription Event;
	char SessionTimeout[VZ_IP_ADDR_LEN];	/* required element of type xsd:string */
} LocalSDK__MetadataConfig;

/* tt:NetworkHost */
typedef  struct 
{
	__u8 Type;	/* required element of type tt:IPType */
	char IPv4Address[VZ_IP_ADDR_LEN];	/* optional element of type tt:IPv4Address */
	char IPv6Address[VZ_IP_ADDR_LEN];	/* optional element of type tt:IPv6Address */
	char DNSname[VZ_IP_ADDR_LEN];	/* optional element of type tt:DNSName */
}LocalSDK_NetworkHost;
/* tt:NTPInformation */
typedef struct
{
	__u8 FromDHCP;	/* required element of type xsd:boolean */
	int sizeNTPFromDHCP;	/* sequence of elements <DNSFromDHCP> */
	LocalSDK_NetworkHost NTPFromDHCP[VZ_LIST_MAX_NUM];	/* optional element of type tt:IPAddress */
	int sizeNTPManual;	/* sequence of elements <DNSManual> */
	LocalSDK_NetworkHost NTPManual[VZ_LIST_MAX_NUM];	/* optional element of type tt:IPAddress */
}LocalSDK_NTPInfo;

/* tds:SetNTP */
typedef  struct 
{
	__u8 FromDHCP;	/* required element of type xsd:boolean */
	int sizeNTPManual;	/* sequence of elements <NTPManual> */
	LocalSDK_NetworkHost NTPManual[VZ_LIST_MAX_NUM];	/* optional element of type tt:NetworkHost */
}LocalSDK_SetNTP;
/* tt:DeviceCapabilities */
typedef  struct
{
	char *XAddr;	/* required element of type xsd:string */
	void *Network;	/* optional element of type tt:NetworkCapabilities */
	void *System;	/* optional element of type tt:SystemCapabilities */
	void *IO;	/* optional element of type tt:IOCapabilities */
	void *Security;	/* optional element of type tt:SecurityCapabilities */
} LocalSDK__DeviceCapabilities;

typedef  struct
{
	int sizeCategory;	/* sequence of elements <Category> */
	__u8 *Category;	/* optional element of type tt:CapabilityCategory */
	__u8 DeviceFlag;	
	LocalSDK__DeviceCapabilities Device;	/* optional element of type tt:DeviceCapabilities */
} LocalSDK__Capabilities;
typedef struct 
{
	char *token;	
	//int sizeAnalyticsModule;	/* SOAP 1.2 RPC return element (when namespace qualified) */	/* sequence of elements <Rule> */
	LocalSDK__AnalyticsConfig Analytics;	/* optional element of type tt:Config */
}LocalSDK__Analytics;

typedef struct 
{
	char *DeviceServerCap;	
	char *MediaCap;
	char *EventsCap;	
	char *PTZCap;
	char *ImagingCap;	
	char *AnalyticsCap;
	char *DeviceIOCap;	
	char *RecordingCap;	
	char *ReplayCap;	
	char *ReceiverCap;	
	char *SearchCap;	
}LocalSDK__Service_Capabilities;


typedef struct
{ 
	char *SourceId;	/* required element of type xsd:string */
	char *Name;	/* required element of type tt:Name */
	char *Location;	/* required element of type tt:Description */
	char *Description;	/* required element of type tt:Description */
	char *Address;	/* required element of type xsd:string */
}LocalSDK__RecordingSource;
typedef struct 
{
	LocalSDK__RecordingSource Source; /* required element of type tt:RecordingSourceInformation */
	char * Content;	/* required element of type tt:Description */
	char *MaximumRetentionTime; /* required element of type xsd:string */
}LocalSDK__RecordingConfig;

typedef struct 
{
	char token[VZ_TOKEN_STR_LEN];	
	LocalSDK__RecordingConfig Config;	/* optional element of type tt:Config */
}LocalSDK__Recording;

typedef struct
{
	char SourceToken[VZ_STR_SHORT_LEN];	/* required element of type tt:RecordingReference */
	char SourceType[VZ_STR_SHORT_LEN];	/* required element of type tt:RecordingReference */
	//int JobSourceState;
	__u8 AutoCreateReceiver;	/* required element of type tt:RecordingJobMode */
}LocalSDK__JobSource;

typedef struct
{
	char RecordingToken[VZ_TOKEN_STR_LEN];;	/* required element of type tt:RecordingReference */
	char Mode[HOST_NAME_LEN];	/* required element of type tt:RecordingJobMode */
	int Priority;	/* required element of type xsd:int */
	int SavePlace;
	int Source_num;	/* required element of type xsd:int */
	LocalSDK__JobSource Source[VZ_LIST_MAX_NUM];	/* optional element of type tt:RecordingJobSource */
}LocalSDK__JobConfiguration;
typedef struct
{
	char JobToken[VZ_TOKEN_STR_LEN];
	char Mode[HOST_NAME_LEN];
	int Place;
}LocalSDK__SetRecordingJob;

typedef struct
{
	char JobToken[VZ_TOKEN_STR_LEN];
	LocalSDK__JobConfiguration JobConfig;	/* required element of type xsd:int */
}LocalSDK__RecordingJob;


typedef struct
{
	char PortToken[VZ_TOKEN_STR_LEN];
	char *PortData;	
}LocalSDK__SerialPort;
typedef struct
{
	int SerialPort_num;
	LocalSDK__SerialPort SerialPort[VZ_LIST_MAX_NUM];	
}LocalSDK__SerialPorts;

typedef struct 
{
	char profiletoken[VZ_TOKEN_STR_LEN];
	char Name[VZ_TOKEN_STR_LEN];	/* optional element of type tt:Name */
	//tt__PTZVector *PTZPosition;	/* optional element of type tt:PTZVector */
	char token[VZ_TOKEN_STR_LEN];	/* optional attribute */
	int channel;
	char enable;
}LocalSDK__PTZPreset;

/*
typedef struct 
{
	char token[VZ_TOKEN_STR_LEN];
	//char *Preset;
	int channel;
	int Preset_num;	
	LocalSDK__PTZPreset Preset[VZ_PTZ_PRESET_NUM];	
}LocalSDK__PTZPresets;
*/
typedef struct 
{
	//tt__PTZSpeed *Speed;	/* optional element of type tt:PTZSpeed */
	__u8 enable;
	__u8 cruise_method;				//0 循环，此时schedule有效，1 定时，此时起始时间有效
	char StayTime[32*16];	/* optional element of type xsd:string */
}LocalSDK__PTZPresetTourSpot;


typedef struct 
{
	int RecurringTime;	/* optional element of type xsd:int */
	char RecurringDuration[32*8];	/* optional element of type xsd:string */
}LocalSDK__PTZPresetTourStartingCondition;
typedef struct 
{
	enum VZ__PTZPresetTourState State;	/* required element of type tt:PTZPresetTourState */
	LocalSDK__PTZPresetTourSpot CurrentTourSpot;	/* optional element of type tt:PTZPresetTourSpot */
}LocalSDK__PTZPresetTourStatus;
typedef struct 
{
	char token[VZ_TOKEN_STR_LEN];
	enum VZ__PTZPresetTourOperation Operation;
	LocalSDK__PTZPresetTourStatus Status;	
	__u8 AutoStart;
	LocalSDK__PTZPresetTourStartingCondition StartingCondition;	
}LocalSDK__PresetTour;
typedef struct 
{
	char ProfileToken[VZ_TOKEN_STR_LEN];
	int channel;
	int PresetTour_num;
	LocalSDK__PresetTour PresetTour[VZ_PTZ_CRUISE_NUM]; /* required element of type tt:PresetTour */
}LocalSDK__PresetTours;
typedef struct 
{
	__u8 Onoff; 
	float BLCLevel;	
}LocalSDK__ImagingBLCSetting;
typedef struct 
{
	char VideoSourceToken[VZ_TOKEN_STR_LEN];
	int ForcePersistence; 
	int Brightness;	
	int ColorSaturation;	
	int Contrast;	
	int hue;	
	int channel;
	//float Sharpness;
	//LocalSDK__ImagingBLCSetting BLC;	/* optional element of type tt:BacklightCompensation20 */
}LocalSDK__ImagingSetting;

typedef struct 
{
	time_t CurrentTime;	/* required element of type xsd:dateTime */
	time_t TerminationTime;	/* required element of type xsd:dateTime */
}LocalSDK__EventTime;

typedef struct 
{
	char Timeout[VZ_PTZ_PRESET_NAMELEN];
	LocalSDK__EventTime Time;	/* required element of type xsd:dateTime */
	char Address[VZ_STR_SHORT_LEN];
	char Filter[VZ_STR_SHORT_LEN*2];
}LocalSDK__PullPointSubscription;

typedef struct 
{
	char Topic[VZ_STR_SHORT_LEN];
	char * Message;	
}LocalSDK__PullMessageInfo;
typedef struct 
{
	//int Timeout;	
	int MessageLimit;	
	int MessageInfo_num;
	char Timeout[VZ_PTZ_PRESET_NAMELEN];
	char SubscripToken[VZ_STR_SHORT_LEN];
	LocalSDK__EventTime Time;
	LocalSDK__PullMessageInfo MessageInfo[VZ_LIST_MAX_NUM];
}LocalSDK__EventPullMessages;
typedef struct 
{
	int Profile_num;	
	int channel;
	vz_onvif_Profile Profile[VZ_LIST_MAX_NUM];
}LocalSDK__Profiles;

typedef struct 
{
	int VideoEncoder_num;	
	vz_onvif_VideoEncoderConfig VideoEncoderConfig[VZ_LIST_MAX_NUM];
}LocalSDK__VideoEncoderConfigs;

typedef struct 
{
	int VideoSource_num;	
	vz_onvif_VideoSourceConfig VideoSourceConfig[VZ_LIST_MAX_NUM];
}LocalSDK__VideoSourceConfigs;



typedef struct 
{
	int RelayFlag;	
	int channel;
	int H264flag;
	vz_onvif_VideoEncoderConfig BaseConfig;
}LocalSDK__OneVideoEncoderConfig;

typedef struct 
{
	int Framerate;	
	char VideoSourceToken[VZ_TOKEN_STR_LEN];
	char RelayVideoSourceToken[VZ_TOKEN_STR_LEN];
	vz_onvif_VideoResolution Resolution;	
	LocalSDK__ImagingSetting Imaging;
}LocalSDK__VideoSource;

typedef struct 
{
	int VideoSource_num;
	LocalSDK__VideoSource VideoSource[VZ_LIST_MAX_NUM];
}LocalSDK__VideoSources;
typedef struct 
{
	int Min;	
	int Max;	
}LocalSDK__IntRange;
typedef struct 
{
	int Resolution_num;
	vz_onvif_VideoResolution ResolutionsAvailable[VZ_RESOLUTION_MAX_NUM];	
	LocalSDK__IntRange GovLengthRange;	
	LocalSDK__IntRange FrameRateRange;	
	LocalSDK__IntRange EncodingIntervalRange;
	LocalSDK__IntRange BitRateRange;
	enum VZ__H264Profile Profile;
}LocalSDK__H264Options;

typedef struct 
{
	int Resolution_num;
	vz_onvif_VideoResolution ResolutionsAvailable[VZ_RESOLUTION_MAX_NUM];	
	LocalSDK__IntRange GovLengthRange;	/* required element of type tt:IntRange */
	LocalSDK__IntRange FrameRateRange;	/* required element of type tt:IntRange */
	LocalSDK__IntRange EncodingIntervalRange;	/* required element of type tt:IntRange */
	enum VZ__Mpeg4Profile Profile;	/* required element of type tt:Mpeg4Profile */
}LocalSDK__Mpeg4Options;

typedef struct 
{
	int Resolution_num;
	vz_onvif_VideoResolution ResolutionsAvailable[VZ_RESOLUTION_MAX_NUM];	
	LocalSDK__IntRange FrameRateRange;	
	LocalSDK__IntRange EncodingIntervalRange;
}LocalSDK__JpegOptions;

typedef struct 
{
	int RelayFlag;
	int channel;
	char ConfigurationToken[VZ_TOKEN_STR_LEN];	/* optional element of type tt:ReferenceToken */
	char ProfileToken[VZ_TOKEN_STR_LEN];	/* optional element of type tt:ReferenceToken */
	LocalSDK__IntRange QualityRange;	/* required element of type tt:IntRange */
	LocalSDK__JpegOptions JPEG;	/* optional element of type tt:JpegOptions */
	LocalSDK__Mpeg4Options MPEG4;	/* optional element of type tt:Mpeg4Options */
	LocalSDK__H264Options H264;	
}LocalSDK__VideoEncoderConfigurationOption;

typedef struct 
{
	int RelayFlag;
	int channel;
	char ConfigurationToken[VZ_TOKEN_STR_LEN];	/* optional element of type tt:ReferenceToken */
	char ProfileToken[VZ_TOKEN_STR_LEN];	/* optional element of type tt:ReferenceToken */
	LocalSDK__IntRange XRange;	
	LocalSDK__IntRange YRange;	
	LocalSDK__IntRange WidthRange;
	LocalSDK__IntRange HeightRange;
	char 	AvailableSourceToken[VZ_LIST_MAX_NUM][VZ_TOKEN_STR_LEN];
}LocalSDK__VideoSourceConfigurationOption;

typedef struct 
{
	float x;	
}LocalSDK__ptz__Zoom;
typedef struct 
{
	float x;
	float y;
}LocalSDK__ptz__PanTilt;
typedef struct 
{
	int RelayFlag;
	int channel;
	char ProfileToken[VZ_TOKEN_STR_LEN];	
	LocalSDK__ptz__PanTilt PanTilt;	
	LocalSDK__ptz__Zoom Zoom;
}LocalSDK__ptz__ContinuousMove;
typedef struct 
{
	int RelayFlag;
	int channel;
	char ProfileToken[VZ_TOKEN_STR_LEN];	
	__u8 PanTilt;	
	__u8 Zoom;
}LocalSDK__ptz__StopMove;

typedef struct
{
	char TrackToken[VZ_TOKEN_STR_LEN];	/* required element of type tt:TrackReference */
	enum VZ__TrackType TrackType;	/* required element of type tt:TrackType */
	char List[STR_SHORT_LEN];
	time_t DataFrom;	/* required element of type xsd:dateTime */
	time_t DataTo;	/* required element of type xsd:dateTime */
}LocalSDK__RecordingTrackInfo;
typedef struct
{
	char RecordingToken[VZ_TOKEN_STR_LEN];	/* required element of type tt:RecordingReference */
	time_t EarliestRecording;	/* optional element of type xsd:dateTime */
	time_t LatestRecording;	/* optional element of type xsd:dateTime */
	enum VZ__RecordingStatus RecordingStatus;	/* required element of type tt:RecordingStatus */
	vz_onvif_RecordingSourceInfo Source;	/* required element of type tt:RecordingSourceInformation */
	char Content[STR_SHORT_LEN];	/* required element of type tt:Description */
	int Track_num;
	LocalSDK__RecordingTrackInfo Track[VZ_LIST_MAX_NUM];	/* optional element of type tt:TrackInformation */
}LocalSDK__RecordingInfo;
typedef struct
{
	float x;	
	float y;	
}LocalSDK__OSDPosVector;

typedef struct
{
#ifdef NEW_ONVIF
	vz_onvif_OSDPosType Type;
#else
	char Type[VZ_TOKEN_STR_LEN];	/* UpperLeft or UpperRight or LowerLeft or LowerRight or Custom*/
#endif
	LocalSDK__OSDPosVector Pos;	/* optional element of type tt:Vector */
}LocalSDK__OSDPosConfig;
typedef struct
{
	float X;	/* required attribute */
	float Y;	/* required attribute */
	float Z;	/* required attribute */
	char Colorspace[VZ_TOKEN_STR_LEN];	/* optional attribute */
	int Transparent;	/* optional attribute */
}LocalSDK__OSDColor;

typedef struct
{
#ifdef NEW_ONVIF
	vz_onvif_OSDPosType Type;
#else
	char Type[VZ_TOKEN_STR_LEN];	/* Plain or Date or Time or DateAndTime  */
#endif
	char DateFormat[VZ_TOKEN_STR_LEN];	/* optional element of type xsd:string */
	char TimeFormat[VZ_TOKEN_STR_LEN];	/* optional element of type xsd:string */
	int FontSize;	/* optional element of type xsd:int */
	LocalSDK__OSDColor FontColor;	/* optional element of type tt:OSDColor */
	LocalSDK__OSDColor BackgroundColor;	/* optional element of type tt:OSDColor */
	char PlainText[VZ_OSD_TEXT_LEN];	/* optional element of type xsd:string */
}LocalSDK__OSDTextConfig;
typedef struct
{
	char ImgPath[VZ_STR_SHORT_LEN];	/* required element of type xsd:string */
}LocalSDK__OSDImgConfig;

typedef struct
{
	char VideoSourceConfigToken[VZ_TOKEN_STR_LEN];	/* required element of type tt:OSDReference */
	char OSDToken[VZ_TOKEN_STR_LEN];	/* required element of type tt:OSDReference */
	int channel;
	int enable;
	//int vzLogoId;
	enum VZ__OSDType Type;	/* required element of type tt:OSDType */
	LocalSDK__OSDPosConfig Position;	/* required element of type tt:OSDPosConfiguration */
	LocalSDK__OSDTextConfig TextString;	/* optional element of type tt:OSDTextConfiguration */
	LocalSDK__OSDImgConfig Image;	/* optional element of type tt:OSDImgConfiguration */
}LocalSDK__OSDConfig;

typedef struct
{
	int OSDNum;
	LocalSDK__OSDConfig OSDS[VZ_LIST_MAX_NUM];	/* optional element of type tt:OSDImgConfiguration */
}LocalSDK__OSDList;

typedef struct 
{
	char ProfileToken[VZ_TOKEN_STR_LEN];	
	char ConfigurationToken[VZ_TOKEN_STR_LEN];	
}LocalSDK__AddVideoAnalyticsConfig;
typedef struct 
{
	int enable;
	//char ConfigurationToken[VZ_TOKEN_STR_LEN];	
	//char ProfileToken[VZ_TOKEN_STR_LEN];	
	//LocalSDK__RuleConfig RuleConfig;
	//LocalSDK__AnalyticsConfig AnalyticsEngineConfig;
	unsigned int ForcePersistence;	/* required element of type xsd:boolean */
	vz_onvif_VideoAnalytics cfg;
}LocalSDK__SetVideoAnalyticsConfig;

typedef struct
{
	int TotalMaxMatches;	/* optional element of type xsd:int */
	int RecordingsNum;
	char Recordings[VZ_LIST_MAX_NUM][VZ_TOKEN_STR_LEN];	/* optional element of type tt:RecordingReference */
	char StartTime[VZ_TZ_STR_LEN];
	char EndTime[VZ_TZ_STR_LEN];
	char DurationTime[VZ_TZ_STR_LEN];
	char TimeSearchToken[VZ_TZ_STR_LEN];	
}LocalSDK__RecordingTimeSearchFilter;
LocalSDK__RecordingTimeSearchFilter gTimeSearchFilter;
typedef struct
{
	char Token[VZ_TOKEN_STR_LEN];	/* optional element of type tt:RecordingReference */
	char Name[VZ_STR_SHORT_LEN];	/* optional element of type tt:RecordingReference */
}LocalSDK__RecordingSourceInfo;

typedef struct
{
	char RecordingToken[VZ_TOKEN_STR_LEN];	/* optional element of type tt:RecordingReference */
	char url[VZ_STR_SHORT_LEN];	/* optional element of type tt:RecordingReference */
	int Track_num;
	LocalSDK__RecordingSourceInfo Source;
	LocalSDK__RecordingTrackInfo Track[VZ_LIST_MAX_NUM];	/* optional element of type tt:TrackInformation */
}LocalSDK__RecordingTimeSearchResult;

typedef struct
{
	int ResultNum;
	int MinResults;	
	int MaxResults;	
	enum VZ__SearchState state;
	char WaitTime[VZ_TZ_STR_LEN];	
	char TimeSearchToken[VZ_TOKEN_STR_LEN];	
	LocalSDK__RecordingTimeSearchResult TimeSearchResult[VZ_LIST_MAX_NUM];	
}LocalSDK__RecordingTimeSearchResults;

typedef struct
{
	char ProfileToken[VZ_TOKEN_STR_LEN];	
	char ConfigurationToken[VZ_TOKEN_STR_LEN];	
}LocalSDK__AddVideoSourceConfig;

typedef struct
{
	char ProfileToken[VZ_TOKEN_STR_LEN];	
	char Uri[VZ_STR_SHORT_LEN];	/* required element of type xsd:string */
	int channel;
}LocalSDK__SnapshotUri;

typedef struct
{
	char Token[VZ_TOKEN_STR_LEN];
	int channel;
	vz_onvif_VideoSourceConfig Config;
}LocalSDK__GetVideoSourceConfig;

typedef struct
{
	char VideoSourceToken[VZ_TOKEN_STR_LEN];
	//class tt__BacklightCompensationOptions20 *BacklightCompensation;	/* optional element of type tt:BacklightCompensationOptions20 */
	LocalSDK__IntRange Brightness; /* optional element of type tt:FloatRange */
	LocalSDK__IntRange ColorSaturation;	/* optional element of type tt:FloatRange */
	LocalSDK__IntRange Contrast;	/* optional element of type tt:FloatRange */
	int channel;
	//class tt__ExposureOptions20 *Exposure;	/* optional element of type tt:ExposureOptions20 */
	//class tt__FocusOptions20 *Focus;	/* optional element of type tt:FocusOptions20 */
	//LocalSDK__IntRange Sharpness;	/* optional element of type tt:FloatRange */
	//class tt__WideDynamicRangeOptions20 *WideDynamicRange;	/* optional element of type tt:WideDynamicRangeOptions20 */
	//class tt__WhiteBalanceOptions20 *WhiteBalance;	/* optional element of type tt:WhiteBalanceOptions20 */
}LocalSDK__ImagingOption;

typedef struct
{
	char Token[VZ_TOKEN_STR_LEN];
	//int remote_channel;
	int channel;
}LocalSDK__RemoteChannel_Info;

typedef struct 
{
	char ConfigurationToken[VZ_TOKEN_STR_LEN];	
	char ProfileToken[VZ_TOKEN_STR_LEN];	
	unsigned char Encoding;	
	vz_onvif_VideoResolution Resolution;	
}LocalSDK__SUbIPCVideoEncoderConfigOption;

typedef struct 
{
	int AudioSource_num;	
	vz_onvif_AudioSourceConfig AudioSourceConfig[VZ_LIST_MAX_NUM];
}LocalSDK__AudioSourceConfigs;

typedef struct 
{
	int AudioEncoder_num;	
	vz_onvif_AudioEncoderConfig AudioEncoderConfig[VZ_LIST_MAX_NUM];
}LocalSDK__AudioEncoderConfigs;

typedef struct
{
	char ProfileToken[VZ_TOKEN_STR_LEN];
	char ConfigToken[VZ_TOKEN_STR_LEN];
	enum VZ__ConfigType ConfigType;
	int channel;
}LocalSDK__RemoteChannel_ProfileConfig_Info;

typedef struct
{
	char RecordingToken[VZ_TOKEN_STR_LEN];
	char TrackToken[VZ_TOKEN_STR_LEN];
	char Description[VZ_STR_SHORT_LEN];
	enum VZ__TrackType TrackType;
}LocalSDK__TrackConfig_Info;

typedef struct
{
	char SearchToken[VZ_TOKEN_STR_LEN];
	time_t Endpoint;	
}LocalSDK__EndSearch_Info;


typedef struct
{
	time_t DataFrom;	/* required element of type xsd:dateTime */
	time_t DataUntil;	/* required element of type xsd:dateTime */
	int NumberRecordings;	/* required element of type xsd:int */
}LocalSDK__RecordingSummary_Info;


typedef struct
{
	char *URI;
	LocalSDK__IntRange XRange;
	LocalSDK__IntRange YRange;
}LocalSDK__PTZSpace2DDescp;

typedef struct
{
	char *URI;
	LocalSDK__IntRange XRange;
}LocalSDK__PTZSpace1DDescp;

typedef struct
{
	LocalSDK__PTZSpace2DDescp ContinuousPanTiltVelocitySpace;	
	LocalSDK__PTZSpace1DDescp ContinuousZoomVelocitySpace;	
	//LocalSDK__PTZSpace1DDescp PanTiltSpeedSpace;
	//LocalSDK__PTZSpace1DDescp ZoomSpeedSpace;
}LocalSDK__PTZSpaces;

typedef struct
{
	char Name[VZ_TOKEN_STR_LEN];	
	LocalSDK__PTZSpaces SupportedPTZSpaces;	
	//int MaximumNumberOfPresets;
	//enum xsd__boolean_ HomeSupported;	
}LocalSDK__PTZNode;

typedef struct
{
	int Node_num;
	LocalSDK__PTZNode Node[VZ_LIST_MAX_NUM];	
}LocalSDK__PTZNodes;

typedef struct
{
	char PortToken[VZ_TOKEN_STR_LEN];
	int BaudRate;	
	enum VZ__ParityBit ParityBit;	/* required element of type tmd:ParityBit */
	int CharacterLength;	
	float StopBit;	
	enum VZ__SerialPortType type;	/* required attribute */
	
}LocalSDK__SerialPortConfig;

typedef struct
{
	int BaudRate_Num;
	int ParityBit_Num;
	int StopBit_Num;
	int DataBits_Num;
	int BaudRate[VZ_LIST_MAX_NUM*2];
	int DataBits[VZ_LIST_MAX_NUM];
	enum VZ__ParityBit ParityBit[VZ_LIST_MAX_NUM];	/* required element of type tmd:ParityBit */
	float StopBit[VZ_LIST_MAX_NUM];	
}LocalSDK__SerialPortConfigOptions;

typedef struct
{
	char PortToken[VZ_TOKEN_STR_LEN];
	LocalSDK__SerialPortConfigOptions Options;
}LocalSDK__GetSerialPortConfigOptions;

typedef struct
{
	char RelayToken[MAX_OSD_NUM_ONE_CHANNEL][VZ_TOKEN_STR_LEN];	/* required element of type tt:OSDReference */
}LocalSDK__OSDRelayTokens;

#endif

