
#include "sys_env_type.h"
#include <iostream>
#include <fstream>
#include <cassert>
#include <stdio.h>

#include <arpa/inet.h>
#include <net/if.h>
#include <time.h>

#include "json/json.h"
#include "json_onvifinfo.h"
#include "string.h"

const std::string VERSION_STR = "version";
const std::string NTP_STR = "ntp";
const std::string HOST_NAME = "hostname";
const std::string NAME_STR = "Name";
const std::string USER_COUNT = "usercount";
const std::string WIDTH_STR = "width";
const std::string HEIGHT_STR = "height";
const std::string B_ENABLE = "bEnable";
const std::string E_RECID = "eRecID";
const std::string N_TRACKNUM = "nTrackNum";
const std::string SOURCE_TOKEN_FLG = "source_token_flg";
const std::string SOURCE_TOKEN_STRUCT = "source_token_struct";
const std::string SOURCE_TOKEN = "source_token";
const std::string EVENT_STR = "event";
const std::string USED_STR = "used";
const std::string VA_CFG = "VACfg";
const std::string STREAM_TYPE = "streamType";
const std::string PORT_STR = "port";
const std::string USER = "user";
const std::string ENABLE_STR = "enable";
const std::string MAIN_STREAM = "main_stream";
const std::string SUB_STREAM = "sub_stream";
const std::string ALARM_STREAM = "alarm_stream";
const std::string HTTP_PORT = "http_port";
const std::string RTSP_PORT = "rtsp_port";
const std::string IP_IN_ADDR = "ip";
const std::string NETMASK_IN_ADDR = "netmask";
const std::string GATEWAY_IN_ADDR = "gateway";
const std::string DNS_IN_ADDR = "dns";
const std::string DHCP_ENABLE = "dhcp_enable";

const std::string ONVIF_INFO = "onvif_info";
const std::string SERVICES = "services";
const std::string DEVICE_INFORMATION = "device_information";
const std::string TIME_CFG = "time_cfg";
const std::string SCOPES = "scopes";
const std::string PROFILES = "Profiles";
const std::string RECORDING_JOBS = "RecordingJobs";
const std::string RECORDINGS = "Recordings";
const std::string SUBSCRIPTIONS = "Subscriptions";
const std::string VIDEO_ANALYTICS_STR = "VideoAnalytics";
const std::string RECEIVERS = "Receivers";
const std::string SERIAL_PORTS = "SerialPorts";
const std::string OSD_CFGS = "OSDCfgs";
const std::string REMOTE_SERVER = "remote_server";

const std::string INFO_STR = "info";
const std::string DEVICE_SERVICE_CAP = "device_service_cap";
const std::string MEDIA_CAP = "media_cap";
const std::string ANALYTIC_CAP = "Analytic_cap";
const std::string EVENT_CAP = "Event_cap";
const std::string EXTENSION_CAP = "Extension_cap";

const std::string SERVICE = "service";
const std::string NAMESPACE = "Namespace";
const std::string X_ADDR = "XAddr";
const std::string MAJOR = "Major";
const std::string MINOR = "Minor";

const std::string NETWORK = "Network";
const std::string SECURITY_STR = "Security";
const std::string SYSTEM_STR = "System";
const std::string MISC_STR = "Misc";

const std::string IP_FILTER = "IPFilter";
const std::string ZERO_CONFIGURATION = "ZeroConfiguration";
const std::string IP_VERSION6 = "IPVersion6";
const std::string DYN_DNS = "DynDNS";
const std::string DOT11_CONFIGURATION = "Dot11Configuration";
const std::string DOT1X_CONFIGURATIONS ="Dot1XConfigurations";
const std::string HOSTNAME_FROMDHCP = "HostnameFromDHCP";
const std::string DHC_PV6 = "DBCPv6";

const std::string TLS1_X0020E0 = "TLS1_X002e0";
const std::string TLS1_X0020E1 = "TLS1_X002e1";
const std::string TLS1_X0020E2 = "TLS1_X002e2";
const std::string ONBOARD_KEYGENERATION = "OnboardKeyGeneration";
const std::string ACCESS_POLICY_CONFIG = "AccessPolicyConfig";
const std::string DEFAULT_ACCESS_POLICY = "DefaultAccessConfig";
const std::string DOT1X = "Dot1X";
const std::string REMOTEUSER_HANDLING = "RemoteUserHandling";
const std::string X_X002E509TOKEN = "X_x002e509Token";
const std::string SAML_TOKEN = "SAMLToken";
const std::string KERBEROS_TOKEN = "KerberosToken";
const std::string USERNAME_TOKEN = "UsernameToken";
const std::string HTTP_DIGEST = "HttpDigest";
const std::string REL_TOKEN = "RELToken";
const std::string SUPPORTED_EAPMETHODS = "SupportedEAPMethods";

const std::string DISCOVERY_RESOLVE = "DiscoveryResolve";
const std::string DISCOVERY_BYE = "DiscoveryBye";
const std::string REMOTE_DISCOVERY = "RemoteDiscovery";
const std::string SYSTEM_BACKUP = "SystemBackup";
const std::string SYSTEM_LOGGING = "SystemLogging";
const std::string FIRMWARE_UPGRADE = "FirmwareUpgrade";
const std::string HTTP_FIRMWARE_UPGRADE = "HttpFirmUpgrade";
const std::string HTTP_SYSTEM_BACKUP = "HttpSystemBackup";
const std::string HTTP_SYSTEM_LOGGING = "HttpSystemLogging";
const std::string HTTP_SUPPORT_INFORMATION = "HttpSupportInformation";

const std::string AUXILIARY_COMMANDS = "AuxiliaryCommands";

const std::string STREAMING_CAPABILITIES = "StreamingCapabilities";
const std::string MEDIA_PROFILE_CAPABILITIES = "MediaProfileCapabilities";

const std::string RTP_MULTICAST = "RTPMulticast";
const std::string RTP_USCORETCP = "RTP_USCORETCP";
const std::string RTP_USCORERTSP_USCORETCP = "RTP_USCORERTSP_USCORETCP";
const std::string MAXIMUM_NUMBER_OFPROFILES = "MaximumNumberOfProfiles";

const std::string RULE_SUPPORT = "RuleSupport";
const std::string ANALYTICS_MODULE_SUPPORT = "AnalyticsMeduleSupport";

const std::string WSSUBSCRIPTION_POLICY_SUPPORT = "WSSubscriptionPlolicySupport";
const std::string WSPULL_POINT_SUPPORT = "WSPullPointSupport";
const std::string WSPSMIS = "WSPausableSubscriptionManagerInterfaceSupport";

const std::string DEVICE_IO = "DeviceIO";
const std::string RECORDING = "Recording";
const std::string SEARCH = "Search";
const std::string REPLAY = "Replay";
const std::string RECEIVER = "Receiver";

const std::string VIDEO_SOURCES = "VideoSources";
const std::string VIDEO_OUTPUTS = "VideoOutputs";
const std::string AUDIO_SOURCES = "AudioSources";
const std::string AUDIO_OUTPUTS = "AudioOutputs";
const std::string RELAY_OUTPUTS = "RelayOutputs";

const std::string DYNAMIC_RECORDINGS = "DynamicRecordings";
const std::string DYNAMIC_TRACKS = "DynamicTracks";
const std::string MAX_RATE = "MaxRate";
const std::string MAX_TOTAL_RATE = "MaxTotalRate";
const std::string MAX_RECORDINGS = "MaxRecordings";
const std::string ENCODING_STR = "Encoding";

