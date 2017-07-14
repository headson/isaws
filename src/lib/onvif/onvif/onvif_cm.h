
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

#ifndef ONVIF_COMM_H
#define ONVIF_COMM_H

/***************************************************************************************/
#define ONVIF_TOKEN_LEN   	100
#define ONVIF_NAME_LEN    	100
#define ONVIF_URI_LEN     	300
#define ONVIF_SCOPE_LEN		128

#define MAX_PTZ_PRESETS     100 
#define MAX_DNS_SERVER		2
#define MAX_SEARCHDOMAIN	4
#define MAX_NTP_SERVER		2
#define MAX_SERVER_PORT		4
#define MAX_GATEWAY			2
#define MAX_RES_NUMS		8
#define MAX_SCOPE_NUMS		100
#define MAX_USERS			10
#define MAX_IP_ADDRS		4

/* Floating point precision */
#define FPP					0.01


/***************************************************************************************/
typedef enum 
{
	ONVIF_OK = 0,
	ONVIF_ERR_INVALID_IPV4_ADDR = -1,
	ONVIF_ERR_INVALID_IPV6_ADDR = -2,
	ONVIF_ERR_INVALID_DNS_NAME = -3,
	ONVIF_ERR_SERVICE_NOT_SUPPORT = -4,
	ONVIF_ERR_PORT_ALREADY_INUSE = -5,
	ONVIF_ERR_INVALID_GATEWAY_ADDR = -6,
	ONVIF_ERR_INVALID_HOSTNAME = -7,
	ONVIF_ERR_MISSINGATTR = -8,
	ONVIF_ERR_INVALID_DATETIME = -9,
	ONVIF_ERR_INVALID_TIMEZONE = -10,
	ONVIF_ERR_PROFILE_EXISTS = -11,
	ONVIF_ERR_MAX_NVT_PROFILES = -12,
	ONVIF_ERR_NO_PROFILE = -13,
	ONVIF_ERR_DEL_FIX_PROFILE = -14,
	ONVIF_ERR_NO_CONFIG = -15,
	ONVIF_ERR_NO_PTZ_PROFILE = -16,
	ONVIF_ERR_NO_HOME_POSITION = -17,
	ONVIF_ERR_NO_TOKEN = - 18,
	ONVIF_ERR_PRESET_EXIST = -19,
	ONVIF_ERR_TOO_MANY_PRESETS = -20,
	ONVIF_ERR_MOVING_PTZ = -21,
	ONVIF_ERR_NO_ENTITY = -22,
	ONVIF_ERR_INVALID_NETWORK_INTERFACE = -23, 
	ONVIF_ERR_INVALID_MTU_VALUE = -24,
	ONVIF_ERR_CONFIG_MODIFY = -25,
	ONVIF_ERR_CONFIGURATION_CONFLICT = -26,
	ONVIF_ERR_INVALID_POSIION = -27,
	ONVIF_ERR_TOO_MANY_SCOPES = -28,
	ONVIF_ERR_FIXED_SCOPE = -29,
	ONVIF_ERR_NO_SCOPE = -30,
	ONVIF_ERR_SCOPE_OVERWRITE = -31,
	ONVIF_ERR_RESOURCE_UNKNOWN_FAULT = -32,
	ONVIF_ERR_NO_SOURCE = -33,
	ONVIF_ERR_CANNOT_OVERWRITE_HOME = -34,
	ONVIF_ERR_SETTINGS_INVALID = -35,
	ONVIF_ERR_NO_IMAGEING_FOR_SOURCE = -36,
	ONVIF_ERR_USERNAME_EXIST = -37,
	ONVIF_ERR_PASSWORD_TOO_LONG = -38,
	ONVIF_ERR_USERNAME_TOO_LONG = -39,
	ONVIF_ERR_PASSWORD = -40,
	ONVIF_ERR_TOO_MANY_USERS = -41,
	ONVIF_ERR_ANONYMOUS_NOT_ALLOWED = -42,
	ONVIF_ERR_USERNAME_TOO_SHORT = -43,
	ONVIF_ERR_USERNAME_MISSING = -44,
	ONVIF_ERR_FIXED_USER = -45,
	ONVIF_ERR_MAX_OSDS = -46,
	ONVIF_ERR_INVALID_STREAMSETUP = -47,
	ONVIF_ERR_BAD_CONFIGURATION = -48,
	ONVIF_ERR_MAX_RECORDING = -49,
	ONVIF_ERR_NO_RECORDING = -50,
	ONVIF_ERR_CANNOT_DELETE = -51,
	ONVIF_ERR_MAX_TRACKS = -52,
	ONVIF_ERR_NO_TRACK = -53,
	ONVIF_ERR_MAX_RECORDING_JOBS = -54,
	ONVIF_ERR_MAX_RECEIVERS = -55,
	ONVIF_ERR_NO_RECORDINGJOB = -56,
	ONVIF_ERR_BAD_MODE = -57,
	ONVIF_ERR_INVALID_TOKEN = -58,
	
	ONVIF_ERR_INVALID_RULE = -59,
	ONVIF_ERR_RULE_ALREADY_EXIST = -60,
	ONVIF_ERR_TOO_MANY_RULES = -61,
	ONVIF_ERR_RULE_NOT_EXIST = -63,
	ONVIF_ERR_NAME_ALREADY_EXIST = -64,
	ONVIF_ERR_TOO_MANY_MODULES = -65,
	ONVIF_ERR_INVALID_MODULE = -66,
	ONVIF_ERR_NAME_NOT_EXIST = -67, 

	ONVIF_ERR_INVALID_FILTER_FAULT = -68,
	ONVIF_ERR_INVALID_TOPIC_EXPRESSION_FAULT = -69,
	ONVIF_ERR_TOPIC_NOT_SUPPORTED_FAULT = -70,
	ONVIF_ERR_INVALID_MESSAGE_CONTENT_EXPRESSION_FAULT = -71,
	
} ONVIF_RET;

/***************************************************************************************/

typedef enum 
{
	CapabilityCategory_Invalid = -1,
	CapabilityCategory_All = 0, 
	CapabilityCategory_Analytics = 1, 
	CapabilityCategory_Device = 2, 
	CapabilityCategory_Events = 3, 
	CapabilityCategory_Imaging = 4, 
	CapabilityCategory_Media = 5, 
	CapabilityCategory_PTZ = 6,
	CapabilityCategory_Recording = 7,
	CapabilityCategory_Search = 8,
	CapabilityCategory_Replay = 9
} onvif_CapabilityCategory;

typedef enum  
{
	VideoEncoding_JPEG = 0, 
	VideoEncoding_MPEG4 = 1, 
	VideoEncoding_H264 = 2
} onvif_VideoEncoding;

typedef enum  
{
	AudioEncoding_G711 = 0, 
	AudioEncoding_G726 = 1, 
	AudioEncoding_AAC = 2
} onvif_AudioEncoding;

typedef enum H264Profile 
{
	H264Profile_Baseline = 0, 
	H264Profile_Main = 1, 
	H264Profile_Extended = 2, 
	H264Profile_High = 3
} onvif_H264Profile;

typedef enum  
{
	Mpeg4Profile_SP = 0,
	Mpeg4Profile_ASP = 1
} onvif_Mpeg4Profile;

typedef enum  
{
	UserLevel_Administrator = 0, 
	UserLevel_Operator = 1,
	UserLevel_User = 2, 
	UserLevel_Anonymous = 3,
	UserLevel_Extended = 4
} onvif_UserLevel;

typedef enum MoveStatus 
{
	MoveStatus_IDLE = 0, 
	MoveStatus_MOVING = 1, 
	MoveStatus_UNKNOWN = 2
} onvif_MoveStatus;

// OSD type
typedef enum 
{
	OSDType_Text = 0,
	OSDType_Image = 1,
	OSDType_Extended =2
} onvif_OSDType;

// OSD position type
typedef enum
{
	OSDPosType_UpperLeft = 0,
	OSDPosType_UpperRight = 1,
	OSDPosType_LowerLeft = 2,
	OSDPosType_LowerRight = 3,
	OSDPosType_Custom = 4
} onvif_OSDPosType;

typedef enum
{
	OSDTextType_Plain,								// The Plain type means the OSD is shown as a text string which defined in the "PlainText" item
	OSDTextType_Date,								// The Date type means the OSD is shown as a date, format of which should be present in the "DateFormat" item
	OSDTextType_Time,								// The Time type means the OSD is shown as a time, format of which should be present in the "TimeFormat" item
	OSDTextType_DateAndTime,						// The DateAndTime type means the OSD is shown as date and time, format of which should be present in the "DateFormat" and the "TimeFormat" item
} onvif_OSDTextType;

// BacklightCompensation mode
typedef enum 
{
	BacklightCompensationMode_OFF = 0, 				// Backlight compensation is disabled
	BacklightCompensationMode_ON = 1				// Backlight compensation is enabled
} onvif_BacklightCompensationMode;

// Exposure mode
typedef enum  
{
	ExposureMode_AUTO = 0, 
	ExposureMode_MANUAL = 1
} onvif_ExposureMode;

// Exposure Priority
typedef enum  
{
	ExposurePriority_LowNoise = 0, 
	ExposurePriority_FrameRate = 1
} onvif_ExposurePriority;

// AutoFocus Mode
typedef enum 
{
	AutoFocusMode_AUTO = 0, 
	AutoFocusMode_MANUAL = 1
} onvif_AutoFocusMode;

typedef enum  
{
	WideDynamicMode_OFF = 0, 
	WideDynamicMode_ON = 1
} onvif_WideDynamicMode;

typedef enum  
{
	IrCutFilterMode_ON = 0,
	IrCutFilterMode_OFF = 1, 
	IrCutFilterMode_AUTO = 2
} onvif_IrCutFilterMode;

typedef enum WhiteBalanceMode 
{
	WhiteBalanceMode_AUTO = 0, 
	WhiteBalanceMode_MANUAL = 1
} onvif_WhiteBalanceMode;

typedef enum onvif_EFlipMode 
{
	EFlipMode_OFF = 0, 
	EFlipMode_ON = 1, 
	EFlipMode_Extended = 2
} onvif_EFlipMode;

typedef enum 
{
	ReverseMode_OFF = 0, 
	ReverseMode_ON = 1, 
	ReverseMode_AUTO = 2, 
	ReverseMode_Extended = 3
} onvif_ReverseMode;

typedef enum  
{
	DiscoveryMode_Discoverable = 0, 
	DiscoveryMode_NonDiscoverable = 1
} onvif_DiscoveryMode;

typedef enum  
{
	SetDateTimeType_Manual = 0, 
	SetDateTimeType_NTP = 1
} onvif_SetDateTimeType;

typedef enum  
{
	StreamType_Invalid = -1,
	StreamType_RTP_Unicast = 0,
	StreamType_RTP_Multicast = 1
} onvif_StreamType;

typedef enum  
{
	TransportProtocol_Invalid = -1,
	TransportProtocol_UDP = 0, 
	TransportProtocol_TCP = 1, 
	TransportProtocol_RTSP = 2, 
	TransportProtocol_HTTP = 3
} onvif_TransportProtocol;

typedef enum
{
	TrackType_Invalid = -1,
	TrackType_Video = 0, 
	TrackType_Audio = 1, 
	TrackType_Metadata = 2,
	TrackType_Extended = 3
} onvif_TrackType;

typedef enum
{
	PropertyOperation_Invalid = -1,
	PropertyOperation_Initialized = 0,
	PropertyOperation_Deleted = 1, 
	PropertyOperation_Changed = 2
} onvif_PropertyOperation;

typedef enum 
{ 
	RecordingStatus_Initiated = 0, 
	RecordingStatus_Recording = 1, 
	RecordingStatus_Stopped = 2, 
	RecordingStatus_Removing = 3, 
	RecordingStatus_Removed = 4, 
	RecordingStatus_Unknown = 5 
} onvif_RecordingStatus;

typedef enum 
{ 
	SearchState_Queued = 0, 
	SearchState_Searching = 1, 
	SearchState_Completed = 2, 
	SearchState_Unknown = 3 
} onvif_SearchState;


/***************************************************************************************/
typedef struct 
{
    float 	Min;									// required
    float 	Max;									// required
} onvif_FloatRange;

typedef struct
{
	float	x;										// required
	float	y;										// required
} onvif_Vector;