const std::string METADATA_SEARCH = "MetadataSearch";
const std::string GENERAL_STARTEVENTS = "GeneralStartEvents";

const std::string REVERSE_PLAYBACK = "ReversePlayback";
const std::string SESSION_TIMEOUT_RANGE = "SessionTimeoutRange";

const std::string RTP_USCORE_MULTICAST = "RTP_USCOREMulticast";
const std::string SUPPORTED_RECEIVERS = "SupportedReceivers";
const std::string MAXIMUM_RTSPURI_LENGTH = "MaximumRTSPURILength";

const std::string MANUFACTURER = "Manufacturer";
const std::string MODEL = "Model";
const std::string FIRMWARE_VERSION = "FirwareVersion";
const std::string SERIAL_NUMBER = "SerialNumber";
const std::string HARDWARE_ID = "HardwareID";

const std::string DATETIME_TYPE = "DateTimeType";
const std::string TZ_STR = "TZ";

const std::string FIXED_SCOPES = "fixed_scopes";
const std::string ADDITIONAL_SCOPES = "additional_scopes";
const std::string FIXED_LEN = "fixed_len";
const std::string ADD_LEN = "add_len";

const std::string FROM_DHCP = "FromDHCP";

const std::string META_PROFILE = "meta_profile";
const std::string TOKEN_STR = "token";
const std::string OLD_TOKEN = "oldtoken";
const std::string VIDEO_SOURCE_CONFIG = "VideoSourceConfig";
const std::string VIDEO_ENCODER_CONFIG = "VideoEncoderConfig";
const std::string VIDEO_ANALYTICS_CONFIG = "VideoAnalyticsConfig";
const std::string VIDEO_RECORD_CONFIG = "VideoRecordConfig";
const std::string AUDIO_SOURCE_CONFIGURATION = "AudioSourceConfiguration";
const std::string AUDIO_ENCODER_CONFIGURATION = "AudioEncoderConfiguration";
const std::string TYPE_STR = "Type";
const std::string FIXED_STR = "fixed";
const std::string OUT_URI = "out_uri";
const std::string FRAME_FMT = "FrameFmt";

const std::string BOUNDS = "Bounds";
const std::string X_STR = "x";
const std::string Y_STR = "y";

const std::string RESOLUTION = "Resolution";
const std::string QUALITY = "Quality";
const std::string RATE_CONTROL = "RateControl";
const std::string H264_STR = "H264";

const std::string FRAME_RATE_LIMIT = "FrameRateLimit";
const std::string ENCODING_INTERVAL = "EncodingInterval";
const std::string BITRATE_LIMIT = "BitrateLimit";

const std::string GOV_LENGTH = "GovLength";
const std::string H264_PROFILE = "H264Profile";

const std::string OLD_SOURCE_TOKEN = "oldSourceToken";

const std::string RECORDING_JOB = "RecordingJob";
const std::string RECORDINGJOB_NUM = "RecordingJob_num";

const std::string RECORDINGJOB_TOKEN = "RecordingJobToken";
const std::string RECORDING_TOKEN = "RecordingToken";
const std::string MODE = "Mode";
const std::string JOB_SOURCES = "JobSources";

const std::string JOB_SOURCE = "JobSource";
const std::string CHANNEL_ID = "ChannelID";
const std::string PRIORITY = "Priority";
const std::string JOB_SOURCE_NUM = "JobSourcenum";
const std::string SOURCE_TYPE_FLG = "SourceType_flg";
const std::string SOURCE_TYPE = "SourceType";
const std::string AUTO_CREATE_RECEIVER_FLG = "AutoCreateReceiver_flg";
const std::string AUTO_CREATE_RECEIVER = "AutoCreateReceiver";

const std::string RECORDING_NUM = "Recording_num";
const std::string MAXRETENT_TIMEFLAG = "MaxRetentionTimeFlag";
const std::string SOURCE = "Source";
const std::string CONTENT = "Content";
const std::string MAXIMUM_RATENTION_TIME = "MaximumRatentionTime";
const std::string RECORDING_CFG = "Recording_Config";
const std::string SOURCE_ID = "SourceID";
const std::string LOCATION = "Location";
const std::string DESCRIPTION = "Description";
const std::string ADDRESS_STR = "Address";

const std::string ITEM_NUM = "Item_Num";
const std::string TOPIC = "topic";
const std::string ITEM = "Item";

const std::string VALUE = "Value";

const std::string SUBSCRIP = "Subscrip";
const std::string SUBSCRIPED_NUM = "Subscriped_num";

const std::string FILTER = "Filter";
const std::string SUBSCRIP_TOKEN = "SubscripToken";
const std::string TERMINATION_TIME = "TerminationTime";

const std::string PROFILEINFO_NUM = "ProfileInfo_num";
const std::string PROFILE_INFO = "ProfileInfo";
const std::string VA_USECOUNT = "VA_UseCount";
const std::string VA_ENGINE = "AnalyEngine";
const std::string RULE_ENGINE = "RuleEngine";

const std::string ANALY_CFG_NAME = "AnalyCfg_name";
const std::string ANALY_CFG_TYPE = "AnalyCfg_type";


const std::string VIDEO_ANALYTICS_NAME = "VideoAnalyticsName";
const std::string VIDEO_ANALYTICS_TOKEN = "VideoAnalyticstoken";

const std::string PROFILE_TOKEN = "ProfileToken";

const std::string RECEIVER_NUM = "Receiver_num";

const std::string CHANNEL = "Channel";
const std::string STATE_STR = "state";
const std::string CONFIG = "Config";

const std::string MEDIA_URI = "MediaUri";
const std::string PROTOCOL = "Protocol";

const std::string COMM = "Comm";
const std::string BAUD_RATE = "BaudRate";
const std::string PARITY = "Parity";
const std::string DATA_BITS = "DataBits";
const std::string STOP_BIT = "StopBit";

const std::string RELAY_TOKEN = "RelayToken";

const std::string CONNECT = "connect";
const std::string STREAM = "stream";
const std::string JPGMNG_URL = "jpgMngUrl";
const std::string VIDEO = "Video";
const std::string AUDIO_SUPPORT = "audiosupport";
const std::string CONNECT_STATE = "connect_state";

const std::string ONVIF_REMOTE_SERVER = "onvif_remote_server";
const std::string REMOTE_RTSP_PORT = "remote_rtsp_port";

const std::string PASS = "pass";
const std::string NO_AUTH = "no_auth";
const std::string TIMEDIFF_ENABLE = "TimeDiffEnable";

const std::string BASIC_CONFIG = "basic_config";
const std::string SUPPORTED = "supported";
const std::string IN_URI = "in_uri";

const std::string ENCODER_CONFIG = "encoder_config";
const std::string VIDEO_ENCODER_CONFIG_TOKEN = "video_encoder_config_token";
const std::string VIDEO_ENCODER_CONFIG_NAME = "video_encoder_config_name";
const std::string FRAME_RATE = "framerate";

const std::string JPG_URL = "JpgUrl";

const std::string VIDEO_SOURCE_TOKEN = "VideoSourceToken";
const std::string VIDEO_SOURCERELAY_TOKEN = "VideoSourceRelayToken";
const std::string VIDEO_SOURCERELAY_TOKEN_S = "VideoSourceRelayToken_S";

const std::string USER_DEFINED_CFG = "UserDefinedConfig";

size_t strlcpy(char *dst,const char *src,size_t siz)
{
	register char *d = dst;
	register const char *s = src;
	register size_t n = siz;

	/* Copy as many bytes as will fit */
	if (n != 0 && --n != 0) {
		do {
			if ((*d++ = *s++) == 0)
				break;
		} while (--n != 0);
	}

	/* Not enough room in dst, add NUL and traverse rest of src */
	if (n == 0) {
		if (siz != 0)
			*d = '\0';		/* NUL-terminate dst */
		while (*s++)
			;
	}

	return(s - src - 1);	/* count does not include NUL */
}

void StringToChar(const std::string &res_str,char *res,int len){
	strlcpy(res,res_str.c_str(),len);
}


int read_json_file(const char* file_path,Json::Value &value){
	Json::Reader reader;
	std::ifstream is;
  int ret = 0;
	is.open(file_path,std::ios::binary);
	if(!is.is_open()){
		return -1;
	}
	if(!reader.parse(is,value)) {
    ret = -1;
  }
  
  is.close();
	return ret;
}

int OnvifInfo_From_Json(Onvif_Info *pOnvifInfo, Json::Value& onvifInfoJson){

	Json::Value services = onvifInfoJson[SERVICES];

	for(int i = 0; i < MAX_ONVIF_SERVICE_TYPE; i++){

		Json::Value service = services[INFO_STR][SERVICE][i];

		StringToChar(service[NAMESPACE].asString(),pOnvifInfo->services.info.service[i].Namespace,STR_SHORT_LEN);
		StringToChar(service[X_ADDR].asString(),pOnvifInfo->services.info.service[i].XAddr,STR_SHORT_LEN);

		Json::Value service_version = service[VERSION_STR];

		pOnvifInfo->services.info.service[i].Version.Major = service_version[MAJOR].asInt();
		pOnvifInfo->services.info.service[i].Version.Minor = service_version[MINOR].asInt();

	}

	Json::Value device_service_cap = services[DEVICE_SERVICE_CAP];

	Json::Value network = device_service_cap[NETWORK];

	pOnvifInfo->services.device_service_cap.Network.IPFilter = (vz_boolean)network[IP_FILTER].asUInt();
	pOnvifInfo->services.device_service_cap.Network.ZeroConfiguration = (vz_boolean)network[ZERO_CONFIGURATION].asUInt();
	pOnvifInfo->services.device_service_cap.Network.IPVersion6 = (vz_boolean)network[IP_VERSION6].asUInt();
	pOnvifInfo->services.device_service_cap.Network.DynDNS = (vz_boolean)network[DYN_DNS].asUInt();
	pOnvifInfo->services.device_service_cap.Network.Dot11Configuration = (vz_boolean)network[DOT11_CONFIGURATION].asUInt();
	pOnvifInfo->services.device_service_cap.Network.Dot1XConfigurations = network[DOT1X_CONFIGURATIONS].asInt();
	pOnvifInfo->services.device_service_cap.Network.HostnameFromDHCP = (vz_boolean)network[HOSTNAME_FROMDHCP].asUInt();
	pOnvifInfo->services.device_service_cap.Network.NTP = network[NTP_STR].asInt();
	pOnvifInfo->services.device_service_cap.Network.DHCPv6 = (vz_boolean)network[DHC_PV6].asUInt();

	Json::Value security = device_service_cap[SECURITY_STR];

	pOnvifInfo->services.device_service_cap.Security.TLS1_x002e0 = (vz_boolean)security[TLS1_X0020E0].asUInt();
	pOnvifInfo->services.device_service_cap.Security.TLS1_x002e1 = (vz_boolean)security[TLS1_X0020E1].asUInt();
	pOnvifInfo->services.device_service_cap.Security.TLS1_x002e2 = (vz_boolean)security[TLS1_X0020E2].asUInt();
	pOnvifInfo->services.device_service_cap.Security.OnboardKeyGeneration = (vz_boolean)security[ONBOARD_KEYGENERATION].asUInt();
	pOnvifInfo->services.device_service_cap.Security.AccessPolicyConfig = (vz_boolean)security[ACCESS_POLICY_CONFIG].asUInt();
	pOnvifInfo->services.device_service_cap.Security.DefaultAccessPolicy = (vz_boolean)security[DEFAULT_ACCESS_POLICY].asUInt();
	pOnvifInfo->services.device_service_cap.Security.Dot1X = (vz_boolean)security[DOT1X].asUInt();
	pOnvifInfo->services.device_service_cap.Security.RemoteUserHandling = (vz_boolean)security[REMOTEUSER_HANDLING].asUInt();
	pOnvifInfo->services.device_service_cap.Security.X_x002e509Token = (vz_boolean)security[X_X002E509TOKEN].asUInt();
	pOnvifInfo->services.device_service_cap.Security.SAMLToken = (vz_boolean)security[SAML_TOKEN].asUInt();
	pOnvifInfo->services.device_service_cap.Security.KerberosToken = (vz_boolean)security[KERBEROS_TOKEN].asUInt();
	pOnvifInfo->services.device_service_cap.Security.UsernameToken = (vz_boolean)security[USERNAME_TOKEN].asUInt();
	pOnvifInfo->services.device_service_cap.Security.HttpDigest = (vz_boolean)security[HTTP_DIGEST].asUInt();
	pOnvifInfo->services.device_service_cap.Security.RELToken = (vz_boolean)security[REL_TOKEN].asUInt();
	StringToChar(security[SUPPORTED_EAPMETHODS].asString(),pOnvifInfo->services.device_service_cap.Security.SupportedEAPMethods,STR_SHORT_LEN);

	Json::Value device_system = device_service_cap[SYSTEM_STR];

	pOnvifInfo->services.device_service_cap.System.DiscoveryResolve = (vz_boolean)device_system[DISCOVERY_RESOLVE].asUInt();
	pOnvifInfo->services.device_service_cap.System.DiscoveryBye = (vz_boolean)device_system[DISCOVERY_BYE].asUInt();
	pOnvifInfo->services.device_service_cap.System.RemoteDiscovery = (vz_boolean)device_system[REMOTE_DISCOVERY].asUInt();
	pOnvifInfo->services.device_service_cap.System.SystemBackup = (vz_boolean)device_system[SYSTEM_BACKUP].asUInt();
	pOnvifInfo->services.device_service_cap.System.SystemLogging = (vz_boolean)device_system[SYSTEM_LOGGING].asUInt();
	pOnvifInfo->services.device_service_cap.System.FirmwareUpgrade = (vz_boolean)device_system[FIRMWARE_UPGRADE].asUInt();
	pOnvifInfo->services.device_service_cap.System.HttpFirmwareUpgrade = (vz_boolean)device_system[HTTP_FIRMWARE_UPGRADE].asUInt();
	pOnvifInfo->services.device_service_cap.System.HttpSystemBackup = (vz_boolean)device_system[HTTP_SYSTEM_BACKUP].asUInt();
	pOnvifInfo->services.device_service_cap.System.HttpSystemLogging = (vz_boolean)device_system[HTTP_SYSTEM_LOGGING].asUInt();
	pOnvifInfo->services.device_service_cap.System.HttpSupportInformation = (vz_boolean)device_system[HTTP_SUPPORT_INFORMATION].asUInt();

	Json::Value misc = device_service_cap[MISC_STR];

	StringToChar(misc[AUXILIARY_COMMANDS].asString(),pOnvifInfo->services.device_service_cap.Misc.AuxiliaryCommands,STR_SHORT_LEN);

	Json::Value media_cap = services[MEDIA_CAP];

	Json::Value streaming_capabilities = media_cap[STREAMING_CAPABILITIES];

	pOnvifInfo->services.media_cap.StreamingCapabilities.RTPMulticast = (vz_boolean)streaming_capabilities[RTP_MULTICAST].asUInt();
	pOnvifInfo->services.media_cap.StreamingCapabilities.RTP_USCORETCP = (vz_boolean)streaming_capabilities[RTP_USCORETCP].asUInt();
	pOnvifInfo->services.media_cap.StreamingCapabilities.RTP_USCORERTSP_USCORETCP = (vz_boolean)streaming_capabilities[RTP_USCORERTSP_USCORETCP].asUInt();

	Json::Value media_profile_capabilities = media_cap[MEDIA_PROFILE_CAPABILITIES];

	pOnvifInfo->services.media_cap.MediaProfileCapabilities.MaximumNumberOfProfiles = media_profile_capabilities[MAXIMUM_NUMBER_OFPROFILES].asInt();

	Json::Value analytic_cap = services[ANALYTIC_CAP];

	pOnvifInfo->services.Analytic_cap.RuleSupport = (vz_boolean)analytic_cap[RULE_SUPPORT].asUInt();
	pOnvifInfo->services.Analytic_cap.AnalyticsModuleSupport = (vz_boolean)analytic_cap[ANALYTICS_MODULE_SUPPORT].asUInt();

	Json::Value event_cap = services[EVENT_CAP];

	pOnvifInfo->services.Event_cap.WSSubscriptionPolicySupport = (vz_boolean)event_cap[WSSUBSCRIPTION_POLICY_SUPPORT].asUInt();
	pOnvifInfo->services.Event_cap.WSPullPointSupport = (vz_boolean)event_cap[WSPULL_POINT_SUPPORT].asUInt();
	pOnvifInfo->services.Event_cap.WSPausableSubscriptionManagerInterfaceSupport = (vz_boolean)event_cap[WSPSMIS].asUInt();

	Json::Value extension_cap = services[EXTENSION_CAP];

	Json::Value device_io = extension_cap[DEVICE_IO];

	pOnvifInfo->services.Extension_cap.DeviceIO.VideoSources = device_io[VIDEO_SOURCES].asInt();
	pOnvifInfo->services.Extension_cap.DeviceIO.AudioOutputs = device_io[AUDIO_OUTPUTS].asInt();
	pOnvifInfo->services.Extension_cap.DeviceIO.AudioSources = device_io[AUDIO_SOURCES].asInt();
	pOnvifInfo->services.Extension_cap.DeviceIO.AudioOutputs = device_io[AUDIO_OUTPUTS].asInt();
	pOnvifInfo->services.Extension_cap.DeviceIO.RelayOutputs = device_io[RELAY_OUTPUTS].asInt();

	Json::Value recording = extension_cap[RECORDING];

	pOnvifInfo->services.Extension_cap.Recording.DynamicRecordings = (vz_boolean)recording[DYNAMIC_RECORDINGS].asUInt();
	pOnvifInfo->services.Extension_cap.Recording.DynamicTracks = (vz_boolean)recording[DYNAMIC_TRACKS].asUInt();
	pOnvifInfo->services.Extension_cap.Recording.MaxRate = recording[MAX_RATE].asInt();
	pOnvifInfo->services.Extension_cap.Recording.MaxTotalRate = recording[MAX_TOTAL_RATE].asInt();
	pOnvifInfo->services.Extension_cap.Recording.MaxRecordings = recording[MAX_RECORDINGS].asInt();
	StringToChar(recording[ENCODING_STR].asString(),pOnvifInfo->services.Extension_cap.Recording.Encoding,NAME_STR_LEN);

	Json::Value extension_search = extension_cap[SEARCH];

	pOnvifInfo->services.Extension_cap.Search.MetadataSearch = (vz_boolean)extension_search[METADATA_SEARCH].asUInt();
	pOnvifInfo->services.Extension_cap.Search.GeneralStartEvents = (vz_boolean)extension_search[GENERAL_STARTEVENTS].asUInt();

	Json::Value replay = extension_cap[REPLAY];

	pOnvifInfo->services.Extension_cap.Replay.ReversePlayback = (vz_boolean)replay[REVERSE_PLAYBACK].asUInt();
	pOnvifInfo->services.Extension_cap.Replay.RTP_USCORERTSP_USCORETCP = (vz_boolean)replay[RTP_USCORERTSP_USCORETCP].asUInt();
	StringToChar(replay[SESSION_TIMEOUT_RANGE].asString(),pOnvifInfo->services.Extension_cap.Replay.SessionTimeoutRange,STR_SHORT_LEN);

	Json::Value receiver = extension_cap[RECEIVER];

	pOnvifInfo->services.Extension_cap.Receiver.RTP_USCOREMulticast = (vz_boolean)receiver[RTP_USCORE_MULTICAST].asUInt();
	pOnvifInfo->services.Extension_cap.Receiver.RTP_USCORETCP = (vz_boolean)receiver[RTP_USCORETCP].asUInt();
	pOnvifInfo->services.Extension_cap.Receiver.RTP_USCORERTSP_USCORETCP = (vz_boolean)receiver[RTP_USCORERTSP_USCORETCP].asUInt();
	pOnvifInfo->services.Extension_cap.Receiver.SupportedReceivers = receiver[SUPPORTED_RECEIVERS].asInt();
	pOnvifInfo->services.Extension_cap.Receiver.MaximumRTSPURILength = receiver[MAXIMUM_RTSPURI_LENGTH].asInt();

	Json::Value device_information = onvifInfoJson[DEVICE_INFORMATION];

	StringToChar(device_information[MANUFACTURER].asString(),pOnvifInfo->device_information.Manufacturer,STR_SHORT_LEN);
	StringToChar(device_information[MODEL].asString(),pOnvifInfo->device_information.Model,STR_SHORT_LEN);
	StringToChar(device_information[FIRMWARE_VERSION].asString(),pOnvifInfo->device_information.FirmwareVersion,STR_SHORT_LEN);
	StringToChar(device_information[SERIAL_NUMBER].asString(),pOnvifInfo->device_information.SerialNumber,STR_SHORT_LEN);
	StringToChar(device_information[HARDWARE_ID].asString(),pOnvifInfo->device_information.HardwareId,STR_SHORT_LEN);

	Json::Value time_cfg = onvifInfoJson[TIME_CFG];

	pOnvifInfo->time_cfg.DateTimeType = (vz__SetDateTimeType)time_cfg[DATETIME_TYPE].asUInt();
	StringToChar(time_cfg[TZ_STR].asString(),pOnvifInfo->time_cfg.TZ,STR_SHORT_LEN);

	Json::Value scopes = onvifInfoJson[SCOPES];

	for(int i = 0; i < FIXED_SCOPE_LEN; i++){

		Json::Value fixed_scopes = scopes[FIXED_SCOPES];

		StringToChar(fixed_scopes[i].asString(),pOnvifInfo->scopes.fixed_scopes[i],SCOPE_MAX_LEN);
	}

	for(int i = 0; i < ADDTIONAL_SCOPE_LEN; i++){

		Json::Value additional_scopes = scopes[ADDITIONAL_SCOPES];

		StringToChar(additional_scopes[i].asString(),pOnvifInfo->scopes.addtional_scopes[i],SCOPE_MAX_LEN);
	}

	pOnvifInfo->scopes.fixed_len = scopes[FIXED_LEN].asInt();
	pOnvifInfo->scopes.add_len = scopes[ADD_LEN].asInt();

	Json::Value hostname = onvifInfoJson[HOST_NAME];

	StringToChar(hostname[NAME_STR].asString(),pOnvifInfo->hostname.name,STR_SHORT_LEN);
	pOnvifInfo->hostname.FromDHCP = (vz_boolean)hostname[FROM_DHCP].asUInt();

	for(int i = 0;i < MAX_CHANNEL; i++){

		Json::Value profiles = onvifInfoJson[PROFILES][i];

		for(int j = 0; j < MAX_PROFILE_NUM; j++){

			Json::Value meta_profile = profiles[META_PROFILE][j];

			StringToChar(meta_profile[NAME_STR].asString(),pOnvifInfo->Profiles[i].meta_profile[j].Name,NAME_STR_LEN);
			StringToChar(meta_profile[TOKEN_STR].asString(),pOnvifInfo->Profiles[i].meta_profile[j].token,NAME_STR_LEN);
			StringToChar(meta_profile[OLD_TOKEN].asString(),pOnvifInfo->Profiles[i].meta_profile[j].oldtoken,NAME_STR_LEN);

			Json::Value video_source_config = meta_profile[VIDEO_SOURCE_CONFIG];

			pOnvifInfo->Profiles[i].meta_profile[j].VideoSourceConfig.UserCount = video_source_config[USER_COUNT].asInt();
			StringToChar(video_source_config[NAME_STR].asString(),pOnvifInfo->Profiles[i].meta_profile[j].VideoSourceConfig.Name,NAME_STR_LEN);
			StringToChar(video_source_config[TOKEN_STR].asString(),pOnvifInfo->Profiles[i].meta_profile[j].VideoSourceConfig.token,NAME_STR_LEN);
			StringToChar(video_source_config[OLD_TOKEN].asString(),pOnvifInfo->Profiles[i].meta_profile[j].VideoSourceConfig.oldtoken,NAME_STR_LEN);

			Json::Value video_bounds = video_source_config[BOUNDS];

			pOnvifInfo->Profiles[i].meta_profile[j].VideoSourceConfig.Bounds.x = video_bounds[X_STR].asInt();
			pOnvifInfo->Profiles[i].meta_profile[j].VideoSourceConfig.Bounds.y = video_bounds[Y_STR].asInt();
			pOnvifInfo->Profiles[i].meta_profile[j].VideoSourceConfig.Bounds.width = video_bounds[WIDTH_STR].asInt();
			pOnvifInfo->Profiles[i].meta_profile[j].VideoSourceConfig.Bounds.height = video_bounds[HEIGHT_STR].asInt();

			StringToChar(video_source_config[NAME_STR].asString(),pOnvifInfo->Profiles[i].meta_profile[j].VideoEncoderConfig.Name,NAME_STR_LEN);
			StringToChar(video_source_config[TOKEN_STR].asString(),pOnvifInfo->Profiles[i].meta_profile[j].VideoEncoderConfig.token,NAME_STR_LEN);
			StringToChar(video_source_config[OLD_TOKEN].asString(),pOnvifInfo->Profiles[i].meta_profile[j].VideoEncoderConfig.oldtoken,NAME_STR_LEN);
			pOnvifInfo->Profiles[i].meta_profile[j].VideoEncoderConfig.Encoding = video_source_config[ENCODING_STR]. asInt();

			Json::Value video_encoder_config = meta_profile[VIDEO_ENCODER_CONFIG];

			Json::Value video_resolution = video_encoder_config[RESOLUTION];

			pOnvifInfo->Profiles[i].meta_profile[j].VideoEncoderConfig.Resolution.Width = video_resolution[WIDTH_STR].asInt();
			pOnvifInfo->Profiles[i].meta_profile[j].VideoEncoderConfig.Resolution.Height = video_resolution[HEIGHT_STR].asInt();

			pOnvifInfo->Profiles[i].meta_profile[j].VideoEncoderConfig.Quality = video_encoder_config[QUALITY].asFloat();

			Json::Value rate_control = video_encoder_config[RATE_CONTROL];

			pOnvifInfo->Profiles[i].meta_profile[j].VideoEncoderConfig.RateControl.FrameRateLimit = rate_control[FRAME_RATE_LIMIT].asInt();
			pOnvifInfo->Profiles[i].meta_profile[j].VideoEncoderConfig.RateControl.EncodingInterval = rate_control[ENCODING_INTERVAL].asInt();
			pOnvifInfo->Profiles[i].meta_profile[j].VideoEncoderConfig.RateControl.BitrateLimit = rate_control[BITRATE_LIMIT].asInt();

			Json::Value h264 = video_encoder_config[H264_STR];

			pOnvifInfo->Profiles[i].meta_profile[j].VideoEncoderConfig.H264.GovLength = h264[GOV_LENGTH].asInt();
			pOnvifInfo->Profiles[i].meta_profile[j].VideoEncoderConfig.H264.H264Profile = h264[H264_PROFILE]. asInt();

			/*Json::Value video_analytics_config = meta_profile[VIDEO_ANALYTICS_CONFIG];

			StringToChar(video_analytics_config[NAME_STR].asString(),pOnvifInfo->Profiles[i].meta_profile[j].VideoAnalyticsConfig.Name,NAME_STR_LEN);
			StringToChar(video_analytics_config[TOKEN_STR].asString(),pOnvifInfo->Profiles[i].meta_profile[j].VideoAnalyticsConfig.token,NAME_STR_LEN);
			*/
			Json::Value video_record_config = meta_profile[VIDEO_RECORD_CONFIG];

			StringToChar(video_record_config[TOKEN_STR].asString(),pOnvifInfo->Profiles[i].meta_profile[j].VideoRecordConfig.token,NAME_STR_LEN);
			pOnvifInfo->Profiles[i].meta_profile[j].VideoRecordConfig.bEnable = video_record_config[B_ENABLE]. asInt();
			pOnvifInfo->Profiles[i].meta_profile[j].VideoRecordConfig.eRecID = video_record_config[E_RECID]. asInt();

			Json::Value audio_source_configuration = meta_profile[AUDIO_SOURCE_CONFIGURATION];

			StringToChar(audio_source_configuration[NAME_STR].asString(),pOnvifInfo->Profiles[i].meta_profile[j].AudioSourceConfiguration.Name,NAME_STR_LEN);
			StringToChar(audio_source_configuration[TOKEN_STR].asString(),pOnvifInfo->Profiles[i].meta_profile[j].AudioSourceConfiguration.token,NAME_STR_LEN);
			StringToChar(audio_source_configuration[OLD_TOKEN].asString(),pOnvifInfo->Profiles[i].meta_profile[j].AudioSourceConfiguration.oldtoken,NAME_STR_LEN);
			StringToChar(audio_source_configuration[OLD_SOURCE_TOKEN].asString(),pOnvifInfo->Profiles[i].meta_profile[j].AudioSourceConfiguration.oldSourceToken,NAME_STR_LEN);

			Json::Value audio_encoder_configuration = meta_profile[AUDIO_ENCODER_CONFIGURATION];

			StringToChar(audio_encoder_configuration[NAME_STR].asString(),pOnvifInfo->Profiles[i].meta_profile[j].AudioEncoderConfiguration.Name,NAME_STR_LEN);
			StringToChar(audio_encoder_configuration[TOKEN_STR].asString(),pOnvifInfo->Profiles[i].meta_profile[j].AudioEncoderConfiguration.token,NAME_STR_LEN);
			StringToChar(audio_encoder_configuration[OLD_TOKEN].asString(),pOnvifInfo->Profiles[i].meta_profile[j].AudioEncoderConfiguration.oldtoken,NAME_STR_LEN);

			pOnvifInfo->Profiles[i].meta_profile[j].Type = (Profile_Type)meta_profile[TYPE_STR].asUInt();
			pOnvifInfo->Profiles[i].meta_profile[j].fixed = (vz_boolean)meta_profile[FIXED_STR].asUInt();
			StringToChar(meta_profile[OUT_URI].asString(),pOnvifInfo->Profiles[i].meta_profile[j].out_uri,MAX_URI_LEN);
			pOnvifInfo->Profiles[i].meta_profile[j].frameFmt = meta_profile[FRAME_FMT].asInt();

		}
	}

	Json::Value recording_jobs = onvifInfoJson[RECORDING_JOBS];

	for(int i = 0; i < MAX_RECORDJOB_NUM; i++){

		Json::Value recording_job = recording_jobs[RECORDING_JOB][i];

		StringToChar(recording_job[RECORDINGJOB_TOKEN].asString(),pOnvifInfo->RecordingJobs.RecordingJob[i].RecordingJobToken,NAME_STR_LEN);
		StringToChar(recording_job[RECORDING_TOKEN].asString(),pOnvifInfo->RecordingJobs.RecordingJob[i].RecordingToken,NAME_STR_LEN);
		StringToChar(recording_job[MODE].asString(),pOnvifInfo->RecordingJobs.RecordingJob[i].Mode,NAME_STR_LEN);

		Json::Value JobSources = recording_job[JOB_SOURCES];
		for(int j = 0; j < MAX_RJTRACK_NUM; j++){
			Json::Value JobSource = JobSources[JOB_SOURCE][j];
			pOnvifInfo->RecordingJobs.RecordingJob[i].JobSource[j].SourceTokenFlag=JobSource[SOURCE_TOKEN_FLG].asInt();
			pOnvifInfo->RecordingJobs.RecordingJob[i].JobSource[j].AutoCreateReceiverFlag=JobSource[AUTO_CREATE_RECEIVER_FLG].asInt();
			pOnvifInfo->RecordingJobs.RecordingJob[i].JobSource[j].AutoCreateReceiver=JobSource[AUTO_CREATE_RECEIVER].asInt();
			Json::Value SourceToken = JobSource[SOURCE_TOKEN_STRUCT];
			pOnvifInfo->RecordingJobs.RecordingJob[i].JobSource[j].SourceToken.TypeFlag=SourceToken[SOURCE_TYPE_FLG].asInt();
			StringToChar(SourceToken[SOURCE_TOKEN].asString(), pOnvifInfo->RecordingJobs.RecordingJob[i].JobSource[j].SourceToken.Token,STR_SHORT_LEN);
			StringToChar(SourceToken[SOURCE_TYPE].asString(), pOnvifInfo->RecordingJobs.RecordingJob[i].JobSource[j].SourceToken.Type,STR_SHORT_LEN);
		}
		pOnvifInfo->RecordingJobs.RecordingJob[i].sizeSource=recording_job[JOB_SOURCE_NUM].asInt();

		//pOnvifInfo->RecordingJobs.RecordingJob[i].ChannelID = recording_job[CHANNEL_ID].asInt();
		pOnvifInfo->RecordingJobs.RecordingJob[i].Priority = recording_job[PRIORITY].asInt();

	}

	pOnvifInfo->RecordingJobs.RecordingJob_num = recording_jobs[RECORDINGJOB_NUM].asInt();

	Json::Value recordings = onvifInfoJson[RECORDINGS];

	for(int i = 0; i < MAX_RECORD_NUM; i++){

		Json::Value recording = recordings[RECORDING][i];

		StringToChar(recording[RECORDING_TOKEN].asString(),pOnvifInfo->Recordings.Recording[i].RecordingToken,NAME_STR_LEN);
		Json::Value cfg=recording[RECORDING_CFG];
		pOnvifInfo->Recordings.Recording[i].Config.MaximumRetentionTimeFlag=cfg[MAXRETENT_TIMEFLAG].asInt();
		pOnvifInfo->Recordings.Recording[i].Config.MaximumRetentionTime=cfg[MAXIMUM_RATENTION_TIME].asInt();
		StringToChar(cfg[CONTENT].asString(),pOnvifInfo->Recordings.Recording[i].Config.Content,STR_SHORT_LEN);


		Json::Value recording_source = cfg[SOURCE];

		StringToChar(recording_source[SOURCE_ID].asString(),pOnvifInfo->Recordings.Recording[i].Config.Source.SourceId,STR_SHORT_LEN);
		StringToChar(recording_source[NAME_STR].asString(),pOnvifInfo->Recordings.Recording[i].Config.Source.Name,VIDEO_SOURCE_TOKEN_LEN);
		StringToChar(recording_source[LOCATION].asString(),pOnvifInfo->Recordings.Recording[i].Config.Source.Location,VIDEO_SOURCE_TOKEN_LEN);
		StringToChar(recording_source[DESCRIPTION].asString(),pOnvifInfo->Recordings.Recording[i].Config.Source.Description,VIDEO_SOURCE_TOKEN_LEN);
		StringToChar(recording_source[ADDRESS_STR].asString(),pOnvifInfo->Recordings.Recording[i].Config.Source.Address,STR_SHORT_LEN);

		pOnvifInfo->Recordings.Recording[i].channel = recording[CHANNEL_ID].asInt();

	}
	pOnvifInfo->Recordings.Recording_num = recordings[RECORDING_NUM].asInt();

	Json::Value onvif_event = onvifInfoJson[EVENT_STR];

	pOnvifInfo->Event.Item_Num = onvif_event[ITEM_NUM].asInt();
	StringToChar(onvif_event[TOPIC].asString(),pOnvifInfo->Event.topic,STR_SHORT_LEN);

	for(int i = 0; i < MAX_SIMPLEITEM_NUM; i++){

		Json::Value event_item = onvif_event[ITEM][i];

		StringToChar(event_item[NAME_STR].asString(),pOnvifInfo->Event.Item[i].Name,NAME_STR_LEN);
		StringToChar(event_item[VALUE].asString(),pOnvifInfo->Event.Item[i].Value,STR_SHORT_LEN);

	}

	Json::Value subscriptions = onvifInfoJson[SUBSCRIPTIONS];

	for(int i = 0; i < MAX_SUBSCRIP_NUM; i++){

		Json::Value subscrip = subscriptions[SUBSCRIP][i];

		StringToChar(subscrip[FILTER].asString(),pOnvifInfo->Subscriptions.Subscrip[i].Filter,STR_SHORT_LEN);
		StringToChar(subscrip[SUBSCRIP_TOKEN].asString(),pOnvifInfo->Subscriptions.Subscrip[i].SubscripToken,STR_SHORT_LEN);
		pOnvifInfo->Subscriptions.Subscrip[i].TerminationTime = subscrip[TERMINATION_TIME].asInt();

	}
	pOnvifInfo->Subscriptions.Subscriped_num = subscriptions[SUBSCRIPED_NUM].asInt();

	for(int i = 0; i < MAX_VIDEO_ANALYTICS_NUM; i++){

		Json::Value video_analytics = onvifInfoJson[VIDEO_ANALYTICS_STR][i];

		pOnvifInfo->VideoAnalytics[i].used = video_analytics[USED_STR].asInt();
		Json::Value VACfg = video_analytics[VA_CFG];

		StringToChar(VACfg[VIDEO_ANALYTICS_NAME].asString(),pOnvifInfo->VideoAnalytics[i].VACfg.Name,STR_SHORT_LEN);	
		StringToChar(VACfg[VIDEO_ANALYTICS_TOKEN].asString(),pOnvifInfo->VideoAnalytics[i].VACfg.token,STR_SHORT_LEN);	
		pOnvifInfo->VideoAnalytics[i].VACfg.UseCount = VACfg[VA_USECOUNT].asInt();	
		
		for(int k = 0; k < MAX_ANALY_MODULE_NUM; k++){
			Json::Value AnalyticsModule = VACfg[VA_ENGINE][k];
			StringToChar(AnalyticsModule[ANALY_CFG_NAME].asString(),pOnvifInfo->VideoAnalytics[i].VACfg.AnalyticsEngineCfg.AnalyticsModule[k].Name,NAME_STR_LEN);
			StringToChar(AnalyticsModule[ANALY_CFG_TYPE].asString(),pOnvifInfo->VideoAnalytics[i].VACfg.AnalyticsEngineCfg.AnalyticsModule[k].Type,NAME_STR_LEN);
		}
		
		for(int k = 0; k < MAX_ANALY_RULE_NUM; k++){
			Json::Value RuleEngine = VACfg[RULE_ENGINE][k];
			StringToChar(RuleEngine[ANALY_CFG_NAME].asString(),pOnvifInfo->VideoAnalytics[i].VACfg.RuleEngineCfg.Rule[k].Name,NAME_STR_LEN);
			StringToChar(RuleEngine[ANALY_CFG_TYPE].asString(),pOnvifInfo->VideoAnalytics[i].VACfg.RuleEngineCfg.Rule[k].Type,NAME_STR_LEN);
		}
	}

	Json::Value receivers = onvifInfoJson[RECEIVERS];

	for(int i = 0; i < MAX_RECEIVER_NUM; i++){

		Json::Value receiver = receivers[RECEIVER][i];

		StringToChar(receiver[TOKEN_STR].asString(),pOnvifInfo->Receivers.Receiver[i].Token,VIDEO_SOURCE_TOKEN_LEN);
		pOnvifInfo->Receivers.Receiver[i].channel = receiver[CHANNEL].asInt();
		pOnvifInfo->Receivers.Receiver[i].state = (vz_ReceiverState)receiver[STATE_STR].asUInt();

		Json::Value receiver_config = receiver[CONFIG];

		StringToChar(receiver_config[MEDIA_URI].asString(),pOnvifInfo->Receivers.Receiver[i].Config.MediaUri,STR_SHORT_LEN);
		pOnvifInfo->Receivers.Receiver[i].Config.Mode = (vz_ReceiverMode)receiver_config[MODE].asUInt();
		pOnvifInfo->Receivers.Receiver[i].Config.StreamType = (vz_StreamType)receiver_config[STREAM_TYPE].asUInt();
		pOnvifInfo->Receivers.Receiver[i].Config.Protocol = (vz_TransportProtocol)receiver_config[PROTOCOL].asUInt();

	}
	pOnvifInfo->Receivers.Receiver_num = receivers[RECEIVER_NUM].asInt();

	for(int i = 0 ; i < MAX_SERIAL_PORTS_NUM; i++){

		Json::Value serial_ports = onvifInfoJson[SERIAL_PORTS][i];

		Json::Value serial_config = serial_ports[CONFIG];

		pOnvifInfo->SerialPorts[i].Config.Comm = serial_config[COMM].asUInt();
		pOnvifInfo->SerialPorts[i].Config.BaudRate = serial_config[BAUD_RATE].asUInt();
		pOnvifInfo->SerialPorts[i].Config.Parity = serial_config[PARITY].asUInt();
		pOnvifInfo->SerialPorts[i].Config.DataBits = serial_config[DATA_BITS].asUInt();
		pOnvifInfo->SerialPorts[i].Config.StopBit = serial_config[STOP_BIT].asUInt();
		StringToChar(serial_config[TYPE_STR].asString(),pOnvifInfo->SerialPorts[i].Config.Type,NAME_STR_LEN);

		StringToChar(serial_ports[TOKEN_STR].asString(),pOnvifInfo->SerialPorts[i].Token,NAME_STR_LEN);

	}
/*
	for(int i = 0; i < MAX_CHANNEL; i++){

		Json::Value osd_cfgs = onvifInfoJson[OSD_CFGS][i];

		for(int j = 0 ; j < MAX_OSD_NUM_ONE_CHANNEL; j++){

			Json::Value token = osd_cfgs[TOKEN_STR];

			StringToChar(token[j].asString(),pOnvifInfo->OSDCfgs[i].Token[j],VIDEO_SOURCE_TOKEN_LEN);
		}
		for(int j = 0; j < MAX_OSD_NUM_ONE_CHANNEL; j++){

			Json::Value relay_token = osd_cfgs[RELAY_TOKEN];

			StringToChar(relay_token[j].asString(),pOnvifInfo->OSDCfgs[i].RelayToken[j],VIDEO_SOURCE_TOKEN_LEN);
		}

	}
*/
#ifdef ONVIF_USB_LAN

	for(int i = 0; i < MAX_REMOTE_CHANNEL; i++){

		Json::Value remote_server = onvifInfoJson[REMOTE_SERVER][i];

		Json::Value remote_connect = remote_server[CONNECT];

		Json::Value onvif_remote_server = remote_connect[ONVIF_REMOTE_SERVER];

		StringToChar(onvif_remote_server[HOST_NAME].asString(),pOnvifInfo->remote_server[i].connect.onvif_remote_server.hostname,NAME_STR_LEN);
		pOnvifInfo->remote_server[i].connect.onvif_remote_server.port = onvif_remote_server[PORT_STR].asInt();
		StringToChar(onvif_remote_server[USER].asString(),pOnvifInfo->remote_server[i].connect.onvif_remote_server.user,NAME_STR_LEN);
		StringToChar(onvif_remote_server[PASS].asString(),pOnvifInfo->remote_server[i].connect.onvif_remote_server.pass,NAME_STR_LEN);
		pOnvifInfo->remote_server[i].connect.onvif_remote_server.no_auth = onvif_remote_server[NO_AUTH]. asInt();
		pOnvifInfo->remote_server[i].connect.onvif_remote_server.TimeDiffEnable = onvif_remote_server[TIMEDIFF_ENABLE].asUInt();

		pOnvifInfo->remote_server[i].connect.remote_rtsp_port = remote_connect[REMOTE_RTSP_PORT].asUInt();

		for(int j = 0; j < MAX_ONVIF_SERVICE_TYPE; j++){

			Json::Value service = remote_server[SERVICE];

			Json::Value service_info = service[INFO_STR];

			Json::Value info_service = service_info[SERVICE][j];

			StringToChar(info_service[NAMESPACE].asString(),pOnvifInfo->remote_server[i].service.info.service[j].Namespace,STR_SHORT_LEN);
			StringToChar(info_service[NAMESPACE].asString(),pOnvifInfo->remote_server[i].service.info.service[j].XAddr,STR_SHORT_LEN);

			Json::Value service_version = info_service[VERSION_STR];

			pOnvifInfo->remote_server[i].service.info.service[j].Version.Major = service_version[MAJOR].asInt();
			pOnvifInfo->remote_server[i].service.info.service[j].Version.Minor = service_version[MINOR].asInt();

		}

		Json::Value remote_stream = remote_server[STREAM];

		Json::Value main_stream = remote_stream[MAIN_STREAM];

		Json::Value main_basic_config = main_stream[BASIC_CONFIG];

		pOnvifInfo->remote_server[i].stream.main_stream.basic_config.supported = (vz_boolean)main_basic_config[SUPPORTED].asUInt();
		pOnvifInfo->remote_server[i].stream.main_stream.basic_config.enable = (vz_boolean)main_basic_config[ENABLE_STR].asUInt();
		StringToChar(main_basic_config[PROFILE_TOKEN].asString(),pOnvifInfo->remote_server[i].stream.main_stream.basic_config.profile_token,PROFILE_TOKEN_LEN);
		StringToChar(main_basic_config[IN_URI].asString(),pOnvifInfo->remote_server[i].stream.main_stream.basic_config.in_uri,MAX_URI_LEN);

		Json::Value sub_stream = remote_stream[SUB_STREAM];

		Json::Value sub_basic_config = sub_stream[BASIC_CONFIG];

		pOnvifInfo->remote_server[i].stream.sub_stream.basic_config.supported = (vz_boolean)sub_basic_config[SUPPORTED].asUInt();
		pOnvifInfo->remote_server[i].stream.sub_stream.basic_config.enable = (vz_boolean)sub_basic_config[ENABLE_STR].asUInt();
		StringToChar(sub_basic_config[PROFILE_TOKEN].asString(),pOnvifInfo->remote_server[i].stream.sub_stream.basic_config.profile_token,PROFILE_TOKEN_LEN);
		StringToChar(sub_basic_config[IN_URI].asString(),pOnvifInfo->remote_server[i].stream.sub_stream.basic_config.in_uri,MAX_URI_LEN);

		Json::Value encoder_config = sub_stream[ENCODER_CONFIG];

		StringToChar(encoder_config[VIDEO_ENCODER_CONFIG_TOKEN].asString(),pOnvifInfo->remote_server[i].stream.sub_stream.encoder_config.video_encoder_config_token,PROFILE_TOKEN_LEN);
		StringToChar(encoder_config[VIDEO_ENCODER_CONFIG_NAME].asString(),pOnvifInfo->remote_server[i].stream.sub_stream.encoder_config.video_encoder_config_name,PROFILE_TOKEN_LEN);
		pOnvifInfo->remote_server[i].stream.sub_stream.encoder_config.encoding = encoder_config[ENCODING_STR].asInt();
		pOnvifInfo->remote_server[i].stream.sub_stream.encoder_config.width = encoder_config[WIDTH_STR].asInt();
		pOnvifInfo->remote_server[i].stream.sub_stream.encoder_config.height = encoder_config[HEIGHT_STR].asInt();
		pOnvifInfo->remote_server[i].stream.sub_stream.encoder_config.framerate = encoder_config[FRAME_RATE].asInt();

		Json::Value alarm_stream = remote_stream[ALARM_STREAM];

		pOnvifInfo->remote_server[i].stream.alarm_stream.supported = (vz_boolean)alarm_stream[SUPPORTED].asUInt();
		pOnvifInfo->remote_server[i].stream.alarm_stream.enable = (vz_boolean)alarm_stream[ENABLE_STR].asUInt();
		StringToChar(alarm_stream[PROFILE_TOKEN].asString(),pOnvifInfo->remote_server[i].stream.alarm_stream.profile_token,PROFILE_TOKEN_LEN);
		StringToChar(alarm_stream[IN_URI].asString(),pOnvifInfo->remote_server[i].stream.alarm_stream.in_uri,MAX_URI_LEN);

		Json::Value jpgmng_url = remote_server[JPGMNG_URL];

		StringToChar(jpgmng_url[JPG_URL].asString(),pOnvifInfo->remote_server[i].jpgMngUrl.JpgUrl,MAX_URI_LEN);

		Json::Value remote_video = remote_server[VIDEO];

		StringToChar(remote_video[VIDEO_SOURCE_TOKEN].asString(),pOnvifInfo->remote_server[i].video.VideoSourceToken,VIDEO_SOURCE_TOKEN_LEN);
		StringToChar(remote_video[VIDEO_SOURCERELAY_TOKEN].asString(),pOnvifInfo->remote_server[i].video.VideoSourceRelayToken,VIDEO_SOURCE_TOKEN_LEN);
		StringToChar(remote_video[VIDEO_SOURCERELAY_TOKEN_S].asString(),pOnvifInfo->remote_server[i].video.VideoSourceRelayToken_S,VIDEO_SOURCE_TOKEN_LEN);

		pOnvifInfo->remote_server[i].audiosupport = remote_server[AUDIO_SUPPORT].asInt();
		pOnvifInfo->remote_server[i].connect_state = remote_server[CONNECT_STATE]. asInt();

	}

#endif
	return 0;
}