/* device capabilities */
typedef struct
{
	// network capabilities
	uint32	IPFilter 			: 1; 				// Indicates support for IP filtering
	uint32	ZeroConfiguration 	: 1; 				// Indicates support for zeroconf
	uint32	IPVersion6			: 1; 				// Indicates support for IPv6
	uint32 	DynDNS 				: 1; 				// Indicates support for dynamic DNS configuration
	uint32  Dot11Configuration  : 1; 				// Indicates support for IEEE 802.11 configuration
	uint32  HostnameFromDHCP    : 1; 				// Indicates support for retrieval of hostname from DHCP
	uint32  DHCPv6              : 1; 				// Indicates support for Stateful IPv6 DHCP

	// system capabilities
	uint32 	DiscoveryResolve 	: 1; 				// Indicates support for WS Discovery resolve requests
	uint32 	DiscoveryBye 		: 1; 				// Indicates support for WS-Discovery Bye
	uint32 	RemoteDiscovery 	: 1; 				// Indicates support for remote discovery
	uint32 	SystemBackup 		: 1; 				// Indicates support for system backup through MTOM
	uint32 	SystemLogging 		: 1; 				// Indicates support for retrieval of system logging through MTOM
	uint32 	FirmwareUpgrade 	: 1; 				// Indicates support for firmware upgrade through MTOM
	uint32  HttpFirmwareUpgrade : 1; 				// Indicates support for system backup through MTOM
	uint32  HttpSystemBackup    : 1; 				// Indicates support for system backup through HTTP
	uint32  HttpSystemLogging   : 1; 				// Indicates support for retrieval of system logging through HTTP
	uint32  HttpSupportInformation : 1; 			// Indicates support for retrieving support information through HTTP

    // scurity capabilities
    uint32  TLS10               : 1; 				// Indicates support for TLS 1.0
    uint32  TLS11               : 1; 				// Indicates support for TLS 1.1
    uint32  TLS12               : 1; 				// Indicates support for TLS 1.2
    uint32  OnboardKeyGeneration: 1; 				// Indicates support for onboard key generation
    uint32  AccessPolicyConfig  : 1; 				// Indicates support for access policy configuration
    uint32  DefaultAccessPolicy : 1; 				// Indicates support for the ONVIF default access policy
    uint32  Dot1X               : 1; 				// Indicates support for IEEE 802.1X configuration
    uint32  RemoteUserHandling  : 1; 				// Indicates support for remote user configuration. Used when accessing another device
    uint32  X509Token           : 1; 				// Indicates support for WS-Security X.509 token
    uint32  SAMLToken           : 1; 				// Indicates support for WS-Security SAML token
    uint32  KerberosToken       : 1; 				// Indicates support for WS-Security Kerberos token
    uint32  UsernameToken       : 1; 				// Indicates support for WS-Security Username token
    uint32  HttpDigest          : 1; 				// Indicates support for WS over HTTP digest authenticated communication layer
    uint32  RELToken            : 1; 				// Indicates support for WS-Security REL token
    
	uint32	reserved			: 1;
    
	int     Dot1XConfigurations;    				// Indicates the maximum number of Dot1X configurations supported by the device
	int     NTP;                    				// Maximum number of NTP servers supported by the devices SetNTP command
	int     SupportedEAPMethods;    				// EAP Methods supported by the device. 
	                                				// The int values refer to the <a href="http://www.iana.org/assignments/eap-numbers/eap-numbers.xhtml">IANA EAP Registry</a>
	int     MaxUsers;               				// The maximum number of users that the device supports

	// misc capabilities
    char    AuxiliaryCommands[100]; 				// Lists of commands supported by SendAuxiliaryCommand
    
    char 	XAddr[100];
} onvif_DevicesCapabilities;

/* media capabilities */
typedef struct
{
    uint32  SnapshotUri         : 1; 				// Indicates if GetSnapshotUri is supported
    uint32  Rotation            : 1; 				// Indicates whether or not Rotation feature is supported
    uint32  VideoSourceMode     : 1; 				// Indicates the support for changing video source mode
    uint32  OSD                 : 1; 				// Indicates if OSD is supported
	uint32	RTPMulticast 		: 1; 				// Indicates support for RTP multicast
	uint32	RTP_TCP				: 1; 				// Indicates support for RTP over TCP
	uint32	RTP_RTSP_TCP		: 1; 				// Indicates support for RTP/RTSP/TCP
	uint32  NonAggregateControl : 1; 				// Indicates support for non aggregate RTSP control
	uint32  NoRTSPStreaming     : 1; 				// Indicates the device does not support live media streaming via RTSP
	uint32  support				: 1; 				// Indication if the device supports media service
	uint32	reserved			: 22;

	int     MaximumNumberOfProfiles; 				// Maximum number of profiles supported
	
    char 	XAddr[100];
} onvif_MediaCapabilities;

/* PTZ capabilities */
typedef struct
{
    uint32  EFlip               : 1;	 			// Indicates whether or not EFlip is supported
    uint32  Reverse             : 1; 				// Indicates whether or not reversing of PT control direction is supported
    uint32  GetCompatibleConfigurations : 1; 		// Indicates support for the GetCompatibleConfigurations command
    uint32  support				: 1; 				// Indication if the device supports ptz service
	uint32	reserved 			: 28;
	
    char 	XAddr[100];
} onvif_PTZCapabilities;

/* event capabilities */
typedef struct
{
	uint32	WSSubscriptionPolicySupport	: 1; 		// Indicates that the WS Subscription policy is supported
	uint32	WSPullPointSupport 	: 1; 				// Indicates that the WS Pull Point is supported
	uint32	WSPausableSubscriptionManagerInterfaceSupport : 1; // Indicates that the WS Pausable Subscription Manager Interface is supported
	uint32  PersistentNotificationStorage : 1; 		// Indication if the device supports persistent notification storage
	uint32  support				: 1; 				// Indication if the device supports events service
	uint32	reserved 			: 27;

	int     MaxNotificationProducers; 				// Maximum number of supported notification producers as defined by WS-BaseNotification
	int     MaxPullPoints;            				// Maximum supported number of notification pull points
	
    char 	XAddr[100];
} onvif_EventCapabilities;

/* image capabilities */
typedef struct
{
    uint32  ImageStabilization  : 1; 				// Indicates whether or not Image Stabilization feature is supported
	uint32  support				: 1; 				// Indication if the device supports image service
	uint32	reserved 			: 30;
	
    char 	XAddr[100];
} onvif_ImagingCapabilities;

/* analytics capabilities*/
typedef struct
{
	uint32  RuleSupport			: 1;				// Indication that the device supports the rules interface and the rules syntax
	uint32  AnalyticsModuleSupport : 1;				// Indication that the device supports the scene analytics module interface
	uint32  CellBasedSceneDescriptionSupported : 1;	// Indication that the device produces the cell based scene description
	uint32  support				: 1; 				// Indication if the device supports Analytics service
	uint32	reserved 			: 28;

	char 	XAddr[100];
} onvif_AnalyticsCapabilities;

/* recording capabilities */
typedef struct
{
    uint32  ReceiverSource      : 1;
    uint32  MediaProfileSource  : 1;
    uint32  DynamicRecordings   : 1; 				// Indication if the device supports dynamic creation and deletion of recordings
    uint32  DynamicTracks       : 1; 				// Indication if the device supports dynamic creation and deletion of tracks
    uint32  Options             : 1; 				// Indication if the device supports the GetRecordingOptions command
    uint32  MetadataRecording   : 1; 				// Indication if the device supports recording metadata
    uint32  JPEG                : 1; 				// Indication if supports JPEG encoding
    uint32  MPEG4               : 1; 				// Indication if supports MPEG4 encoding
    uint32  H264                : 1; 				// Indication if supports H264 encoding
    uint32  G711                : 1; 				// Indication if supports G711 encoding
    uint32  G726                : 1; 				// Indication if supports G726 encoding
    uint32  AAC                 : 1; 				// Indication if supports AAC encoding
    uint32  support				: 1; 				// Indication if the device supports recording service
	uint32	reserved 			: 19;

	uint32  MaxStringLength;
	float   MaxRate;            					// Maximum supported bit rate for all tracks of a recording in kBit/s
	float   MaxTotalRate;       					// Maximum supported bit rate for all recordings in kBit/s.
    int     MaxRecordings;      					// Maximum number of recordings supported.
    int     MaxRecordingJobs;   					// Maximum total number of supported recording jobs by the device
	
	char 	XAddr[100];
} onvif_RecordingCapabilities;

/* search capabilities */
typedef struct
{
    uint32  MetadataSearch      : 1;
    uint32  GeneralStartEvents  : 1; 				// Indicates support for general virtual property events in the FindEvents method
    uint32  support				: 1; 				// Indication if the device supports search service
	uint32	reserved 			: 29;
	
	char 	XAddr[100];
} onvif_SearchCapabilities;

/* replay capabilities */
typedef struct
{
    uint32  ReversePlayback     : 1; 				// Indicator that the Device supports reverse playback as defined in the ONVIF Streaming Specification
    uint32  RTP_RTSP_TCP        : 1; 				// Indicates support for RTP/RTSP/TCP
    uint32  support				: 1; 				// Indication if the device supports replay service
	uint32	reserved 			: 29;

	onvif_FloatRange	SessionTimeoutRange; 		// The minimum and maximum valid values supported as session timeout in seconds
	
	char 	XAddr[100];
} onvif_ReplayCapabilities;

typedef struct
{
	onvif_DevicesCapabilities	device;     		// The capabilities for the device service is returned in the Capabilities element
    onvif_MediaCapabilities		media;      		// The capabilities for the media service is returned in the Capabilities element
    onvif_PTZCapabilities		ptz;        		// The capabilities for the PTZ service is returned in the Capabilities element
	onvif_EventCapabilities		events;     		// The capabilities for the event service is returned in the Capabilities element
	onvif_ImagingCapabilities	image;      		// The capabilities for the imaging service is returned in the Capabilities element

#ifdef VIDEO_ANALYTICS
	onvif_AnalyticsCapabilities	analytics;			// The capabilities for the analytics service is returned in the Capabilities element
#endif

#ifdef PROFILE_G_SUPPORT
	onvif_RecordingCapabilities	recording;  		// The capabilities for the recording service is returned in the Capabilities element
	onvif_SearchCapabilities	search;     		// The capabilities for the search service is returned in the Capabilities element
	onvif_ReplayCapabilities	replay;     		// The capabilities for the replay service is returned in the Capabilities element
#endif	
} onvif_Capabilities;

typedef struct
{
	char 	Manufacturer[64];						// required, The manufactor of the device
	char 	Model[64];								// required, The device model
	char 	FirmwareVersion[64];					// required, The firmware version in the device
	char 	SerialNumber[64];						// required, The serial number of the device
	char 	HardwareId[64];							// required, The hardware ID of the device
} onvif_DeviceInformation;

typedef struct 
{
	int 	Width;									// required
	int 	Height;									// required
} onvif_VideoResolution;

typedef struct
{
	int		Min;									// required								
	int 	Max;									// required
} onvif_IntRange;

typedef struct
{
	int     x;	                                    // required 
	int     y;	                                    // required 
	int     width;	                                // required 
	int     height;	                                // required 
} onvif_IntRectangle;

typedef struct
{
	float   bottom;	                                // required
	float   top;	                                // required
	float   right;	                                // required
	float   left;	                                // required
} onvif_Rectangle;

typedef struct
{
	uint32	LevelFlag	: 1;						// Indicates whether the field Level is valid
	uint32  Reserved	: 31;
	
    onvif_BacklightCompensationMode	Mode;           // required, Backlight compensation mode (on/off)
    float   Level;                                  // optional, Optional level parameter (unit unspecified)
} onvif_BacklightCompensation;

typedef struct
{
	uint32	PriorityFlag		: 1;				// Indicates whether the field Priority is valid
	uint32	MinExposureTimeFlag	: 1;				// Indicates whether the field MinExposureTime is valid
	uint32	MaxExposureTimeFlag	: 1;				// Indicates whether the field MaxExposureTime is valid
	uint32	MinGainFlag			: 1;				// Indicates whether the field MinGain is valid
	uint32	MaxGainFlag			: 1;				// Indicates whether the field MaxGain is valid
	uint32	MinIrisFlag			: 1;				// Indicates whether the field MinIris is valid
	uint32	MaxIrisFlag			: 1;				// Indicates whether the field MaxIris is valid
	uint32	ExposureTimeFlag	: 1;				// Indicates whether the field ExposureTime is valid
	uint32	GainFlag			: 1;				// Indicates whether the field Gain is valid
	uint32	IrisFlag			: 1;				// Indicates whether the field Iris is valid
	uint32  Reserved			: 22;
	
    onvif_ExposureMode		Mode;                   // required, Auto - Enabled the exposure algorithm on the device; Manual - Disabled exposure algorithm on the device
    onvif_ExposurePriority 	Priority;               // optional, The exposure priority mode (low noise/framerate)
    onvif_Rectangle         Window;                 // required, 
	
    float   MinExposureTime;	                    // optional, Minimum value of exposure time range allowed to be used by the algorithm
	float   MaxExposureTime;	                    // optional, Maximum value of exposure time range allowed to be used by the algorithm
	float   MinGain;	                            // optional, Minimum value of the sensor gain range that is allowed to be used by the algorithm
	float   MaxGain;	                            // optional, Maximum value of the sensor gain range that is allowed to be used by the algorithm
	float   MinIris;	                            // optional, Minimum value of the iris range allowed to be used by the algorithm
	float   MaxIris;	                            // optional, Maximum value of the iris range allowed to be used by the algorithm
	float   ExposureTime;	                        // optional, The fixed exposure time used by the image sensor
	float   Gain;	                                // optional, The fixed gain used by the image sensor (dB)
	float   Iris;	                                // optional, The fixed attenuation of input light affected by the iris (dB). 0dB maps to a fully opened iris
} onvif_Exposure;

typedef struct
{
	uint32	DefaultSpeedFlag	: 1;				// Indicates whether the field DefaultSpeed is valid
	uint32	NearLimitFlag		: 1;				// Indicates whether the field NearLimit is valid
	uint32	FarLimitFlag		: 1;				// Indicates whether the field FarLimit is valid	
	uint32  Reserved			: 29;
	
    onvif_AutoFocusMode	AutoFocusMode;              // required, Mode of auto fucus
    
    float   DefaultSpeed;	                        // optional, 
	float   NearLimit;	                            // optional, Parameter to set autofocus near limit (unit: meter)
	float   FarLimit;	                            // optional, Parameter to set autofocus far limit (unit: meter)
} onvif_FocusConfiguration;

typedef struct
{
	uint32	LevelFlag	: 1;						// Indicates whether the field Level is valid
	uint32  Reserved	: 31;
	
    onvif_WideDynamicMode	Mode;                   // required, Wide dynamic range mode (on/off), 0-OFF, 1-ON
    float   Level;	                                // optional, Optional level parameter (unit unspecified)
} onvif_WideDynamicRange; 

typedef struct
{
	uint32	CrGainFlag	: 1;						// Indicates whether the field CrGain is valid
	uint32	CbGainFlag	: 1;						// Indicates whether the field CbGain is valid
	uint32  Reserved	: 30;
	
    onvif_WhiteBalanceMode	Mode;	                // required, 'AUTO' or 'MANUAL'
    
	float   CrGain;	                                // optional, Rgain (unitless)
	float   CbGain;	                                // optional, Bgain (unitless)
} onvif_WhiteBalance; 

typedef struct
{
	uint32	BacklightCompensationFlag	: 1;		// Indicates whether the field BacklightCompensation is valid
	uint32	BrightnessFlag				: 1;		// Indicates whether the field Brightness is valid
	uint32	ColorSaturationFlag			: 1;		// Indicates whether the field ColorSaturation is valid
	uint32	ContrastFlag				: 1;		// Indicates whether the field Contrast is valid
	uint32	ExposureFlag				: 1;		// Indicates whether the field Exposure is valid
	uint32	FocusFlag					: 1;		// Indicates whether the field Focus is valid
	uint32	IrCutFilterFlag				: 1;		// Indicates whether the field IrCutFilter is valid
	uint32	SharpnessFlag				: 1;		// Indicates whether the field Sharpness is valid
	uint32	WideDynamicRangeFlag		: 1;		// Indicates whether the field WideDynamicRange is valid
	uint32	WhiteBalanceFlag			: 1;		// Indicates whether the field WhiteBalance is valid
	uint32  Reserved					: 22;
	
	onvif_BacklightCompensation BacklightCompensation;  // optional, Enabled/disabled BLC mode (on/off)
	float   Brightness;                             // optional, Image brightness (unit unspecified)
	float   ColorSaturation;                        // optional, Color saturation of the image (unit unspecified)
	float   Contrast;                               // optional, Contrast of the image (unit unspecified)
	onvif_Exposure Exposure;                        // optional, Exposure mode of the device
	onvif_FocusConfiguration 	Focus;              // optional, Focus configuration
    onvif_IrCutFilterMode    	IrCutFilter;        // optional, Infrared Cutoff Filter settings	
	float   Sharpness;                              // optional, Sharpness of the Video image
	onvif_WideDynamicRange 		WideDynamicRange;   // optional, WDR settings
	onvif_WhiteBalance 			WhiteBalance;       // optional, White balance settings	
} onvif_ImagingSettings;

typedef struct
{
    uint32	Mode_ON	    : 1;                		// Indicates whether mode ON is valid
	uint32 	Mode_OFF	: 1;                		// Indicates whether mode OFF is valid
	uint32  LevelFlag	: 1;						// Indicates whether the field LevelFlag is valid
	uint32 	Reserved    : 29;
	
    onvif_FloatRange	Level;                   	// optional, Level range of BacklightCompensation
} onvif_BacklightCompensationOptions;

typedef struct
{
    uint32	Mode_AUTO           : 1;        		// Indicates whether mode AUTO is valid
	uint32	Mode_MANUAL         : 1;        		// Indicates whether mode Manual is valid
    uint32	Priority_LowNoise   : 1;       	 		// Indicates whether Priority LowNoise is valid
	uint32	Priority_FrameRate  : 1;        		// Indicates whether Priority FrameRate is valid
	uint32	MinExposureTimeFlag	: 1;				// Indicates whether the field MinExposureTime is valid
	uint32	MaxExposureTimeFlag	: 1;				// Indicates whether the field MaxExposureTime is valid
	uint32	MinGainFlag			: 1;				// Indicates whether the field MinGain is valid
	uint32	MaxGainFlag			: 1;				// Indicates whether the field MaxGain is valid
	uint32	MinIrisFlag			: 1;				// Indicates whether the field MinIris is valid
	uint32	MaxIrisFlag			: 1;				// Indicates whether the field MaxIris is valid
	uint32	ExposureTimeFlag	: 1;				// Indicates whether the field ExposureTime is valid
	uint32	GainFlag			: 1;				// Indicates whether the field Gain is valid
	uint32	IrisFlag			: 1;				// Indicates whether the field Iris is valid
	uint32	Reserved            : 19;

	onvif_FloatRange	MinExposureTime;        	// optional, Valid range of the Minimum ExposureTime
    onvif_FloatRange	MaxExposureTime;          	// optional, Valid range of the Maximum ExposureTime
    onvif_FloatRange	MinGain;                  	// optional, Valid range of the Minimum Gain
    onvif_FloatRange	MaxGain;                 	// optional, Valid range of the Maximum Gain
    onvif_FloatRange	MinIris;                 	// optional, Valid range of the Minimum Iris
    onvif_FloatRange	MaxIris;                 	// optional, Valid range of the Maximum Iris
    onvif_FloatRange	ExposureTime;            	// optional, Valid range of the ExposureTime
    onvif_FloatRange	Gain;                   	// optional, Valid range of the Gain
    onvif_FloatRange	Iris;                    	// optional, Valid range of the Iris
} onvif_ExposureOptions;

typedef struct
{
    uint32	AutoFocusModes_AUTO	    : 1;    		// Indicates whether mode aUTO is valid
	uint32 	AutoFocusModes_MANUAL   : 1;   	 		// Indicates whether mode Manual is valid
	uint32	DefaultSpeedFlag		: 1;			// Indicates whether the field DefaultSpeed is valid
	uint32	NearLimitFlag			: 1;			// Indicates whether the field NearLimit is valid
	uint32	FarLimitFlag			: 1;			// Indicates whether the field FarLimit is valid
	uint32 	Reserved                : 27;

	onvif_FloatRange	DefaultSpeed;          		// optional, Valid range of DefaultSpeed
	onvif_FloatRange	NearLimit;              	// optional, Valid range of NearLimit
	onvif_FloatRange	FarLimit;               	// optional, Valid range of FarLimit
} onvif_FocusOptions;

typedef struct
{
    uint32 	Mode_ON	    : 1;                		// Indicates whether mode ON is valid
	uint32 	Mode_OFF	: 1;                		// Indicates whether mode OFF is valid
	uint32	LevelFlag	: 1;						// Indicates whether the field Level is valid
	uint32 	Reserved    : 29;
	
    onvif_FloatRange	Level;                 		// optional, Valid range of Level
} onvif_WideDynamicRangeOptions;

typedef struct
{
    uint32	Mode_AUTO 	: 1;        				// Indicates whether mode AUDO is valid
	uint32	Mode_MANUAL	: 1;        				// Indicates whether mode Manual is valid
	uint32	YrGainFlag	: 1;						// Indicates whether the field CrGain is valid
	uint32	YbGainFlag	: 1;						// Indicates whether the field CbGain is valid
	uint32 	Reserved	: 28;

	onvif_FloatRange	YrGain;                 	// optional, Valid range of YrGain
	onvif_FloatRange	YbGain;                  	// optional, Valid range of YbGain
} onvif_WhiteBalanceOptions;

typedef struct
{
	uint32	IrCutFilterMode_ON	    	: 1; 		// Indicates whether IrCutFilter mode ON is valid
	uint32 	IrCutFilterMode_OFF			: 1; 		// Indicates whether IrCutFilter mode OFF is valid
	uint32 	IrCutFilterMode_AUTO		: 1; 		// Indicates whether IrCutFilter mode AUTO is valid
	uint32	BacklightCompensationFlag	: 1;		// Indicates whether the field BacklightCompensation is valid	
	uint32	BrightnessFlag				: 1;		// Indicates whether the field Brightness is valid
	uint32  ColorSaturationFlag			: 1;		// Indicates whether the field ColorSaturation is valid
	uint32  ContrastFlag				: 1;		// Indicates whether the field Contrast is valid
	uint32  ExposureFlag				: 1;		// Indicates whether the field Exposure is valid
	uint32  FocusFlag					: 1;		// Indicates whether the field Focus is valid
	uint32  SharpnessFlag				: 1;		// Indicates whether the field Sharpness is valid
	uint32  WideDynamicRangeFlag		: 1;		// Indicates whether the field WideDynamicRange is valid
	uint32  WhiteBalanceFlag			: 1;		// Indicates whether the field WhiteBalance is valid
	uint32 	Reserved    				: 20;
	
    onvif_BacklightCompensationOptions  BacklightCompensation;  // optional, Valid range of Backlight Compensation
    
    onvif_FloatRange      	Brightness;          	// optional, Valid range of Brightness
    onvif_FloatRange      	ColorSaturation;    	// optional, alid range of Color Saturation
    onvif_FloatRange      	Contrast;            	// optional, Valid range of Contrast

    onvif_ExposureOptions   Exposure;           	// optional, Valid range of Exposure    
    onvif_FocusOptions      Focus;              	// optional, Valid range of Focus

    onvif_FloatRange      	Sharpness;           	// optional, Valid range of Sharpness
    
    onvif_WideDynamicRangeOptions   WideDynamicRange;   // optional, Valid range of WideDynamicRange
    onvif_WhiteBalanceOptions		WhiteBalance;	// optional, Valid range of WhiteBalance
} onvif_ImagingOptions;

typedef struct 
{
	uint32	ErrorFlag	: 1; 						// Indicates whether the field Error is valid
	uint32	Reserved 	: 31;
	
	float 	Position;								// required, Status of focus position
	onvif_MoveStatus	MoveStatus;					// required, Status of focus MoveStatus
	char 	Error[100];								// optional, Error status of focus
} onvif_FocusStatus;

typedef struct 
{
	uint32	FocusStatusFlag	: 1; 					// Indicates whether the field FocusStatus is valid
	uint32	Reserved 		: 31;
	
	onvif_FocusStatus	FocusStatus;				// optional, Status of focus
} onvif_ImagingStatus;

typedef struct
{
    char    token[ONVIF_TOKEN_LEN];	                // required
	float   Framerate;	                            // required, Frame rate in frames per second
	
	onvif_VideoResolution   Resolution;	            // required, Horizontal and vertical resolution 
	onvif_ImagingSettings	Imaging;
} onvif_VideoSource;

typedef struct
{
	char 	Name[ONVIF_NAME_LEN];					// required, User readable name. Length up to 64 characters
	int 	UseCount;								// required, Number of internal references currently using this configuration. This parameter is read-only and cannot be changed by a set request
	char 	token[ONVIF_TOKEN_LEN];					// required, Token that uniquely refernces this configuration. Length up to 64 characters
	char 	SourceToken[ONVIF_TOKEN_LEN];			// required, Reference to the physical input

	onvif_IntRectangle	Bounds;					    // required, Rectangle specifying the Video capturing area. The capturing area shall not be larger than the whole Video source area	
} onvif_VideoSourceConfiguration;

typedef struct
{
	onvif_IntRange  XRange;	                        // required
	onvif_IntRange  YRange;	                        // required
	onvif_IntRange  WidthRange;	                    // required
	onvif_IntRange  HeightRange;	                // required
} onvif_IntRectangleRange;

typedef struct
{
	onvif_IntRectangleRange BoundsRange;	        // required 
} onvif_VideoSourceConfigurationOptions;

typedef struct
{
	int		FrameRateLimit;							// required, Maximum output framerate in fps. If an EncodingInterval is provided the resulting encoded framerate will be reduced by the given factor
	int		EncodingInterval;						// required, Interval at which images are encoded and transmitted. (A value of 1 means that every frame is encoded, a value of 2 means that every 2nd frame is encoded ...)
	int		BitrateLimit;							// required, the maximum output bitrate in kbps
} onvif_VideoRateControl;

typedef struct
{
	int		GovLength;								// required, Determines the interval in which the I-Frames will be coded. An entry of 1 indicates I-Frames are continuously generated. 
													//	An entry of 2 indicates that every 2nd image is an I-Frame, and 3 only every 3rd frame, etc. The frames in between are coded as P or B Frames.
	onvif_Mpeg4Profile	Mpeg4Profile;				// required, the Mpeg4 profile, either simple profile (SP) or advanced simple profile (ASP)													
} onvif_Mpeg4Configuration;