int JsonRead_OnvifInfo(const char* file_path,Onvif_Info *pOnvifInfo){
	Json::Value onvifInfoJson;
	if(read_json_file(file_path,onvifInfoJson)!=0) return -1;
	return OnvifInfo_From_Json(pOnvifInfo,onvifInfoJson);
}



bool ParserStr2Json(std::string &inputstr,Json::Value &outjson) {
  Json::Reader reader;
  bool parsingSuccessful = reader.parse(inputstr, outjson);
  if (!parsingSuccessful) {
    printf("parsingSuccessful error"); //reader.getFormattedErrorMessages();
    return false;
  }
  return true;
}


int ParserNetPort(const char *value,int value_size,NetworkPort *net_port) {
  std::string jsonstr;
  jsonstr.assign(value, value_size);
  Json::Value cfg_json;
  bool res = ParserStr2Json(jsonstr,cfg_json);
  if (!res) {
    return -1;
  }
  net_port->http_port = cfg_json[HTTP_PORT].asUInt();
  net_port->rtsp_port = cfg_json[RTSP_PORT].asUInt();
  return 0;
}

int ParserNetInterface(const char *value,int value_size,NetworkInfo *net_interface) {
  std::string jsonstr;
  jsonstr.assign(value, value_size);
  Json::Value cfg_json;
  bool res = ParserStr2Json(jsonstr,cfg_json);
  if (!res) {
    return -1;
  }
  net_interface->ip.s_addr = inet_addr(cfg_json[IP_IN_ADDR].asCString());
  net_interface->netmask.s_addr = inet_addr(cfg_json[NETMASK_IN_ADDR].asCString());
  net_interface->gateway.s_addr = inet_addr(cfg_json[GATEWAY_IN_ADDR].asCString());
  net_interface->dns.s_addr = inet_addr(cfg_json[DNS_IN_ADDR].asCString());
  net_interface->dhcp_enable = cfg_json[DHCP_ENABLE].asInt();
  return 0;
}