typedef struct
{
	int		GovLength;								// required, Group of Video frames length. Determines typically the interval in which the I-Frames will be coded. An entry of 1 indicates I-Frames are continuously generated. 
													//	An entry of 2 indicates that every 2nd image is an I-Frame, and 3 only every 3rd frame, etc. The frames in between are coded as P or B Frames
	onvif_H264Profile	H264Profile;				// required, the H.264 profile, either baseline, main, extended or high
} onvif_H264Configuration;

typedef struct
{
	char	IPv4Address[32];						// required, The multicast address
	int		Port;									// required, The RTP mutlicast destination port. A device may support RTCP. In this case the port value shall be even to allow the corresponding RTCP stream to be mapped
													//	to the next higher (odd) destination port number as defined in the RTSP specification
	int		TTL;									// required, In case of IPv6 the TTL value is assumed as the hop limit. Note that for IPV6 and administratively scoped IPv4 multicast the primary use for hop limit / TTL is 
													//    to prevent packets from (endlessly) circulating and not limiting scope. In these cases the address contains the scope
	BOOL	AutoStart;								// required, Read only property signalling that streaming is persistant. Use the methods StartMulticastStreaming and StopMulticastStreaming to switch its state													
} onvif_MulticastConfiguration;

typedef struct
{
	uint32	RateControlFlag	: 1;					// Indicates whether the field RateControl is valid
	uint32	MPEG4Flag		: 1;					// Indicates whether the field MPEG4 is valid
	uint32	H264Flag		: 1;					// Indicates whether the field H264 is valid
	uint32 	Reserved		: 29;
	
	char 	Name[ONVIF_NAME_LEN];					// required, User readable name. Length up to 64 characters
	int 	UseCount;								// required, Number of internal references currently using this configuration. This parameter is read-only and cannot be changed by a set request
	char 	token[ONVIF_TOKEN_LEN];					// required, Token that uniquely refernces this configuration. Length up to 64 characters

	onvif_VideoEncoding		    Encoding;		    // required, Used video codec, either Jpeg, H.264 or Mpeg4
	onvif_VideoResolution	    Resolution;		    // required, Configured video resolution

	int		Quality;								// required, Relative value for the video quantizers and the quality of the video. A high value within supported quality range means higher quality

	onvif_VideoRateControl	    RateControl;	    // optional, Optional element to configure rate control related parameters
	onvif_Mpeg4Configuration	MPEG4;			    // optional, Optional element to configure Mpeg4 related parameters
	onvif_H264Configuration	    H264;			    // optional, Optional element to configure H.264 related parameters
	
	onvif_MulticastConfiguration    Multicast;	    // required, Defines the multicast settings that could be used for video streaming
	
	int		SessionTimeout;							// required, The rtsp session timeout for the related video stream, unit is second
} onvif_VideoEncoderConfiguration;

typedef struct 
{
	char    token[ONVIF_TOKEN_LEN];	            	// required
	int     Channels;	                            // required, number of available audio channels. (1: mono, 2: stereo)
} onvif_AudioSource;

typedef struct
{
	char 	Name[ONVIF_NAME_LEN];					// required, User readable name. Length up to 64 characters
	int 	UseCount;								// required, Number of internal references currently using this configuration. This parameter is read-only and cannot be changed by a set request
	char 	token[ONVIF_TOKEN_LEN];					// required, Token that uniquely refernces this configuration. Length up to 64 characters
	
	char 	SourceToken[ONVIF_TOKEN_LEN];			// required, Token of the Audio Source the configuration applies to
} onvif_AudioSourceConfiguration;


typedef struct
{
	char 	Name[ONVIF_NAME_LEN];					// required, User readable name. Length up to 64 characters
	int 	UseCount;								// required, Number of internal references currently using this configuration. This parameter is read-only and cannot be changed by a set request
	char 	token[ONVIF_TOKEN_LEN];					// required, Token that uniquely refernces this configuration. Length up to 64 characters
	
	onvif_AudioEncoding Encoding;					// required, Audio codec used for encoding the audio input (either G.711, G.726 or AAC)

	int		Bitrate;								// required, The output bitrate in kbps
	int		SampleRate;								// required, The output sample rate in kHz

	onvif_MulticastConfiguration    Multicast;	    // required, Defines the multicast settings that could be used for video streaming

	int		SessionTimeout;							// required, The rtsp session timeout for the related audio stream, unit is second
} onvif_AudioEncoderConfiguration;

typedef struct 
{
	onvif_AudioEncoding Encoding;	                // required, The enoding used for audio data (either G.711, G.726 or AAC)
	
	int     BitrateList[10];	                    // required, List of supported bitrates in kbps for the specified Encoding
	int     SampleRateList[10];	                    // required, List of supported Sample Rates in kHz for the specified Encoding
} onvif_AudioEncoderConfigurationOption;

typedef struct
{
    int     sizeOptions;                            // required, valid Options numbers                           
    onvif_AudioEncoderConfigurationOption   Options[3];   // optional, list of supported AudioEncoderConfigurations
} onvif_AudioEncoderConfigurationOptions;

typedef struct
{
	onvif_VideoResolution	ResolutionsAvailable[MAX_RES_NUMS];	// required, List of supported image sizes

	onvif_IntRange	FrameRateRange;					// required, Supported frame rate in fps (frames per second)
	onvif_IntRange	EncodingIntervalRange;			// required, Supported encoding interval range. The encoding interval corresponds to the number of frames devided by the encoded frames. An encoding interval value of "1" means that all frames are encoded
} onvif_JpegOptions;

typedef struct
{	
	uint32 	Mpeg4Profile_SP 	: 1;				// required, Indicates whether the SP profile is valid
	uint32 	Mpeg4Profile_ASP 	: 1;				// required, Indicates whether the ASP profile is valid
	uint32 	Reserverd			: 30;

	onvif_VideoResolution	ResolutionsAvailable[MAX_RES_NUMS];	// required, List of supported image sizes

	onvif_IntRange	GovLengthRange;					// required, Supported group of Video frames length. This value typically corresponds to the I-Frame distance
	onvif_IntRange	FrameRateRange;					// required, Supported frame rate in fps (frames per second)
	onvif_IntRange	EncodingIntervalRange;			// required, Supported encoding interval range. The encoding interval corresponds to the number of frames devided by the encoded frames. An encoding interval value of "1" means that all frames are encoded	
} onvif_Mpeg4Options;

typedef struct
{
	uint32 	H264Profile_Baseline	: 1;			// required, Indicates whether the Baseline profile is valid				
	uint32 	H264Profile_Main 		: 1;			// required, Indicates whether the Main profile is valid	
	uint32 	H264Profile_Extended 	: 1;			// required, Indicates whether the Extended profile is valid	
	uint32 	H264Profile_High		: 1;			// required, Indicates whether the High profile is valid	
	uint32 	Reserverd				: 28;

	onvif_VideoResolution	ResolutionsAvailable[MAX_RES_NUMS];	// required, List of supported image sizes
	
	onvif_IntRange	GovLengthRange;					// required, Supported group of Video frames length. This value typically corresponds to the I-Frame distance
	onvif_IntRange	FrameRateRange;					// required, Supported frame rate in fps (frames per second)
	onvif_IntRange	EncodingIntervalRange;			// required, Supported encoding interval range. The encoding interval corresponds to the number of frames devided by the encoded frames. An encoding interval value of "1" means that all frames are encoded	
} onvif_H264Options;

typedef struct 
{
	uint32	JPEGFlag 	: 1;				        // Indicates whether the field JPEG is valid
	uint32	MPEG4Flag 	: 1;				        // Indicates whether the field MPEG4 is valid
	uint32	H264Flag 	: 1;				        // Indicates whether the field H264 is valid
	uint32	Reserved 	: 29;				
	
	onvif_IntRange		QualityRange;		        // required, Range of the quality values. A high value means higher quality
	
	onvif_JpegOptions 	JPEG;				        // optional, Optional JPEG encoder settings ranges
	onvif_Mpeg4Options	MPEG4;				        // optional, Optional MPEG-4 encoder settings ranges
	onvif_H264Options	H264;				        // optional, Optional H.264 encoder settings ranges	
} onvif_VideoEncoderConfigurationOptions;

typedef struct 
{
	BOOL 	Status;									// required, True if the metadata stream shall contain the PTZ status (IDLE, MOVING or UNKNOWN)
	BOOL 	Position;								// required, True if the metadata stream shall contain the PTZ position 
} onvif_PTZFilter;

typedef struct 
{
	uint32	AnalyticsFlag 	: 1;				    // Indicates whether the field Analytics is valid
	uint32	PTZStatusFlag 	: 1;				    // Indicates whether the field PTZStatus is valid
	uint32	Reserved 		: 30;
	
	char 	Name[ONVIF_NAME_LEN];					// required , User readable name. Length up to 64 characters
	int 	UseCount;								// required, Number of internal references currently using this configuration. This parameter is read-only and cannot be changed by a set request 
	char 	token[ONVIF_TOKEN_LEN];					// required, Token that uniquely refernces this configuration. Length up to 64 characters 
	BOOL 	Analytics;								// optional, Defines whether the streamed metadata will include metadata from the analytics engines (video, cell motion, audio etc.) 
	int 	SessionTimeout;							// required, The rtsp session timeout for the related audio stream, unit is second
	
	onvif_PTZFilter 		PTZStatus;				// optional, optional element to configure which PTZ related data is to include in the metadata stream 
	
	onvif_MulticastConfiguration Multicast;			// required, efines the multicast settings that could be used for video streaming
} onvif_MetadataConfiguration;

typedef struct 
{
	uint32	PanTiltPositionSupportedFlag	: 1;	// Indicates whether the field PanTiltPositionSupported is valid
	uint32	ZoomPositionSupportedFlag 		: 1;	// Indicates whether the field ZoomPositionSupported is valid
	uint32	Reserved 						: 30;
	
	BOOL 	PanTiltStatusSupported;					// required, True if the device is able to stream pan or tilt status information
	BOOL 	ZoomStatusSupported;					// required, True if the device is able to stream zoom status inforamtion
	BOOL 	PanTiltPositionSupported;				// optional, True if the device is able to stream the pan or tilt position
	BOOL 	ZoomPositionSupported;					// optional, True if the device is able to stream zoom position information
} onvif_PTZStatusFilterOptions;

typedef struct 
{
	onvif_PTZStatusFilterOptions PTZStatusFilterOptions;	// required, This message contains the metadata configuration options. If a metadata configuration is specified, 
													//   the options shall concern that particular configuration. If a media profile is specified, the options shall be compatible with that media profile. 
													//   If no tokens are specified, the options shall be considered generic for the device
} onvif_MetadataConfigurationOptions;

typedef struct
{
	uint32	PosFlag		: 1;						// Indicates whether the field Pos is valid
	uint32  Reserved	: 31;
	
	onvif_OSDPosType	Type;						// required, For OSD position type

	onvif_Vector		Pos;						// Optional, when Type is Custom, this field is valid
} onvif_OSDPosConfiguration;


typedef struct
{
	uint32  ColorspaceFlag  : 1;					// Indicates whether the field Colorspace is valid
	uint32	TransparentFlag	: 1;					// Indicates whether the field Transparent is valid
	uint32 	Reserved		: 30;
	
	float	X;										// required, 
	float	Y;										// required, 
	float	Z;										// required, 

	int		Transparent;							// Optional, The value range of "Transparent" could be defined by vendors only should follow this rule: the minimum value means non-transparent and the maximum value maens fully transparent
	char 	Colorspace[256];						// Optional, support the following colorspace
													// 	http://www.onvif.org/ver10/colorspace/YCbCr
													//	http://www.onvif.org/ver10/colorspace/CIELUV
													// 	http://www.onvif.org/ver10/colorspace/CIELAB 
													// 	http://www.onvif.org/ver10/colorspace/HSV
} onvif_OSDColor;

typedef struct
{
	uint32	DateFormatFlag		: 1;				// Indicates whether the field DateFormat is valid
	uint32	TimeFormatFlag		: 1;				// Indicates whether the field TimeFormat is valid
	uint32	FontSizeFlag		: 1;				// Indicates whether the field FontSize is valid
	uint32	FontColorFlag		: 1;				// Indicates whether the field FontColor is valid
	uint32	BackgroundColorFlag	: 1;				// Indicates whether the field BackgroundColor is valid
	uint32	PlainTextFlag		: 1;				// Indicates whether the field PlainText is valid
	uint32	DateEnableFlag		: 1;				// Indicates whether the DateString is visible
	uint32	TimeEnableFlag		: 1;				// Indicates whether the TimeString is visible
	uint32	PlainEnableFlag		: 1;				// Indicates whether the PlainString is visible

	uint32 	Reserved			: 23;
	
	onvif_OSDTextType	Type;						// required, 
	
	char	DateFormat[64];							// Optional, List of supported OSD date formats. This element shall be present when the value of Type field has Date or DateAndTime. The following DateFormat are defined:
													/*
														M/d/yyyy - e.g. 3/6/2013
														MM/dd/yyyy - e.g. 03/06/2013
														dd/MM/yyyy - e.g. 06/03/2013
														yyyy/MM/dd - e.g. 2013/03/06
														yyyy-MM-dd - e.g. 2013-06-03
														dddd, MMMM dd, yyyy - e.g. Wednesday, March 06, 2013
														MMMM dd, yyyy - e.g. March 06, 2013
														dd MMMM, yyyy - e.g. 06 March, 2013
													*/
	char	TimeFormat[64];							// Optional, List of supported OSD time formats. This element shall be present when the value of Type field has Time or DateAndTime. The following TimeFormat are defined:
													/*
														h:mm:ss tt - e.g. 2:14:21 PM
														hh:mm:ss tt - e.g. 02:14:21 PM
														H:mm:ss - e.g. 14:14:21
														HH:mm:ss - e.g. 14:14:21
													*/

	int		FontSize;								// Optional, Font size of the text in pt
	
	onvif_OSDColor	FontColor;						// Optional, Font color of the text
	onvif_OSDColor	BackgroundColor;				// Optional, Background color of the text
	
	char	PlainText[256];							// Optional, The content of text to be displayed	
} onvif_OSDTextConfiguration;

typedef struct
{
	char	ImgPath[256];							// required, The URI of the image which to be displayed
} onvif_OSDImgConfiguration;

typedef struct
{
	uint32	TextStringFlag	: 1;					// Indicates whether the field TextString is valid
	uint32	ImageFlag		: 1;					// Indicates whether the field Image is valid
	uint32	ImageEnableFlag	: 1;					// Indicates whether the Image is visible
	uint32 	Reserved		: 29;
	
	char	token[ONVIF_TOKEN_LEN];					// required, OSD config token
	char	VideoSourceConfigurationToken[ONVIF_TOKEN_LEN];	// required, Reference to the video source configuration

	onvif_OSDType	Type;							// required, Type of OSD
	
	onvif_OSDPosConfiguration	Position;			// required, Position configuration of OSD
	onvif_OSDTextConfiguration 	TextString;			// Optional, Text configuration of OSD. It shall be present when the value of Type field is Text
	onvif_OSDImgConfiguration	Image;				// Optional, Image configuration of OSD. It shall be present when the value of Type field is Image
} onvif_OSDConfiguration;

typedef struct 
{
	uint32 	ImageFlag		: 1;					// Indicates whether the field Image is valid
	uint32 	PlainTextFlag	: 1;					// Indicates whether the field PlainText is valid
	uint32 	DateFlag		: 1;					// Indicates whether the field Date is valid
	uint32 	TimeFlag		: 1;					// Indicates whether the field Time is valid
	uint32 	DateAndTimeFlag	: 1;					// Indicates whether the field DateAndTime is valid
	uint32 	Reserved		: 27;
	
	int 	Total;									// required 
	int 	Image;									// optional
	int 	PlainText;								// optional
	int 	Date;									// optional
	int 	Time;									// optional
	int 	DateAndTime;							// optional
} onvif_MaximumNumberOfOSDs;

typedef struct 
{
	uint32  ColorspaceFlag	: 1;					// Indicates whether the field Colorspace is valid
	uint32  Reserved		: 31;					
	
	float 	X;										// required, 
	float 	Y;										// required, 
	float 	Z;										// required, 
	
	char 	Colorspace[128];						// optional, 
} onvif_Color;

typedef struct
{
	onvif_FloatRange 	X;							// required
	onvif_FloatRange 	Y;							// required
	onvif_FloatRange 	Z;							// required 

	char 	Colorspace[128];						// required
} onvif_ColorspaceRange;

typedef struct 
{
	int				ColorListSize;	
	onvif_Color 	ColorList[10];					// optional, List the supported color

	int						ColorspaceRangeSize;
	onvif_ColorspaceRange	ColorspaceRange[10];	// optional, Define the rang of color supported
} onvif_ColorOptions;

typedef struct 
{
	uint32  ColorFlag		: 1;					// Indicates whether the field Color is valid
	uint32  TransparentFlag	: 1;					// Indicates whether the field Transparent is valid
	uint32  Reserved		: 30;
	
	onvif_ColorOptions 	Color;						// optional, Optional list of supported colors 
	onvif_IntRange 		Transparent;				// optional, Range of the transparent level. Larger means more tranparent
} onvif_OSDColorOptions;

typedef struct 
{
	uint32	OSDTextType_Plain		: 1;			// Indicates whether support OSD text type plain
	uint32  OSDTextType_Date		: 1;			// Indicates whether support OSD text type date
	uint32  OSDTextType_Time		: 1;			// Indicates whether support OSD text type time
	uint32  OSDTextType_DateAndTime	: 1;			// Indicates whether support OSD text type dateandtime
	uint32  FontSizeRangeFlag		: 1;			// Indicates whether the field FontSizeRange is valid
	uint32  FontColorFlag			: 1;			// Indicates whether the field FontColor is valid	
	uint32  BackgroundColorFlag		: 1;			// Indicates whether the field BackgroundColor is valid
	uint32  Reserved				: 25;
	
	onvif_IntRange	FontSizeRange;					// optional, range of the font size value

	int		DateFormatSize;
	char 	DateFormat[10][64];						// optional, List of supported date format

	int		TimeFormatSize;
	char 	TimeFormat[10][64];						// optional, List of supported time format
	
	onvif_OSDColorOptions	FontColor;				// optional, List of supported font color
	onvif_OSDColorOptions 	BackgroundColor;		// optional, List of supported background color
} onvif_OSDTextOptions;

typedef struct 
{
	int 	ImagePathSize;	
	char 	ImagePath[10][256];						// required, List of avaiable uris of image
} onvif_OSDImgOptions;

typedef struct 
{
	uint32	OSDType_Text			: 1;			// Indicates whether support OSD text type
	uint32  OSDType_Image			: 1;			// Indicates whether support OSD image type
	uint32  OSDType_Extended		: 1;			// Indicates whether support OSD extended type
	uint32  OSDPosType_UpperLeft	: 1;			// Indicates whether support OSD position UpperLeft type
	uint32  OSDPosType_UpperRight	: 1;			// Indicates whether support OSD position UpperRight type
	uint32 	OSDPosType_LowerLeft	: 1;			// Indicates whether support OSD position LowerLeft type
	uint32  OSDPosType_LowerRight	: 1;			// Indicates whether support OSD position LowerRight type
	uint32  OSDPosType_Custom		: 1;			// Indicates whether support OSD position Custom type
	uint32  TextOptionFlag			: 1;			// Indicates whether the field TextOption is valid
	uint32	ImageOptionFlag			: 1;			// Indicates whether the field ImageOption is valid
	uint32 	Reserved				: 22;
	
	onvif_MaximumNumberOfOSDs MaximumNumberOfOSDs;	// required, The maximum number of OSD configurations supported for the specificate video source configuration. 
													//   If a device limits the number of instances by OSDType, it should indicate the supported number via the related attribute
	onvif_OSDTextOptions	TextOption;				// optional, Option of the OSD text configuration. This element shall be returned if the device is signaling the support for Text
	onvif_OSDImgOptions 	ImageOption;			// optional, Option of the OSD image configuration. This element shall be returned if the device is signaling the support for Image
} onvif_OSDConfigurationOptions;

typedef struct
{
	float	x;										// required
} onvif_Vector1D;

typedef struct 
{
	uint32	PanTiltFlag	: 1;						// Indicates whether the field PanTilt is valid
	uint32	ZoomFlag	: 1;						// Indicates whether the field Zoom is valid
	uint32 	Reserved	: 30;
	
	onvif_Vector 	PanTilt;						// optional, Pan and tilt position. The x component corresponds to pan and the y component to tilt
	onvif_Vector1D	Zoom;							// optional, A zoom position
} onvif_PTZVector;

typedef struct 
{
	uint32	PanTiltFlag	: 1;						// Indicates whether the field PanTilt is valid
	uint32	ZoomFlag	: 1;						// Indicates whether the field Zoom is valid
	uint32 	Reserved	: 30;
	
	onvif_Vector 	PanTilt;						// optional, Pan and tilt speed. The x component corresponds to pan and the y component to tilt. If omitted in a request, the current (if any) PanTilt movement should not be affected
	onvif_Vector1D	Zoom;							// optional, A zoom speed. If omitted in a request, the current (if any) Zoom movement should not be affected
} onvif_PTZSpeed;

typedef struct 
{
	uint32	PTZPositionFlag	: 1;					// Indicates whether the field PTZPosition is valid
	uint32 	Reserved		: 31;
	
	char 	Name[ONVIF_NAME_LEN];					// required, A list of preset position name
	char 	token[ONVIF_TOKEN_LEN];					// required

	onvif_PTZVector	PTZPosition;					// optional, A list of preset position
} onvif_PTZPreset;

typedef struct 
{
	onvif_FloatRange 	XRange;						// required 
	onvif_FloatRange	YRange;						// required 
} onvif_PanTiltLimits;

typedef struct 
{
	onvif_FloatRange 	XRange;						// required 
} onvif_ZoomLimits;

typedef struct onvif_PTControlDirection
{
	uint32	EFlipFlag	: 1;						// Indicates whether the field EFlip is valid
	uint32	ReverseFlag	: 1;						// Indicates whether the field Reverse is valid
	uint32 	Reserved	: 30;
	
	onvif_EFlipMode		EFlip;						// optional, Optional element to configure related parameters for E-Flip
	onvif_ReverseMode	Reverse;					// optional, Optional element to configure related parameters for reversing of PT Control Direction
} onvif_PTControlDirection;

typedef struct 
{
	uint32	PTControlDirectionFlag	: 1;			// Indicates whether the field PTControlDirection is valid
	uint32 	Reserved				: 31;
	
	onvif_PTControlDirection	PTControlDirection;	// optional, Optional element to configure PT Control Direction related features
} onvif_PTZConfigurationExtension;

typedef struct
{
	uint32	DefaultPTZSpeedFlag		: 1;			// Indicates whether the field DefaultPTZSpeed is valid
	uint32	DefaultPTZTimeoutFlag	: 1;			// Indicates whether the field DefaultPTZTimeout is valid
	uint32	PanTiltLimitsFlag		: 1;			// Indicates whether the field PanTiltLimits is valid
	uint32	ZoomLimitsFlag			: 1;			// Indicates whether the field ZoomLimits is valid
	uint32	ExtensionFlag			: 1;			// Indicates whether the field Extension is valid
	uint32 	Reserved				: 27;
	
	char 	Name[ONVIF_NAME_LEN];					// required 
	int 	UseCount;								// required 
	char 	token[ONVIF_TOKEN_LEN];					// required 
	char 	NodeToken[ONVIF_TOKEN_LEN];				// required, A mandatory reference to the PTZ Node that the PTZ Configuration belongs to
	
	onvif_PTZSpeed		DefaultPTZSpeed;			// optional, If the PTZ Node supports absolute or relative PTZ movements, it shall specify corresponding default Pan/Tilt and Zoom speeds
	int					DefaultPTZTimeout;			// optional, If the PTZ Node supports continuous movements, it shall specify a default timeout, after which the movement stops 
	onvif_PanTiltLimits	PanTiltLimits;				// optional, The Pan/Tilt limits element should be present for a PTZ Node that supports an absolute Pan/Tilt. If the element is present it signals the support for configurable Pan/Tilt limits. 
													//	If limits are enabled, the Pan/Tilt movements shall always stay within the specified range. The Pan/Tilt limits are disabled by setting the limits to ¨CINF or +INF 
	onvif_ZoomLimits 	ZoomLimits;					// optional, The Zoom limits element should be present for a PTZ Node that supports absolute zoom. If the element is present it signals the supports for configurable Zoom limits. 
													//	If limits are enabled the zoom movements shall always stay within the specified range. The Zoom limits are disabled by settings the limits to -INF and +INF
	
	onvif_PTZConfigurationExtension	Extension;		// optional 
} onvif_PTZConfiguration;

typedef struct 
{
	// Indicates which preset tour operations are available for this PTZ Node
	
	uint32	PTZPresetTourOperation_Start	: 1;
	uint32	PTZPresetTourOperation_Stop		: 1;
	uint32	PTZPresetTourOperation_Pause	: 1;
	uint32	PTZPresetTourOperation_Extended	: 1;
	uint32	Reserved						: 28;	
	
	int 	MaximumNumberOfPresetTours;				// required, Indicates number of preset tours that can be created. Required preset tour operations shall be available for this PTZ Node if one or more preset tour is supported
} onvif_PTZPresetTourSupported;

typedef struct 
{
	uint32	SupportedPresetTourFlag	: 1;			// Indicates whether the field SupportedPresetTour is valid
	uint32 	Reserved				: 31;
	
	onvif_PTZPresetTourSupported	SupportedPresetTour;	// optional, Detail of supported Preset Tour feature
} onvif_PTZNodeExtension;

typedef struct 
{
	onvif_FloatRange 	XRange;						// required 
	onvif_FloatRange	YRange;						// required 
} onvif_Space2DDescription;

typedef struct 
{
	onvif_FloatRange 	XRange;						// required 
} onvif_Space1DDescription;

typedef struct 
{
    uint32	AbsolutePanTiltPositionSpaceFlag	: 1;			// Indicates whether the field AbsolutePanTiltPositionSpace is valid
    uint32	AbsoluteZoomPositionSpaceFlag	    : 1;			// Indicates whether the field AbsoluteZoomPositionSpace is valid
    uint32	RelativePanTiltTranslationSpaceFlag	: 1;			// Indicates whether the field RelativePanTiltTranslationSpace is valid
    uint32	RelativeZoomTranslationSpaceFlag	: 1;			// Indicates whether the field RelativeZoomTranslationSpace is valid
    uint32	ContinuousPanTiltVelocitySpaceFlag	: 1;			// Indicates whether the field ContinuousPanTiltVelocitySpace is valid
    uint32	ContinuousZoomVelocitySpaceFlag	    : 1;			// Indicates whether the field ContinuousZoomVelocitySpace is valid
    uint32	PanTiltSpeedSpaceFlag	            : 1;			// Indicates whether the field PanTiltSpeedSpace is valid
    uint32	ZoomSpeedSpaceFlag	                : 1;			// Indicates whether the field ZoomSpeedSpace is valid
	uint32 	Reserved				            : 24;
	
	onvif_Space2DDescription	AbsolutePanTiltPositionSpace;	// optional, The Generic Pan/Tilt Position space is provided by every PTZ node that supports absolute Pan/Tilt, since it does not relate to a specific physical range. 
																//	Instead, the range should be defined as the full range of the PTZ unit normalized to the range -1 to 1 resulting in the following space description
	onvif_Space1DDescription	AbsoluteZoomPositionSpace;		// optional, The Generic Zoom Position Space is provided by every PTZ node that supports absolute Zoom, since it does not relate to a specific physical range. 
																//	Instead, the range should be defined as the full range of the Zoom normalized to the range 0 (wide) to 1 (tele). 
																//	There is no assumption about how the generic zoom range is mapped to magnification, FOV or other physical zoom dimension
	onvif_Space2DDescription	RelativePanTiltTranslationSpace;// optional, The Generic Pan/Tilt translation space is provided by every PTZ node that supports relative Pan/Tilt, since it does not relate to a specific physical range. 
																//	Instead, the range should be defined as the full positive and negative translation range of the PTZ unit normalized to the range -1 to 1, 
																//	where positive translation would mean clockwise rotation or movement in right/up direction resulting in the following space description 
	onvif_Space1DDescription	RelativeZoomTranslationSpace;	// optional, The Generic Zoom Translation Space is provided by every PTZ node that supports relative Zoom, since it does not relate to a specific physical range. 
																//	Instead, the corresponding absolute range should be defined as the full positive and negative translation range of the Zoom normalized to the range -1 to1, 
																//	where a positive translation maps to a movement in TELE direction. The translation is signed to indicate direction (negative is to wide, positive is to tele). 
																//	There is no assumption about how the generic zoom range is mapped to magnification, FOV or other physical zoom dimension. This results in the following space description
	onvif_Space2DDescription	ContinuousPanTiltVelocitySpace;	// optional, The generic Pan/Tilt velocity space shall be provided by every PTZ node, since it does not relate to a specific physical range. 
																//	Instead, the range should be defined as a range of the PTZ unit¡¯s speed normalized to the range -1 to 1, where a positive velocity would map to clockwise 
																//	rotation or movement in the right/up direction. A signed speed can be independently specified for the pan and tilt component resulting in the following space description 
	onvif_Space1DDescription	ContinuousZoomVelocitySpace;	// optional, The generic zoom velocity space specifies a zoom factor velocity without knowing the underlying physical model. The range should be normalized from -1 to 1, 
																//	where a positive velocity would map to TELE direction. A generic zoom velocity space description resembles the following
	onvif_Space1DDescription	PanTiltSpeedSpace;				// optional, The speed space specifies the speed for a Pan/Tilt movement when moving to an absolute position or to a relative translation. 
																//	In contrast to the velocity spaces, speed spaces do not contain any directional information. The speed of a combined Pan/Tilt 
																//	movement is represented by a single non-negative scalar value
	onvif_Space1DDescription	ZoomSpeedSpace;					// optional, The speed space specifies the speed for a Zoom movement when moving to an absolute position or to a relative translation. 
																//	In contrast to the velocity spaces, speed spaces do not contain any directional information 
} onvif_PTZSpaces;

typedef struct 
{
	uint32  NameFlag				: 1;			// Indicates whether the field Name is valid
	uint32	ExtensionFlag			: 1;			// Indicates whether the field Extension is valid
	uint32	FixedHomePositionFlag	: 1;			// Indicates whether the field FixedHomePosition is valid
	uint32	Reserved				: 29;	
	
	char 	token[ONVIF_TOKEN_LEN];					// required 
	char 	Name[ONVIF_NAME_LEN];					// optional, A unique identifier that is used to reference PTZ Nodes
	
	onvif_PTZSpaces 		SupportedPTZSpaces;		// required, A list of Coordinate Systems available for the PTZ Node. For each Coordinate System, the PTZ Node MUST specify its allowed range
	
	int 	MaximumNumberOfPresets;					// required, All preset operations MUST be available for this PTZ Node if one preset is supported 
	BOOL 	HomeSupported;							// required, A boolean operator specifying the availability of a home position. If set to true, the Home Position Operations MUST be available for this PTZ Node 
	
	onvif_PTZNodeExtension 	Extension;				// optional 
	
	BOOL 	FixedHomePosition;						// optional, Indication whether the HomePosition of a Node is fixed or it can be changed via the SetHomePosition command
} onvif_PTZNode;


typedef struct 
{
	// Supported options for EFlip feature	
    uint32  EFlipMode_OFF           : 1;
	uint32  EFlipMode_ON            : 1;
	uint32  EFlipMode_Extended      : 1;

	// Supported options for Reverse feature
	uint32  ReverseMode_OFF         : 1;
	uint32  ReverseMode_ON          : 1;
	uint32  ReverseMode_AUTO        : 1;
	uint32  ReverseMode_Extended    : 1;
	uint32  Reserved                : 25;
} onvif_PTControlDirectionOptions;

typedef struct 
{
    uint32  PTControlDirectionFlag  : 1;		    // Indicates whether the field PTControlDirection is valid
	uint32	Reserved				: 31;
	
	onvif_IntRange  PTZTimeout;	               		// required, A timeout Range within which Timeouts are accepted by the PTZ Node
	onvif_PTControlDirectionOptions PTControlDirection;	// optional,  
} onvif_PTZConfigurationOptions;

typedef struct 
{
	uint32  PanTiltFlag	: 1;		    			// Indicates whether the field PanTilt is valid
	uint32  ZoomFlag  	: 1;		    			// Indicates whether the field Zoom is valid
	uint32	Reserved	: 30;
	
	onvif_MoveStatus 	PanTilt;					// optional 
	onvif_MoveStatus 	Zoom;						// optional 
} onvif_PTZMoveStatus;

typedef struct 
{
	uint32  PositionFlag	: 1;		    		// Indicates whether the field Position is valid
	uint32  MoveStatusFlag	: 1;		    		// Indicates whether the field MoveStatus is valid
	uint32  ErrorFlag		: 1;		    		// Indicates whether the field MoveStatus is valid
	uint32	Reserved		: 29;
	
	onvif_PTZVector 	Position;					// optional, Specifies the absolute position of the PTZ unit together with the Space references. The default absolute spaces of the corresponding PTZ configuration MUST be referenced within the Position element
	onvif_PTZMoveStatus MoveStatus;					// optional, Indicates if the Pan/Tilt/Zoom device unit is currently moving, idle or in an unknown state
	
	char 	Error[100];								// optional, States a current PTZ error
	char 	UtcTime[64];							// required, Specifies the UTC time when this status was generated 
} onvif_PTZStatus;

typedef struct 
{
	uint32  NameFlag	: 1;		    			// Indicates whether the field Name is valid
	uint32  MTUFlag		: 1;		    			// Indicates whether the field MTU is valid
	uint32	Reserved	: 30;
	
	char 	Name[ONVIF_NAME_LEN];					// optional, Network interface name, for example eth0
	char 	HwAddress[32];							// required, Network interface MAC address
	int 	MTU;									// optional, Maximum transmission unit
} onvif_NetworkInterfaceInfo;

typedef struct 
{
	char 	Address[32];							// required 
	int 	PrefixLength;							// required 
	
	BOOL 	DHCP;									// required, Indicates whether or not DHCP is used
} onvif_IPv4Configuration;

typedef struct 
{
	BOOL 	Enabled;								// required, Indicates whether or not IPv4 is enabled
	
	onvif_IPv4Configuration	Config;					// required, IPv4 configuration
} onvif_IPv4NetworkInterface;

typedef struct 
{
	uint32  InfoFlag	: 1;		    			// Indicates whether the field Info is valid
	uint32  IPv4Flag	: 1;		    			// Indicates whether the field IPv4 is valid
	uint32	Reserved	: 30;
	
	char 	token[ONVIF_TOKEN_LEN];					// required 
	BOOL 	Enabled;								// required, Indicates whether or not an interface is enabled
	
	onvif_NetworkInterfaceInfo	Info;				// optional, Network interface information
	onvif_IPv4NetworkInterface 	IPv4;				// optional, IPv4 network interface configuration 
} onvif_NetworkInterface;

typedef struct
{
	BOOL 	HTTPFlag;								// Indicates if the http protocol required
	BOOL 	HTTPEnabled;							// Indicates if the http protocol is enabled or not
	BOOL 	HTTPSFlag;								// Indicates if the https protocol required
	BOOL 	HTTPSEnabled;							// Indicates if the https protocol is enabled or not
	BOOL 	RTSPFlag;								// Indicates if the rtsp protocol required
	BOOL 	RTSPEnabled;							// Indicates if the rtsp protocol is enabled or not

	int  	HTTPPort[MAX_SERVER_PORT];				// The port that is used by the protocol
	int  	HTTPSPort[MAX_SERVER_PORT];				// The port that is used by the protocol
	int  	RTSPPort[MAX_SERVER_PORT];				// The port that is used by the protocol
} onvif_NetworkProtocol;

typedef struct 
{
	uint32  SearchDomainFlag	: 1;		    	// Indicates whether the field Name is valid
	uint32	Reserved			: 31;
	
	BOOL 	FromDHCP;								// required, Indicates whether or not DNS information is retrieved from DHCP 
	char 	SearchDomain[MAX_SEARCHDOMAIN][64];		// optional, Search domain
	char	DNSServer[MAX_DNS_SERVER][32];			// required
} onvif_DNSInformation;

typedef struct 
{
	BOOL 	FromDHCP;								// required, Indicates if NTP information is to be retrieved by using DHCP
	char	NTPServer[MAX_NTP_SERVER][32];			// required
} onvif_NTPInformation;

typedef struct 
{
	uint32  NameFlag	: 1;		    			// Indicates whether the field Name is valid
	uint32	Reserved	: 31;
	
	BOOL	FromDHCP;								// required, Indicates whether the hostname is obtained from DHCP or not
	BOOL	RebootNeeded;							// required, Indicates whether or not a reboot is required after configuration updates
	char 	Name[100];								// optional, Indicates the hostname
} onvif_HostnameInformation;

typedef struct 
{
	char	IPv4Address[MAX_GATEWAY][32];			// required, IPv4 address string	
} onvif_NetworkGateway;

typedef struct 
{
	char 	TZ[32];									// required, Posix timezone string
} onvif_TimeZone;

typedef struct
{
	int		Hour;									// Range is 0 to 23
	int		Minute;									// Range is 0 to 59
	int		Second;									// Range is 0 to 61 (typically 59)
} onvif_Time;

typedef struct
{
	int		Year;									// 
	int		Month;									// Range is 1 to 12
	int		Day;									// Range is 1 to 31
} onvif_Date;

typedef struct
{
	onvif_Time	Time;								// required 
	onvif_Date  Date;								// required 
} onvif_DateTime;

typedef struct 
{
	uint32  TimeZoneFlag	: 1;		    		// Indicates whether the field TimeZone is valid
	uint32	Reserved		: 31;
	
	BOOL 	DaylightSavings;						// required, Informative indicator whether daylight savings is currently on/off
	
	onvif_SetDateTimeType 	DateTimeType;			// required, Indicates if the time is set manully or through NTP	
	onvif_TimeZone			TimeZone;				// optional, Timezone information in Posix format 
} onvif_SystemDateTime;

typedef struct 
{
	onvif_TransportProtocol Protocol;				// required, Defines the network protocol for streaming, either UDP=RTP/UDP, RTSP=RTP/RTSP/TCP or HTTP=RTP/RTSP/HTTP/TCP 
} onvif_Transport;

typedef struct 
{
	onvif_StreamType	Stream;						// required, Defines if a multicast or unicast stream is requested
	onvif_Transport 	Transport;					// required 
} onvif_StreamSetup;

typedef struct
{
	char 	SourceId[128];							// required, Identifier for the source chosen by the client that creates the structure.
													//	This identifier is opaque to the device. Clients may use any type of URI for this field. A device shall support at least 128 characters
	char 	Name[64];								// required, Informative user readable name of the source, e.g. "Camera23". A device shall support at least 20 characters
	char 	Location[100];							// required, Informative description of the physical location of the source, e.g. the coordinates on a map
	char 	Description[128];						// required, Informative description of the source
	char 	Address[128];							// required, URI provided by the service supplying data to be recorded. A device shall support at least 128 characters
} onvif_RecordingSourceInformation;

typedef struct
{
	uint32  MaximumRetentionTimeFlag : 1;			// Indicates whether the field MaximumRetentionTime is valid
	uint32  Reserved 				 : 31;
	
	onvif_RecordingSourceInformation	Source;		// required, Information about the source of the recording
	char 	Content[256];							// required, Informative description of the source
	uint32 	MaximumRetentionTime;					// optional, specifies the maximum time that data in any track within the
													//	recording shall be stored. The device shall delete any data older than the maximum retention
													//	time. Such data shall not be accessible anymore. If the MaximumRetentionPeriod is set to 0,
													//	the device shall not limit the retention time of stored data, except by resource constraints.
													//	Whatever the value of MaximumRetentionTime, the device may automatically delete
													//	recordings to free up storage space for new recordings.
} onvif_RecordingConfiguration;

typedef struct 
{
	onvif_TrackType	TrackType;						// required, Type of the track. It shall be equal to the strings "Video", "Audio" or "Metadata"
	
	char 	Description[100];						// required, Informative description of the track
} onvif_TrackConfiguration;

typedef struct 
{
	uint32  TypeFlag : 1;							// Indicates whether the field Type is valid
	uint32  Reserved : 31;
	
	char 	Token[ONVIF_TOKEN_LEN];					// required,
	char 	Type[256];								// optional, default is "http://www.onvif.org/ver10/schema/Receiver", "http://www.onvif.org/ver10/schema/Profile"
} onvif_SourceReference;

typedef struct 
{
	char 	SourceTag[64];							// required, If the received RTSP stream contains multiple tracks of the same type, the
													//	SourceTag differentiates between those Tracks. This field can be ignored in case of recording a local source
	char 	Destination[ONVIF_TOKEN_LEN];			// required, The destination is the tracktoken of the track to which the device shall store the received data
} onvif_RecordingJobTrack;

typedef struct 
{
	uint32  SourceTokenFlag 		: 1;			// Indicates whether the field SourceToken is valid
	uint32  AutoCreateReceiverFlag 	: 1;			// Indicates whether the field AutoCreateReceiver is valid
	uint32  Reserved 				: 30;
	
	onvif_SourceReference	SourceToken;			// optional, This field shall be a reference to the source of the data. The type of the source
													//	is determined by the attribute Type in the SourceToken structure. If Type is
													//	http://www.onvif.org/ver10/schema/Receiver, the token is a ReceiverReference. In this case
													//	the device shall receive the data over the network. If Type is
													//	http://www.onvif.org/ver10/schema/Profile, the token identifies a media profile, instructing the
													//	device to obtain data from a profile that exists on the local device
	BOOL 	AutoCreateReceiver;						// optional, If this field is TRUE, and if the SourceToken is omitted, the device
													//	shall create a receiver object (through the receiver service) and assign the
													//	ReceiverReference to the SourceToken field. When retrieving the RecordingJobConfiguration
													//	from the device, the AutoCreateReceiver field shall never be present

	int 	sizeTracks;
	
	onvif_RecordingJobTrack	Tracks[5];				// optional, List of tracks associated with the recording
} onvif_RecordingJobSource;

typedef struct 
{
	char 	RecordingToken[ONVIF_TOKEN_LEN];		// required, Identifies the recording to which this job shall store the received data
	char 	Mode[16];								// required, The mode of the job. If it is idle, nothing shall happen. If it is active, the device shall try to obtain data from the receivers. 
													//	A client shall use GetRecordingJobState to determine if data transfer is really taking place
													//   The only valid values for Mode shall be "Idle" or "Active"
	int 	Priority;								// required, This shall be a non-negative number. If there are multiple recording jobs that store data to
													//	the same track, the device will only store the data for the recording job with the highest
													//	priority. The priority is specified per recording job, but the device shall determine the priority
													//	of each track individually. If there are two recording jobs with the same priority, the device
													//	shall record the data corresponding to the recording job that was activated the latest
	int 	sizeSource;
	
	onvif_RecordingJobSource	Source[5];			// optional, Source of the recording
} onvif_RecordingJobConfiguration;

typedef struct 
{
	uint32  ErrorFlag	: 1;						// Indicates whether the field Error is valid
	uint32  Reserved	: 31;
	
	char 	SourceTag[64];							// required, Identifies the track of the data source that provides the data
	char 	Destination[ONVIF_TOKEN_LEN];			// required, Indicates the destination track
	char 	Error[100];								// optional, Optionally holds an implementation defined string value that describes the error. The string should be in the English language
	char 	State[16];								// required, Provides the job state of the track. 
													//	The valid values of state shall be "Idle", "Active" and "Error". If state equals "Error", the Error field may be filled in with an implementation defined value
} onvif_RecordingJobStateTrack;

typedef struct 
{
	onvif_SourceReference	SourceToken;			// required, Identifies the data source of the recording job
	char 	State[16];								// required, Holds the aggregated state over all substructures of RecordingJobStateSource
													// 	Idle : All state values in sub-nodes are "Idle"
													// 	PartiallyActive : The state of some sub-nodes are "active" and some sub-nodes are "idle"
													//	Active : The state of all sub-nodes is "Active"
													//	Error : At least one of the sub-nodes has state "Error"
	int 	sizeTrack;
	
	onvif_RecordingJobStateTrack	Track[5];		// optional, 
} onvif_RecordingJobStateSource;

typedef struct 
{
	char 	RecordingToken[ONVIF_TOKEN_LEN];		// required, Identification of the recording that the recording job records to
	char 	State[16];								// required, Holds the aggregated state over the whole RecordingJobInformation structure
													// 	Idle : All state values in sub-nodes are "Idle"
													// 	PartiallyActive : The state of some sub-nodes are "active" and some sub-nodes are "idle"
													//	Active : The state of all sub-nodes is "Active"
													//	Error : At least one of the sub-nodes has state "Error"
	
	int 	sizeSources;
	
	onvif_RecordingJobStateSource	Sources[5];		// optional, Identifies the data source of the recording job
} onvif_RecordingJobStateInformation;

typedef struct
{
	uint32  SpareFlag				: 1;			// Indicates whether the field Spare is valid
	uint32  CompatibleSourcesFlag	: 1;			// Indicates whether the field CompatibleSources is valid
	uint32  Reserved				: 30;
	
	int 	Spare;									// optional, Number of spare jobs that can be created for the recording
	char 	CompatibleSources[160];					// optional, A device that supports recording of a restricted set of Media Service Profiles returns the list of profiles that can be recorded on the given Recording
} onvif_JobOptions;

typedef struct
{
	uint32  SpareTotalFlag		: 1;				// Indicates whether the field SpareTotal is valid
	uint32  SpareVideoFlag		: 1;				// Indicates whether the field SpareVideo is valid
	uint32  SpareAudioFlag		: 1;				// Indicates whether the field SpareAudio is valid
	uint32  SpareMetadataFlag	: 1;				// Indicates whether the field SpareMetadata is valid
	uint32  Reserved			: 28;
	
	int 	SpareTotal;								// optional, Total spare number of tracks that can be added to this recording
	int 	SpareVideo;								// optional, Number of spare Video tracks that can be added to this recording
	int 	SpareAudio;								// optional, Number of spare Aduio tracks that can be added to this recording
	int 	SpareMetadata;							// optional, Number of spare Metadata tracks that can be added to this recording
} onvif_TrackOptions;

typedef struct
{
	onvif_JobOptions	Job;						// required, 
	onvif_TrackOptions	Track;						// required, 
} onvif_RecordingOptions;

typedef struct
{
	char 	TrackToken[ONVIF_TOKEN_LEN];			// required
	
	onvif_TrackConfiguration 	Configuration;		// required
} onvif_Track;

typedef struct _ONVIF_TRACK
{
	struct _ONVIF_TRACK * next;

	onvif_Track	Track;
} ONVIF_TRACK;

typedef struct 
{
	char 	RecordingToken[ONVIF_TOKEN_LEN];		// required
	
	onvif_RecordingConfiguration 	Configuration;	// required
	
	ONVIF_TRACK	* Tracks;
} onvif_Recording;

typedef struct
{
	char 	JobToken[ONVIF_TOKEN_LEN];				// required
	
	onvif_RecordingJobConfiguration	JobConfiguration;	// required
} onvif_RecordingJob;

typedef struct 
{
	char 	Name[32];									// required, Item name
	char 	Value[ONVIF_TOKEN_LEN];						// required, Item value. The type is defined in the corresponding description
} onvif_SimpleItem;

typedef struct _ONVIF_SIMPLEITEM
{
	struct _ONVIF_SIMPLEITEM * next;

	onvif_SimpleItem SimpleItem;						// Value name pair as defined by the corresponding description
} ONVIF_SIMPLEITEM;

typedef struct
{
	char 	Name[32];									// required, Item name
	char  * Any;                                        // optional
} onvif_ElementItem;

typedef struct _ONVIF_ELEMENTITEM
{
	struct _ONVIF_ELEMENTITEM * next;

	onvif_ElementItem ElementItem;						// Value name pair as defined by the corresponding description
} ONVIF_ELEMENTITEM;

typedef struct 
{
	ONVIF_SIMPLEITEM	* SimpleItem;					// optional
	ONVIF_ELEMENTITEM	* ElementItem;					// optional
} onvif_ItemList;

typedef struct
{
	uint32  PropertyOperationFlag	: 1;				// Indicates whether the field PropertyOperation is valid
	uint32  SourceFlag				: 1;				// Indicates whether the field Source is valid
	uint32  KeyFlag					: 1;				// Indicates whether the field Key is valid
	uint32  DataFlag				: 1;				// Indicates whether the field Data is valid
	uint32  Reserved				: 28;
	
	time_t UtcTime;										// required
	
	onvif_ItemList	Source;								// optional, Token value pairs that triggered this message. Typically only one item is present
	onvif_ItemList	Key;								// optional element of type tt:ItemList */
	onvif_ItemList	Data;								// optional element of type tt:ItemList */
	
	onvif_PropertyOperation	PropertyOperation;			// optional 
} onvif_Message;

typedef struct
{
	char	ConsumerAddress[256];						// required, 
	char 	ProducterAddress[256];						// required, 

	char	Dialect[256];								// required, 
	char	Topic[256];									// required, 

	onvif_Message	Message;							// required
} onvif_NotificationMessage;

typedef struct 
{
	time_t 	DataFrom;									// required, The earliest point in time where there is recorded data on the device
	time_t 	DataUntil;									// required, The most recent point in time where there is recorded data on the device
	int 	NumberRecordings;							// required, The device contains this many recordings
} onvif_RecordingSummary;

typedef struct 
{
	char 	TrackToken[ONVIF_TOKEN_LEN];				// required, 
	
	onvif_TrackType TrackType;							// required, Type of the track: "Video", "Audio" or "Metadata".
														//	The track shall only be able to hold data of that type
	
	char 	Description[100];							// required, Informative description of the contents of the track
	time_t 	DataFrom;									// required, The start date and time of the oldest recorded data in the track
	time_t 	DataTo;										// required, The stop date and time of the newest recorded data in the track
} onvif_TrackInformation;

typedef struct 
{
	uint32  EarliestRecordingFlag	: 1;				// Indicates whether the field EarliestRecording is valid
	uint32  LatestRecordingFlag		: 1;				// Indicates whether the field LatestRecording is valid
	uint32  Reserved				: 30;
	
	char 	RecordingToken[ONVIF_TOKEN_LEN];			// required, 
	
	onvif_RecordingSourceInformation	Source;			// required, Information about the source of the recording
	
	time_t	EarliestRecording;							// optional, 
	time_t	LatestRecording;							// optional, 
	char	Content[256];								// required, 

	int 	sizeTrack;	
	onvif_TrackInformation	Track[5];					// optional, Basic information about the track. Note that a track may represent a single contiguous time span or consist of multiple slices
	
	onvif_RecordingStatus	RecordingStatus;			// required, 
} onvif_RecordingInformation;

typedef struct 
{
	uint32  BitrateFlag	: 1;							// Indicates whether the field Bitrate is valid
	uint32  Reserved	: 31;
	
	int 	Bitrate;									// optional, Average bitrate in kbps
	int 	Width;										// required, The width of the video in pixels
	int 	Height;										// required, The height of the video in pixels

	onvif_VideoEncoding Encoding;						// required, Used video codec, either Jpeg, H.264 or Mpeg4

	float 	Framerate;									// required, Average framerate in frames per second
} onvif_VideoAttributes;

typedef struct 
{
	uint32  BitrateFlag	: 1;							// Indicates whether the field Bitrate is valid
	uint32  Reserved	: 31;
	
	int 	Bitrate;									// optional, The bitrate in kbps

	onvif_AudioEncoding Encoding;						// required, Audio codec used for encoding the audio (either G.711, G.726 or AAC)

	int 	Samplerate;									// required, The sample rate in kHz
} onvif_AudioAttributes;

typedef struct 
{
	uint32  PtzSpacesFlag	: 1;						// Indicates whether the field PtzSpaces is valid
	uint32  Reserved		: 31;
	
	BOOL 	CanContainPTZ;								// required, Indicates that there can be PTZ data in the metadata track in the specified time interval
	BOOL 	CanContainAnalytics;						// required, Indicates that there can be analytics data in the metadata track in the specified time interval
	BOOL 	CanContainNotifications;					// required, Indicates that there can be notifications in the metadata track in the specified time interval
	char 	PtzSpaces[256];								// optional, List of all PTZ spaces active for recording. Note that events are only recorded on position changes and 
														//	the actual point of recording may not necessarily contain an event of the specified type
} onvif_MetadataAttributes;

typedef struct 
{
	uint32  VideoAttributesFlag		: 1;				// Indicates whether the field VideoAttributes is valid
	uint32  AudioAttributesFlag		: 1;				// Indicates whether the field AudioAttributes is valid
	uint32  MetadataAttributesFlag	: 1;				// Indicates whether the field MetadataAttributes is valid
	uint32  Reserved				: 29;
	
	onvif_TrackInformation		TrackInformation;		// required, The basic information about the track. Note that a track may represent a single contiguous time span or consist of multiple slices
	onvif_VideoAttributes		VideoAttributes;		// optional, If the track is a video track, exactly one of this structure shall be present and contain the video attributes
	onvif_AudioAttributes		AudioAttributes;		// optional, If the track is an audio track, exactly one of this structure shall be present and contain the audio attributes
	onvif_MetadataAttributes	MetadataAttributes;		// optional, If the track is an metadata track, exactly one of this structure shall be present and contain the metadata attributes
} onvif_TrackAttributes;

typedef struct 
{
	char 	RecordingToken[ONVIF_TOKEN_LEN];			// required, A reference to the recording that has these attributes
	
	int 	sizeTrackAttributes;
	onvif_TrackAttributes	TrackAttributes[5];			// optional, A set of attributes for each track

	time_t 	From;										// required, The attributes are valid from this point in time in the recording
	time_t 	Until;										// required, The attributes are valid until this point in time in the recording. 
														//	Can be equal to 'From' to indicate that the attributes are only known to be valid for this particular point in time
} onvif_MediaAttributes;

typedef struct 
{
	uint32  RecordingInformationFilterFlag	: 1;		// Indicates whether the field RecordingInformationFilter is valid
	uint32  Reserved						: 31;
	
	int 	sizeIncludedSources;
	onvif_SourceReference	IncludedSources[10];		// optional, A list of sources that are included in the scope. If this list is included, only data from one of these sources shall be searched
	
	int 	sizeIncludedRecordings;
	char 	IncludedRecordings[10][ONVIF_TOKEN_LEN];	// optional, A list of recordings that are included in the scope. If this list is included, only data from one of these recordings shall be searched

	char 	RecordingInformationFilter[128];			// optional, An xpath expression used to specify what recordings to search. 
														//	Only those recordings with an RecordingInformation structure that matches the filter shall be searched
} onvif_SearchScope;

typedef struct _ONVIF_RECORDINGINFORMATION
{
	struct _ONVIF_RECORDINGINFORMATION * next;

	onvif_RecordingInformation	RecordingInformation;
} ONVIF_RECINF;

typedef struct 
{
	onvif_SearchState SearchState;						// required, The state of the search when the result is returned. Indicates if there can be more results, or if the search is completed
	
	ONVIF_RECINF * RecordInformation;					// optional, A RecordingInformation structure for each found recording matching the search
} onvif_FindRecordingResultList;

typedef struct 
{
	char 	RecordingToken[ONVIF_TOKEN_LEN];			// required, The recording where this event was found. Empty string if no recording is associated with this event
	char 	TrackToken[ONVIF_TOKEN_LEN];				// required, A reference to the track where this event was found. Empty string if no track is associated with this event
	time_t 	Time;										// required, The time when the event occured

	onvif_NotificationMessage 	Event;					// required, The description of the event

	BOOL 	StartStateEvent;							// required, If true, indicates that the event is a virtual event generated for this particular search session to give the state of a property at the start time of the search
} onvif_FindEventResult;

typedef struct _ONVIF_EVENTRESULT
{
	struct _ONVIF_EVENTRESULT * next;

	onvif_FindEventResult	FindEventResult;
} ONVIF_EVENTRESULT;

typedef struct 
{
	onvif_SearchState SearchState;						// required, The state of the search when the result is returned. Indicates if there can be more results, or if the search is completed

	ONVIF_EVENTRESULT * Result;							// optional
} onvif_FindEventResultList;


//////////////////////////////////////////////////////////////////////////
//	Video analytics struct defines
//////////////////////////////////////////////////////////////////////////

typedef struct 
{
	onvif_ItemList	Parameters;							// required
	
	char 	Name[32];									// required
	char 	Type[32];									// required
} onvif_Config;

typedef struct _ONVIF_CONFIG
{
	struct _ONVIF_CONFIG * next;

	onvif_Config	Config;
} ONVIF_CONFIG;

typedef struct 
{
	ONVIF_CONFIG * AnalyticsModule;						// optional
} onvif_AnalyticsEngineConfiguration;

typedef struct 
{
	ONVIF_CONFIG * Rule;								// optional
} onvif_RuleEngineConfiguration;

typedef struct 
{
	char 	Name[ONVIF_NAME_LEN];						// required
	int 	UseCount;									// required
	char 	token[ONVIF_TOKEN_LEN];						// required

	onvif_AnalyticsEngineConfiguration 	AnalyticsEngineConfiguration;	// required
	onvif_RuleEngineConfiguration 		RuleEngineConfiguration;		// required 
} onvif_VideoAnalyticsConfiguration;

typedef struct
{
	char 	Name[64];									// required
	char 	Type[64];									// required
} onvif_SimpleItemDescription;

typedef struct _ONVIF_SIMPLE_ITEM_DESC
{
	struct _ONVIF_SIMPLE_ITEM_DESC * next;

	onvif_SimpleItemDescription	SimpleItemDescription;
} ONVIF_SIMPLE_ITEM_DESC;

typedef struct 
{
	ONVIF_SIMPLE_ITEM_DESC	* SimpleItemDescription;
	ONVIF_SIMPLE_ITEM_DESC	* ElementItemDescription;
} onvif_ItemListDescription;

typedef struct 
{
	uint32  SourceFlag		: 1;						// Indicates whether the field Source is valid
	uint32  KeyFlag			: 1;						// Indicates whether the field Source is valid
	uint32  DataFlag		: 1;						// Indicates whether the field Source is valid
	uint32  IsPropertyFlag	: 1;						// Indicates whether the field Source is valid
	uint32  Reserved		: 28;
	
	onvif_ItemListDescription	Source;					// optional 
	onvif_ItemListDescription	Key;					// optional 
	onvif_ItemListDescription	Data;					// optional 

	BOOL	IsProperty;									// optional 
	char	ParentTopic[100];							// required 
} onvif_ConfigDescription_Messages;

typedef struct _ONVIF_CFG_DESC_MSG
{
	struct _ONVIF_CFG_DESC_MSG * next;

	onvif_ConfigDescription_Messages Messages;
} ONVIF_CFG_DESC_MSG;

typedef struct 
{
	onvif_ItemListDescription 	Parameters;				//required 

	ONVIF_CFG_DESC_MSG	* Messages;		

	char 	Name[ONVIF_NAME_LEN];						// required 
} onvif_ConfigDescription;

typedef struct _ONVIF_CFG_DESC
{
	struct _ONVIF_CFG_DESC * next;

	onvif_ConfigDescription	ConfigDescription;
} ONVIF_CFG_DESC;

typedef struct 
{
	int 	sizeRuleContentSchemaLocation;
	char 	RuleContentSchemaLocation[10][256];			// optional

	ONVIF_CFG_DESC	* RuleDescription;
} onvif_SupportedRules;




#ifdef __cplusplus
extern "C" {
#endif


const char * 				onvif_CapabilityCategoryToString(onvif_CapabilityCategory category);
onvif_CapabilityCategory 	onvif_StringToCapabilityCategory(const char * str);

const char * 				onvif_VideoEncodingToString(onvif_VideoEncoding encoding);
onvif_VideoEncoding 		onvif_StringToVideoEncoding(const char * str);

const char * 				onvif_AudioEncodingToString(onvif_AudioEncoding encoding);
onvif_AudioEncoding 		onvif_StringToAudioEncoding(const char * str);

const char * 				onvif_H264ProfileToString(onvif_H264Profile profile);
onvif_H264Profile 			onvif_StringToH264Profile(const char * str);

const char * 				onvif_Mpeg4ProfileToString(onvif_Mpeg4Profile profile);
onvif_Mpeg4Profile 			onvif_StringToMpeg4Profile(const char * str);

const char * 				onvif_UserLevelToString(onvif_UserLevel level);
onvif_UserLevel 			onvif_StringToUserLevel(const char * str);

const char * 				onvif_MoveStatusToString(onvif_MoveStatus status);
onvif_MoveStatus 			onvif_StringToMoveStatus(const char * str);

const char * 				onvif_OSDTypeToString(onvif_OSDType type);
onvif_OSDType 				onvif_StringToOSDType(const char * type);

const char * 				onvif_OSDPosTypeToString(onvif_OSDPosType type);
onvif_OSDPosType 			onvif_StringToOSDPosType(const char * type);

const char * 				onvif_OSDTextTypeToString(onvif_OSDTextType type);
onvif_OSDTextType 			onvif_StringToOSDTextType(const char * type);

const char * 				onvif_BacklightCompensationModeToString(onvif_BacklightCompensationMode mode);
onvif_BacklightCompensationMode onvif_StringToBacklightCompensationMode(const char * str);

const char * 				onvif_ExposureModeToString(onvif_ExposureMode mode);
onvif_ExposureMode 			onvif_StringToExposureMode(const char * str);

const char * 				onvif_ExposurePriorityToString(onvif_ExposurePriority mode);
onvif_ExposurePriority 		onvif_StringToExposurePriority(const char * str);

const char * 				onvif_AutoFocusModeToString(onvif_AutoFocusMode mode);
onvif_AutoFocusMode 		onvif_StringToAutoFocusMode(const char * str);

const char * 				onvif_WideDynamicModeToString(onvif_WideDynamicMode mode);
onvif_WideDynamicMode 		onvif_StringToWideDynamicMode(const char * str);

const char * 				onvif_IrCutFilterModeToString(onvif_IrCutFilterMode mode);
onvif_IrCutFilterMode 		onvif_StringToIrCutFilterMode(const char * str);

const char * 				onvif_WhiteBalanceModeToString(onvif_WhiteBalanceMode mode);
onvif_WhiteBalanceMode 		onvif_StringToWhiteBalanceMode(const char * str);

const char * 				onvif_EFlipModeToString(onvif_EFlipMode mode);
onvif_EFlipMode 			onvif_StringToEFlipMode(const char * str);

const char * 				onvif_ReverseModeToString(onvif_ReverseMode mode);
onvif_ReverseMode 			onvif_StringToReverseMode(const char * str);

const char *				onvif_DiscoveryModeToString(onvif_DiscoveryMode mode);
onvif_DiscoveryMode			onvif_StringToDiscoveryMode(const char * str);

const char * 				onvif_SetDateTimeTypeToString(onvif_SetDateTimeType type);
onvif_SetDateTimeType		onvif_StringToSetDateTimeType(const char * str);

const char * 				onvif_StreamTypeToString(onvif_StreamType type);
onvif_StreamType			onvif_StringToStreamType(const char * str);

const char * 				onvif_TransportProtocolToString(onvif_TransportProtocol type);
onvif_TransportProtocol		onvif_StringToTransportProtocol(const char * str);

const char * 				onvif_TrackTypeToString(onvif_TrackType type);
onvif_TrackType				onvif_StringToTrackType(const char * str);

const char * 				onvif_PropertyOperationToString(onvif_PropertyOperation type);
onvif_PropertyOperation		onvif_StringToPropertyOperation(const char * str);

const char * 				onvif_RecordingStatusToString(onvif_RecordingStatus status);
onvif_RecordingStatus		onvif_StringToRecordingStatus(const char * str);

const char *				onvif_SearchStateToString(onvif_SearchState state);
onvif_SearchState			onvif_StringToSearchState(const char * str);

#ifdef __cplusplus
}
#endif

#endif	/* end of ONVIF_COMM_H */





