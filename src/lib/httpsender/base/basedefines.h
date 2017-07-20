#ifndef HTTP_SENDER_BASE_BASEDEFINES_H_
#define HTTP_SENDER_BASE_BASEDEFINES_H_

namespace hs {

typedef char __u8;

#define MAX_STREAM_NAME			25 		///< Maximum length of normal string.
#define MAC_LENGTH				6 		///< Length of MAC address.
#define ACOUNT_NUM				10 		///< How many acounts which are stored in system.
#define SCHDULE_NUM				8 		///< How many schedules will be stored in system.
#define MAX_FILE_NAME			128		///< Maximum length of file name.
#define MAX_HOSTNAME_LEN		128
#ifndef MAX_URI_LEN
#define MAX_URI_LEN				1024
#endif

#define MAX_SERIALDATA_LEN		512
#define MAX_IP_EXT_LEN			2048
#define MAX_FTP_EXT_LEN			2048
#define MAX_EMAIL_EXT_LEN		2048
#define MAX_DEVICE_IP_LEN		32
#define MAX_LOGGING_TEXT_SIZE	256

#define LOGGING_ID_START 0XA0
#define ID_HTTP_SEND      (LOGGING_ID_START | 0X01)
#define ID_TCP_SERVER     (LOGGING_ID_START | 0X02)
#define ID_EVENT_SERVER   (LOGGING_ID_START | 0X03)
#define ID_BOA            (LOGGING_ID_START | 0X04)
#define ID_AV_SERVER      (LOGGING_ID_START | 0X05)
#define ID_SYSTEM_SERVER  (LOGGING_ID_START | 0X05)
////////////////////////////////////////////////////////////////////////////////
// Event Server type define
#define LOGGING_EVT_RECV_IVS_RESULT         0XA0
#define LOGGING_EVT_START_CHECK_DATABASE_ID 0XA1
#define LOGGING_EVT_END_CHECK_DATABASE_ID   0XA2
#define LOGGING_EVT_START_CHECK_WHITELIST   0XA3
#define LOGGING_EVT_END_CHECK_WHITELIST     0XA4
#define LOGGING_EVT_RECV_RS485_DATA         0XA5
#define LOGGING_EVT_SEND_RS485_DATA         0XA6
#define LOGGING_EVT_RECV_TTMSG              0XA7
#define LOGGING_EVT_OPEND_IO                0XA8
#define LOGGING_BOA_RECV_SDK_TRIGGER        0xA0
#define LOGGING_BOA_SEND_SYS_TRIGGER        0xA1
#define LOGGING_HTTP_RECV_AVS_ONPLATE       0xA0
#define LOGGING_HTTP_SEND_SERVER            0xA1
#define LOGGING_HTTP_RECV_SERVER            0xA2
#define LOGGING_HTTP_WRITE_GPIO             0xA3

#define DEVICEREG_DATA_LEN_MAX				32
#define MAX_BUFFER_SIZE               256 * 1024

typedef struct UserGetImgInfo {
	int datasize;
	int curpose;
	char pdata[MAX_BUFFER_SIZE];
  std::string posturl;
} UserGetImgInfo;

typedef struct DeviceRegData {
	std::string device_name;
	std::string ipaddr;
	std::string port;
	std::string user_name;
	std::string pass_wd;
	std::string serialno;
	std::string channel_num;
} DeviceRegData;


typedef struct VZ_CenterServer_Net {
  char hostname[MAX_HOSTNAME_LEN];
  int port;
  unsigned char enable_ssl;
  int ssl_port;
  int	http_timeout;
} VZ_CenterServer_Net;

typedef struct VZ_CenterServer_DeviceReg {
  __u8 enable;
  char uri[MAX_URI_LEN];
} VZ_CenterServer_DeviceReg;

typedef struct VZ_CenterServer_PlateResult {
  __u8 enable;
  char uri[MAX_URI_LEN];
  __u8 plateResultLevel;
  __u8 is_send_image;
  __u8 is_send_small_image;
} VZ_CenterServer_PlateResult;

typedef struct VZ_CenterServer_GioInAlarm {
  __u8 enable;
  char uri[MAX_URI_LEN];
} VZ_CenterServer_GioInAlarm;

typedef struct VZ_CenterServer_Alarm {
  VZ_CenterServer_PlateResult plate_result;
  VZ_CenterServer_GioInAlarm gio_in;
} VZ_CenterServer_Alarm;

typedef struct VZ_CenterServer_Serial {
  __u8 enable;
  char uri[MAX_URI_LEN];
} VZ_CenterServer_Serial;

typedef struct VZ_CenterServer {
  VZ_CenterServer_Net			net;
  VZ_CenterServer_DeviceReg	device_reg;
  VZ_CenterServer_Alarm		alarm;
  VZ_CenterServer_Serial		serial;
} VZ_CenterServer;

typedef struct OptimizeLogging {
#ifndef WIN32
  int64_t			millisec;
#else
  int				millisec;
#endif
  unsigned int    event_id;
  unsigned char   proc_id;
  unsigned short  type;
  unsigned char   msg_size;
  char            logging_msg[0];
} OptimizeLogging;

typedef enum {

  CENTERSERVER_NET = 0,
  CENTERSERVER_DEVICEREG,
  CENTERSERVER_PLATERESULT,
  CENTERSERVER_GIOINALARM,
  CENTERSERVER_SERIAL,
  CENTERSERVER_IPEXT,
  CENTERSERVER_OFFLINECHECK,
  CENTERSERVER_SERVERPOLL,
  HTTP_FTP_CONFIG,
  HTTP_EMAIL_CONFIG

} CENTERSERVER_DATATYPE;

typedef enum {

  HTTP_IVS_RESULT = 0,
  HTTP_SERIAL_DATA,
  HTTP_GIO_TRIGGER,
  HTTP_DEVICE_REG,
} HTTP_MSGTYPE;

typedef enum {
	REG_CANCEL_HEARTBEAT = 0,
	REG_COMMON_HEARTBEAT,
	REG_COMET_POLLING
} DEVICE_REG_STATUS;

#define IPEXTMAXLEN    2048

static unsigned char hexchars[] = "0123456789ABCDEF";
static char basis_64[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define CHAR64(c)  (((c) < 0 || (c) > 127) ? -1 : index_64[(c)])

static signed char index_64[128] = {
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 62, -1, -1, -1, 63,
  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -1, -1, -1, -1, -1, -1,
  -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14,
  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -1, -1, -1, -1, -1,
  -1, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -1, -1, -1, -1, -1
};

#define HEX_TO_DECIMAL(char1, char2)	\
    (((char1 >= 'A') ? (((char1 & 0xdf) - 'A') + 10) : (char1 - '0')) * 16) + \
    (((char2 >= 'A') ? (((char2 & 0xdf) - 'A') + 10) : (char2 - '0')))

typedef struct _IMAGE_DATA {
  char *imagedata;
  int imagelen;
  bool datagetend;
} IMAGE_DATA;

}
////////////////////////////////////////////////////////////////////////////////

const char POST_DATA_TEST[] =
  "{"
  "   \"AlarmInfoPlate\" : {"
  "      \"channel\" : 0,"
  "      \"deviceName\" : \"IVS\","
  "      \"ipaddr\" : \"192.168.4.71\","
  "      \"result\" : {"
  "         \"PlateResult\" : {"
  "            \"bright\" : 0,"
  "            \"carBright\" : 0,"
  "            \"carColor\" : 0,"
  "            \"colorType\" : 0,"
  "            \"colorValue\" : 0,"
  "            \"confidence\" : 0,"
  "            \"direction\" : 0,"
  "            \"imagePath\" : \"%2Fmmc%2FVzIPCCap%2F2016_09_03%2F1750152404__%CE%DE_.jpg\","
  "            \"license\" : \"_Œﬁ_\","
  "            \"location\" : {"
  "               \"RECT\" : {"
  "                  \"bottom\" : 0,"
  "                  \"left\" : 0,"
  "                  \"right\" : 0,"
  "                  \"top\" : 0"
  "               }"
  "            },"
  "            \"timeStamp\" : {"
  "               \"Timeval\" : {"
  "                  \"sec\" : 1472896215,"
  "                  \"usec\" : 245208"
  "               }"
  "            },"
  "            \"timeUsed\" : 0,"
  "            \"triggerType\" : 4,"
  "            \"type\" : 0"
  "         }"
  "      },"
  "      \"serialno\" : \"7a827083-62ad93cd\""
  "   }"
  "}";

#define sys_hwinfo  "sys_hwinfo"
#define Sys_Title   "Sys_Title"
#define Acount_Info "Acount_Info"
#define NetworkInterface_Cfg "NetworkInterface_Cfg"

#ifdef WIN32

const char DP_SERVER_IP_ADDR[]                      = "192.168.3.111";
const unsigned short DP_SERVER_PORT                 = 5291;

const char KVDB_SERVER_IP_ADDR[]                    = "192.168.3.111";
const unsigned short KVDB_SERVER_PORT               = 5299;

const char FILECACHED_IP_ADDR[]                     = "192.168.3.111";
const unsigned short FILECACHED_PORT                = 5320;

const char INTERNAL_IVSLOG_SERVER_BIND_ADDR[]       = "0.0.0.0";
const int INTERNAL_IVSLOG_SERVER_BIND_PORT       = 5577;

const char INTERNAL_IVSLOG_SERVER_ZMQ_ADDR[]        = "0.0.0.0";
const int INTERNAL_IVSLOG_SERVER_ZMQ_PORT        = 5567;

const char INTERNAL_WHITELIST_SERVER_BIND_ADDR[]    = "0.0.0.0";
const int INTERNAL_WHITELIST_SERVER_BIND_PORT    = 5579;

const char INTERNAL_WHITELIST_SERVER_ZMQ_ADDR[]     = "0.0.0.0";
const int INTERNAL_WHITELIST_SERVER_ZMQ_PORT     = 5566;
#else

const char DP_SERVER_IP_ADDR[]                      = "127.0.0.1";
const unsigned short DP_SERVER_PORT                 = 5291;

const char KVDB_SERVER_IP_ADDR[]                    = "127.0.0.1";
const unsigned short KVDB_SERVER_PORT               = 5299;

const char FILECACHED_IP_ADDR[]                     = "127.0.0.1";
const unsigned short FILECACHED_PORT                = 5320;

const char INTERNAL_IVSLOG_SERVER_BIND_ADDR[]       = "0.0.0.0";
const int INTERNAL_IVSLOG_SERVER_BIND_PORT       = 5577;

const char INTERNAL_IVSLOG_SERVER_ZMQ_ADDR[]        = "0.0.0.0";
const int INTERNAL_IVSLOG_SERVER_ZMQ_PORT        = 5567;

const char INTERNAL_WHITELIST_SERVER_BIND_ADDR[]    = "0.0.0.0";
const int INTERNAL_WHITELIST_SERVER_BIND_PORT    = 5579;

const char INTERNAL_WHITELIST_SERVER_ZMQ_ADDR[]     = "0.0.0.0";
const int INTERNAL_WHITELIST_SERVER_ZMQ_PORT     = 5566;

#endif

#define DEFAULT_FTP_SERVER_IP "192.168.1.1"
#define DEFAULT_FTP_ENABLE  0
#define DEFAULT_FTP_USER_NAME "ftpuser"
#define DEFAULT_FTP_PASSWORD  "test"
#define DEFAULT_FTP_FOLDER_NAME "default_folder"
#define DEFAULT_FTP_IMAGE_ACOUNT  100
#define DEFAULT_FTP_P_ID  100
#define DEFAULT_FTP_PORT_STR  21

#define DEFAULT_EMAIL_ENABLE  0
#define DEFAULT_EMAIL_SERVER_IP "smtp.163.com"
#define DEFAULT_EMAIL_USER_NAME "smtpuser"
#define DEFAULT_EMAIL_PASSWORD  "user"
#define DEFAULT_EMAIL_AUTHENTICATION  0
#define DEFAULT_EMAIL_SENDER_EMAIL  "smtpuser"
#define DEFAULT_EMAIL_RECEIVE_EMAIL "user@domain.com"
#define DEFAULT_EMAIL_CC_STR    "defaultccstr"
#define DEFAULT_EMAIL_SUBJECT   "TI"
#define DEFAULT_EMAIL_TEXT_STR  "teststr"
#define DEFAULT_EMAIL_ATTACHEMENTS  "attachements"
#define DEFAULT_EMAIL_ATTFILE_FORMAT  "attfileformat"
#define DEFAULT_EMAIL_ASMT_PATTACH  "asmtpattach"
#define DEFAULT_EMAIL_VIEW_STR  "viewstr"

#define DEFAULT_NET_HTTP_PORT 80
#define DEFAULT_HOST_NAME		"192.168.1.106"

#define DEFAULT_DEVICE_TITLE  "IVS"
#define DEFAULT_DEVICE_USER "admin"
#define DEFAULT_DEVICE_PASSWORD "admin"

#ifndef WIN32
#define DEFAULT_FTP_TEST_FILE_PATH "/tmp/app/exec/FtpTest.txt"
#else
#define DEFAULT_FTP_TEST_FILE_PATH "E://tftp/cmdline.txt"
#endif
#define DEFAULT_REG_URI         "/devicemanagement/php/receivedeviceinfo.php"
#define DEFAULT_PLATE_POST_URI  "/devicemanagement/php/plateresult.php"
#define DEFAULT_GPIO_POST_URL   "/devicemanagement/php/gio.php"
#define DEFAULT_SERIAL_POST_URL "/devicemanagement/php/serial.php"

const std::string JSON_TYPE = "type";
const std::string JSON_STATE = "state";
const std::string JSON_ERROR_MSG = "error_msg";
const std::string JSON_BODY = "body";
const std::string ERROR_DATA = "errorreqdata";
const std::string DEFAULT_BODY_DATA = "NullBodyData";
const std::string ERROR_MSG_SUCCESS = "success";
const std::string WRONG_REQ_TYPE = "error type";
const std::string WRONG_OPEN_TEST_FILE = "fopen test file failure";
const std::string WRONG_GET_LOCAL_TIME = "get local time failure";


/********************************* type ∂®“Â **********************************/

const std::string SET_FTP_ENABLE = "set_ftp_enable";
const std::string SET_FTP_SERVER_IP = "set_ftp_server_ip";
const std::string SET_FTP_USER_NAME = "set_ftp_user_name";
const std::string SET_FTP_PASSWORD = "set_ftp_password";
const std::string SET_FTP_FOLDER_NAME = "set_ftp_folder_name";
const std::string SET_FTP_IMAGE_ACOUNT = "set_ftp_image_acount";
const std::string SET_FTP_P_ID = "set_ftp_p_id";
const std::string SET_FTP_PORT_STR = "set_ftp_port_str";
const std::string GET_FTP_CONFIG = "get_ftp_cfg";
const std::string SET_FTP_CONFIG = "set_ftp_cfg";

const std::string SEND_FTP_TEST_FILE = "send_ftp_test_file";

const std::string GET_EMAIL_CONFIG = "get_email_cfg";
const std::string SET_EMAIL_CONFIG = "set_email_cfg";
const std::string SET_EMAIL_SERVER_IP = "set_email_server_ip";
const std::string SET_EMAIL_USER_NAME = "set_email_user_name";
const std::string SET_EMAIL_PASSWORD = "set_email_password";
const std::string SET_EMAIL_AUTHENTICATION = "set_email_authentication";
const std::string SET_EMAIL_SENDER_EMAIL = "set_email_sender_email";
const std::string SET_EMAIL_RECEIVE_EMAIL = "set_email_receive_email";
const std::string SET_EMAIL_CC_STR = "set_email_cc_str";
const std::string SET_EMAIL_SUBJECT = "set_email_subject";
const std::string SET_EMAIL_TEXT_STR = "set_email_text_str";
const std::string SET_EMAIL_ATTACHMENTS = "set_email_attachments";
const std::string SET_EMAIL_VIEW_STR = "set_email_view_str";
const std::string SET_EMAIL_ASMT_PATTATH = "set_email_asmt_pattath";
const std::string SET_EMAIL_ATTFILE_FORMAT = "set_email_attfile_format";

/******************************************************************************/

const char JSON_RESPONSE_ALARMINFOPLATE[] = "Response_AlarmInfoPlate";
const char JSON_INFO[] = "info";
const char JSON_SNAP_IMAGE_RELATIVE_URL[] = "snapImageRelativeUrl";
const char JSON_SNAP_IMAGE_ABSOLUTELY_URL[] = "snapImageAbsolutelyUrl";
const char JSON_MANUAL_TRIGGER[] = "manualTrigger";
const char JSON_SERIAL_DATA[] = "serialData";
const char JSON_SERIAL_DATA_HEAD[] = "SerialData";
const char JSON_SERIAL_CHANNEL[] = "serialChannel";
const char JSON_IO_CHANNEL_NUM[] = "channelNum";
const char JSON_DATA[] = "data";
const char JSON_DATA_LEN[] = "dataLen";
const char JSON_DEVICE_NAME[] = "device_name";
const char JSON_DEVICENAME[] = "deviceName";
const char JSON_IP_ADDRESS[] = "ipaddr";
const char JSON_PORT[] = "port";
const char JSON_USERNAME[] = "user_name";
const char JSON_PASSWORD[] = "pass_wd";
const char JSON_SERIALNO[] = "serialno";
const char JSON_CHANNEL_NUM[] = "channel_num";
const char JSON_CHANNEL[] = "channel";
const char JSON_LICENSE[] = "license";
const char JSON_IP_EXT[] = "IpExt";
const char JSON_IP_EXT_IN[] = "ipext";
const char JSON_DEVICE[] = "device";
const char JSON_NET[] = "net";
const char JSON_CENTER_SERVER[] = "centerserver";
const char JSON_HOST_NAME[] = "hostname";
const char JSON_ENABLE_SSL[] = "enable_ssl";
const char JSON_SSL_PORT[] = "ssl_port";
const char JSON_HTTP_TIMEOUT[] = "http_timeout";
const char JSON_ENABLE_DEVICEREG[] = "enable_device_reg";
const char JSON_DEVICEREG_URI[] = "device_reg_uri";
const char JSON_ALARM_ENABLE[] = "alarm_enable";
const char JSON_ALARM_URI[] = "alarm_uri";
const char JSON_PLATERESULT_ENABLE[] = "plateResultLevel";
const char JSON_IS_SEND_IMAGE[] = "is_send_image";
const char JSON_IS_SEND_SMALLIMAGE[] = "is_send_small_image";
const char JSON_GIO_ENABLE[] = "gio_enable";
const char JSON_GIO_URI[] = "gio_uri";
const char JSON_SERIAL_ENABLE[] = "serial_enable";
const char JSON_SERIAL_URI[] = "serial_uri";
const char JSON_COLOR_VALUE[] = "colorValue";
const char JSON_CONFIDENCE[] = "confidence";
const char JSON_BRIGHT[] = "bright";
const char JSON_CAR_BRIGHT[] = "carBright";
const char JSON_CAR_COLOR[] = "carColor";
const char JSON_COLOR_TYPE[] = "colorType";
const char JSON_DIRECTION[] = "direction";
const char JSON_LEFT[] = "left";
const char JSON_TOP[] = "top";
const char JSON_RIGHT[] = "right";
const char JSON_BOTTOM[] = "bottom";
const char JSON_RECT[] = "RECT";
const char JSON_LOCATION[] = "location";
const char JSON_TIMEUSED[] = "timeUsed";

const char JSON_SEC[] = "sec";
const char JSON_USEC[] = "usec";
const char JSON_DECYEAR[] = "decyear";
const char JSON_DECMON[] = "decmon";
const char JSON_DECDAY[] = "decday";
const char JSON_DECHOUR[] = "dechour";
const char JSON_DECMIN[] = "decmin";
const char JSON_DECSEC[] = "decsec";
const char JSON_TIMEVAL[] = "Timeval";
const char JSON_TIMESTAMP[] = "timeStamp";
const char JSON_TRIGGERTYPE[] = "triggerType";

const char JSON_TRIGGERPORT[] = "port";
const char JSON_TRIGGERIMAGE[] = "TriggerImage";
const char JSON_IMAGEFILE[] = "imageFile";
const char JSON_IMAGEFILE_BASE64_LEN[] = "imageFileBase64Len";
const char JSON_IMAGEFILE_LEN[] = "imageFileLen";
const char JSON_BEFORE_BASE64_IMAGEFILE_LEN[] = "beforeBase64imageFileLen";
const char JSON_IMAGEPATH[] = "imagePath";
const char JSON_IMAGEFRAGMENT_FILE[] = "imageFragmentFile";
const char JSON_IMAGEFRAGENMENT_FILELEN[] = "imageFragmentFileLen";
const char JSON_BEFORE_BASE64_IMAGEFRAGENMENT_FILELEN[] = "imageBeforeBase64FragmentFileLen";
const char JSON_PLATE_RESULT[] = "PlateResult";
const char JSON_RESULT[] = "result";
const char JSON_ALARMINFO_PLATE[] = "AlarmInfoPlate";
const char JSON_VALUE[] = "value";
const char JSON_SOURSE[] = "source";
const char JSON_TRIGGER_RESULT[] = "TriggerResult";
const char JSON_ALARM_GIOIN[] = "AlarmGioIn";

const char JSON_FTP_URL[] = "ftpurl";
const char JSON_FTP_USER[] = "ftpuser";
const char JSON_FTP_PASSWORD[] = "ftppassword";
const char JSON_FTP_FILE_SIZE[] = "ftpfilesize";
const char JSON_FTP_FILE_DATA[] = "ftpfiledata";

const char SYS_FILE[] =	"/mnt/usr/sysenv.cfg";
#ifdef WIN32
const char SYS_JSON_FILE[] = "F://softfile/sysenv.jc";
#else
const char SYS_JSON_FILE[] = "/mnt/usr/sysenv.jc";
#endif


const std::string WIFIINFO_STR = "wifiinfo";
const std::string SYSINFO_STR = "sysinfo";
const std::string VERSION_STR = "version";
const std::string DEVICE = "device";
const std::string ACOUNTS = "acounts";
const std::string USER = "user";
const std::string PASSWORD = "password";
const std::string AUTHORITY = "authority";
const std::string DEV_TYPE = "device_type";
const std::string NET = "net";
const std::string IP_IN_ADDR = "ip";
const std::string NETMASK_IN_ADDR = "netmask";
const std::string GATEWAY_IN_ADDR = "gateway";
const std::string DNS_IN_ADDR = "dns";
const std::string IP2_IN_ADDR = "ip2";
const std::string NETMASK2_IN_ADDR = "netmask2";
#ifdef WIFI_LAN
extern WifiInfo gWifiInfo;
const std::string WIFI_CFG_ESSID = "essid";
const std::string WIFI_CFG_KEY = "key";
const std::string WIFI_CFG_WEPKEYID = "wepkeyid";
const std::string WIFI_CFG_KEYSIZE = "keysize";
const std::string WIFI_CFG_AUTHMODE = "authmode";
const std::string WIFI_CFG = "wificfg";
const std::string WIFIIP_IN_ADDR = "wifiip";
const std::string WIFIIP_NETMASK_ADDR = "wifinetmask";
const std::string WIFIIP_GATEWAY_ADDR = "wifigateway";
const std::string WIFIIP_TYPE = "wifiiptype";
#endif
const std::string HTTP_PORT = "http_port";
const std::string HTTPS_PORT = "https_port";
const std::string DHCP_ENABLE = "dhcp_enable";
const std::string DHCP_CONFIG = "dhcp_config";
const std::string NTP_TIMEZONE = "ntp_timezone";
const std::string TIME_FORMAT = "time_format";
const std::string DAYLIGHT_TIME = "daylight_time";
const std::string MAC_STR = "MAC";
const std::string MPEG4_RESOLUTION = "mpeg4resolution";
const std::string MPEG42_RESOLUTION = "mpeg42resolution";
const std::string LIVE_RESOLUTION = "liveresolution";
const std::string MPEG4_QUALITY = "mpeg4quality";
const std::string SUPPORT_MPEG4 = "supportmpeg4";
const std::string IMAGE_FORMAT = "imageformat";
const std::string IMAGE_SOURCE = "imagesource";
const std::string DEFAULT_STORAGE = "defaultstorage";
const std::string DEFAULT_CARDGETHTM = "defaultcardgethtm";
const std::string BRAND_URL = "brandurl";
const std::string BRAND_NAME = "brandname";
const std::string SUPPORT_TSTAMP = "supporttstamp";
const std::string SUPPORT_MOTION = "supportmotion";
const std::string SUPPORT_WIRELESS = "supportwireless";
const std::string SERVICES_FTPCLIENT = "serviceftpclient";
const std::string SERVICES_MTPCLIENT = "servicesmtpclient";
const std::string SERVICE_PPPOE = "servicepppoe";
const std::string SERVICES_NTPCLIENT = "servicesntpclient";
const std::string SERVICE_DDNSCLIENT = "serviceddnsclient";
const std::string SUPPORT_MASKAREA = "supportmaskarea";
const std::string MAX_CHANNEL_NUM = "maxchannel";
const std::string SUPPORT_RS485 = "supportrs485";
const std::string SUPPORT_RS232 = "supportrs232";
const std::string LAYOUT_NUM = "layoutnum";
const std::string SUPPORT_MUI = "supportmui";
const std::string MUI_STR = "mui";
const std::string SUPPORT_SEQUENCE = "supportsequence";
const std::string QUADMODE_SELECT = "quadmodeselect";
const std::string SERVICE_IPFILTER = "serviceipfilter";
const std::string OEM_FLAG0 = "oemflag0";
const std::string SUPPORT_DNCONTROL = "supportdncontrol";
const std::string SUPPORT_AVC = "supportavc";
const std::string SUPPORT_AUDIO = "supportaudio";
const std::string SUPPORT_PTZPAGE = "supportptzpage";
const std::string MULTICCAST_ENABLE = "multicast_enable";
const std::string RTSP_PORT = "rtsp_port";

const std::string UPNP_PORTMAP = "upnp_portmap";
const std::string DDNS_STR = "ddns";
const std::string G3_CONFIG = "g3_config";
const std::string FTP_CONFIG = "ftp_config";
const std::string SMTP_CONFIG = "smtp_config";
const std::string CENTER_SERVER = "centerserver";
const std::string OPEN_VPN  = "openvpn";
const std::string NTP_STR = "ntp";

const std::string UPNPORTMAP_ENABLE = "upnpportmap_enable";
const std::string UPNP_HTTP_PORT = "upnp_http_port";
const std::string UPNP_RTSP_PORT = "upnp_rtsp_port";

const std::string DDNS_ENABLE = "ddns_enable";
const std::string DDNS_BRANDURL = "ddns_brandurl";
const std::string DDNS_NAME = "ddns_name";
const std::string DDNS_USER = "ddns_user";
const std::string DDNS_PASS = "ddns_pass";

const std::string ENABLE_3G = "enable_3g";
const std::string AUTOSTART_3G = "autostart_3g";
const std::string TYPE_3G = "type_3g";
const std::string USER_3G = "user_3g";
const std::string PASSWORD_3G = "password_3g";
const std::string DEFAULT_ASYNCMAP_3G = "default_asyncmap_3g";

const std::string SERVIER_IP = "servier_ip";
const std::string USER_NAME = "username";
const std::string FOLDER_NAME = "foldername";
const std::string IMAGE_ACOUNT = "image_acount";
const std::string P_ID = "pid";
const std::string PORT_STR = "port";

const std::string AUTHENTICATION = "authentication";
const std::string SENDER_EMAIL = "sender_email";
const std::string RECEIVER_EMAIL = "recerver_email";
const std::string CC_STR = "CC";
const std::string SUBJECT = "subject";
const std::string TEXT_STR = "text";
const std::string ATTACHMENTS = "attachments";
const std::string VIEW_STR = "view";
const std::string ASMT_PATTACH = "asmtpattach";
const std::string ATTFILE_FORMAT = "attfileformat";

const std::string HOST_NAME = "hostname";
const std::string ENABLE_SSL = "enable_ssl";
const std::string SSL_PORT = "ssl_port";
const std::string HTTP_TIMEOUT = "HttpTimeOut";
const std::string DEVICE_REG_URI = "device_reg_uri";
const std::string ENABLE_DEVICE_REG = "enable_device_reg";
const std::string ALARM_URI = "alarm_uri";
const std::string ALARM_ENABLE = "alarm_enable";
const std::string PLATE_RESULT_LEVEL = "plateResultLevel";
const std::string BSEND_IMAGE = "bSendImage";
const std::string BSEND_SMALL_IMAGE = "bSendSmallImage";
const std::string SERIAL_URI = "serial_uri";
const std::string SERIAL_ENABLE = "serial_enable";
const std::string GIO_URI = "gio_uri";
const std::string GIO_ENABLE = "gio_enable";


const std::string SERVER_NAME = "server_name";
const std::string FREQUENCY = "frequency";
const std::string ENABLE_STR = "enable";
const std::string USED_STR = "used";
const std::string VA_CFG = "VACfg";
const std::string MEDIA_STR = "media";

const std::string CHANNEL_NAME = "channel_name";
const std::string SOURCE_TOKEN_FLG = "source_token_flg";
const std::string SOURCE_TOKEN_STRUCT = "source_token_struct";
const std::string SOURCE_TOKEN = "source_token";
const std::string MAIN_STREAM = "main_stream";
const std::string SUB_STREAM = "sub_stream";
const std::string ALARM_STREAM = "alarm_stream";
const std::string JPEG_STREAM = "jpeg_stream";
const std::string USER_COUNT = "usercount";
const std::string AUDIO_CAPTURE_ENABLE = "audio_capture_enable";
const std::string STREAM_SOURCE = "stream_source";
const std::string OSD_STR = "osd";

const std::string IS_SUPPORTED = "is_supported";
const std::string NBITS_ENCODE_TYPE_SUPPORT = "nbitsEncodeTypeSupport";
const std::string NBITS_RESO_TYPE_SUPPORT = "nbitsResoTypeSupport";
const std::string CODEC = "codec";

const std::string NRATE_CONTROL = "nRateControl";
const std::string NIMAGE_QUALITY = "nImageQuality";
const std::string NBIT_RATE = "nBitrate";
const std::string NVIDEO_SIZEXY = "nVideoSizeXY";
const std::string NFRAME_RATE = "nFrameRate";
const std::string NENCODE_TYPE = "nEncodeType";

const std::string NJPEG_QUEALITY = "nJpegQuality";

const std::string N_DYNAMIC_RATIO = "nDynamicratio";
const std::string N_FRQUENCY = "nFrquency";
const std::string N_BRIGHTNESS = "nBrightness";
const std::string N_CONTRAST = "nContrast";
const std::string N_SATURATION = "nSaturation";
const std::string N_HUE = "nHue";
const std::string N_SHARPNESS = "nSharpness";
const std::string N_SHUTTER = "nShutter";
const std::string N_GAIN = "nGain";
const std::string N_DENOISE = "nDenoise";
const std::string N_FLIP = "nFlip";
const std::string N_NIGHTMODE = "nNightmode";
const std::string N_LEDLIGHT = "nLedLight";
const std::string N_LEDLIGHTMAX = "nLedLightMax";
const std::string N_LEDCONTROLMODE = "nLedControlMode";

const std::string DSTAMP_ENABLE = "dstampenable";
const std::string TSTAMP_ENABLE = "tstampenable";
const std::string NLOGO_ENABEL = "nLogoEnable";
const std::string NTEXT_ENABLE = "nTextEnable";
const std::string DATE_FORMAT = "dateformat";
const std::string TSTAMP_FORMAT = "tstampformat";
const std::string NDETAIL_INFO = "nDetailInfo";
const std::string DATE_POSITION = "dateposition";
const std::string TIME_POSITION = "timeposition";
const std::string NLOGO_POSITION = "nLogoPosition";
const std::string NTEXT_POSITION = "nTextPosition";
const std::string OVER_LAYTEXT = "overlaytext";

const std::string PTZ_STR = "ptz";
const std::string PTZ_TYPE = "ptz_type";
const std::string PTZ_PRESET = "ptz_preset";
const std::string PTZ_CRUISE = "ptz_cruise";

const std::string PTZ_COMM = "PtzComm";
const std::string PTZ_ADDRESS = "PtzAddress";
const std::string PTZ_BAUDRATE = "PtzBaudRate";
const std::string PTZ_PARITY = "PtzParity";
const std::string PTZ_DATABITS = "PtzDataBits";
const std::string PTZ_STOPBIT = "PtzStopBit";
const std::string PTZ_TYPE_STR = "Type";

const std::string PRESET_UNIT = "PresetUnit";
const std::string ENABLED_STR = "Enabled";
const std::string NAME_STR = "Name";

const std::string PTZ_CRUISE_ENABLE = "ptz_cruise_enable";
const std::string PTZ_CRUISE_SETS = "ptz_cruise_sets";

const std::string CRUISE_METHOD = "cruise_method";
const std::string PTZ_CRUISE_POINTS = "ptz_cruise_points";
const std::string SCHEDULE = "schedule";

const std::string PRESET_NUM = "PresetNum";
const std::string T_DURATION = "tDuration";
const std::string T_START = "tStart";

const std::string N_HOUR = "nHour";
const std::string N_MIN = "nMin";
const std::string N_SEC = "nSec";

const std::string DAY_ENABLE = "day_enable";
const std::string START_POS = "start_pos";
const std::string T_END = "tEnd";


const std::string VZMOTION_CONFIG = "vzmotion_config";
const std::string OUTPUT_MODEL = "outputmodel";

const std::string RECORD_CONFIG = "record_config";
const std::string N_RECNUM = "nRecNum";
const std::string STRU_RECORDING = "struRecording";

const std::string B_CHANNELNUM = "bChannelNum";
const std::string B_ENABLE = "bEnable";
const std::string STR_NAME = "strName";
const std::string E_RECID = "eRecID";
const std::string N_TRACKNUM = "nTrackNum";
const std::string STRU_TRACK = "struTrack";

const std::string E_FRAMEFMT = "eFrameFmt";
const std::string E_RECTYPE = "eRecType";

const std::string STORAGE = "storage";
const std::string RECORD_PLACE = "RecordPlace";
const std::string SCHEDULE_JPGPLACE = "ScheduleJpgPlace";
const std::string JPG_INTERVAL = "JpgInterval";
const std::string ENABLE_SCHEDULEJPG = "EnableScheduleJpg";
const std::string ENABLE_RECORD = "EnableRecord";

const std::string SOURCE_CONFIG = "source_config";
const std::string ENABLE_ALG = "enableAlg";
const std::string USER_ID = "userID";
const std::string ALG_ID_STR = "algId";
const std::string STREAM_TYPE = "streamType";

const std::string ALARM_SHOT = "alarmshot";
const std::string B_ENABLE_SHOT = "bEnableShot";
const std::string B_ENABLE_FTPUPDATE = "bEnableFtpUpdate";
const std::string B_ENABLE_MAILUPDATE = "bEnableMailUpdate";
const std::string B_ENABLE_STOREHARDWARE = "bEnableStoreHardware";
const std::string PLACE_STR = "place";
const std::string JPG_MAXCOUNT = "jpgMaxCount";


const std::string OLD_CFGS = "old_cfgs";
const std::string AUDIO_CONFIG = "audio_config";
const std::string CODEC_ADVCONFIG = "codec_advconfig";
const std::string CODEC_ROICONFIG = "codec_roiconfig";
const std::string STREAM_CONFIG = "stream_config";
const std::string MOTION_CONFIG = "motion_config";
const std::string LAN_CONFIG = "lan_config";
const std::string T_CURLOG = "tCurLog";

const std::string AUDIO_ON = "audioON";
const std::string AUDIO_ENABLE = "audioenable";
const std::string AUDIO_MODE = "audiomode";
const std::string AUDIO_INVOLUME = "audioinvolume";
const std::string CODEC_TYPE_STR = "codectype";
const std::string SAMPLE_RATE = "samplerate";
const std::string BIT_RATE = "bitrate";
const std::string ALARM_LEVEL = "alarmlevel";
const std::string AUDIO_OUTVOLUME = "audiooutvolume";

const std::string IP_RATIO = "ipRatio";
const std::string FI_FRAME = "fIframe";
const std::string QP_INIT = "qpInit";
const std::string QP_MIN = "qpMin";
const std::string QP_MAX = "qpMax";
const std::string ME_CONFIG = "meConfig";
const std::string PACKET_SIZE = "packetSize";
const std::string DUMMY = "dummy";

const std::string NUM_ROI = "numROI";
const std::string ROI_STR = "roi";

const std::string START_X = "startx";
const std::string START_Y = "starty";
const std::string WIDTH_STR = "width";
const std::string HEIGHT_STR = "height";

const std::string PORT_NUM = "portnum";
const std::string PORT_NAME = "portname";

const std::string MOTION_ENABLE = "motionenable";
const std::string MOTION_CENABLE = "motioncenable";
const std::string MOTION_LEVEL = "motionlevel";
const std::string MOTION_CVALUE = "motioncvalue";
const std::string MOTION_BLOCK = "motionblock";

const std::string TITLE_STR = "title";
const std::string N_WHITE_BALANCE = "nWhiteBalance";
const std::string N_DAY_NIGHT = "nDayNight";
const std::string N_HDR_MODE = "nHdrMode";
const std::string N_TVCABLE  = "nTVcable";
const std::string N_BINNING = "nBinning";
const std::string N_BACKLIGHT_CONTROL = "nBacklightControl";
const std::string N_BACKLIGHT = "nBackLight";
const std::string LOST_ALARM = "lostalarm";
const std::string N_STREAM_TYPE = "nStreamType";
const std::string N_VIDEO_CODECMODE = "nVideocodecmode";
const std::string N_VIDEO_COMBO = "nVideocombo";
const std::string N_VIDEO_CODECRES = "nVideocodecres";
const std::string MPEG41_XSIZE = "Mpeg41Xsize";
const std::string MPEG41_YSIZE = "Mpeg41Ysize";
const std::string MPEG42_XSIZE = "Mpeg42Xsize";
const std::string MPEG42_YSIZE = "Mpeg42Ysize";
const std::string JPEG_XSIZE = "JpegXsize";
const std::string JPEG_YSIZE = "JpegYsize";
const std::string AVC1_XSIZE = "Avc1Xsize";
const std::string AVC1_YSIZE = "Avc1Ysize";
const std::string AVC2_XSIZE = "Avc2Xsize";
const std::string AVC2_YSIZE = "Avc2Ysize";
const std::string SUPPORT_STREAM2 = "Supportstream2";
const std::string SUPPORT_STREAM3 = "Supportstream3";
const std::string SUPPORT_STREAM4 = "Supportstream4";
const std::string SUPPORT_STREAM5 = "Supportstream5";
const std::string SUPPORT_STREAM6 = "Supportstream6";
const std::string N_ALARM_DURATION = "nAlarmDuration";
const std::string N_AEW_SWITCH = "nAEWswitch";
const std::string N_NF_MODE = "nfMode";
const std::string N_NF_TSENABLE = "nfTsEnable";
const std::string GIOIN_ENABLE = "gioinenable";
const std::string GIOIN_TYPE = "giointype";
const std::string GIOOUT_ENABLE = "giooutenable";
const std::string GIOOUT_TYPE = "gioouttype";
const std::string GIODIRECT_OUT0 = "giodirectout0";
const std::string AVI_DURATION = "aviduration";
const std::string AVI_FORMAT = "aviformat";
const std::string ALARM_STATUS = "alarmstatus";
const std::string MIRROR = "mirror";
const std::string ADVANCE_MODE = "AdvanceMode";
const std::string M41S_FEATURE = "M41SFeature";
const std::string M42S_FEATURE = "M42SFeature";
const std::string JPGS_FEATURE = "JPGSFeature";
const std::string DEMO_CFG = "democfg";
const std::string REG_USR = "regusr";
const std::string OSD_STREAM = "osdstream";
const std::string OSD_WINNUM = "osdwinnum";
const std::string OSD_WIN = "osdwin";
const std::string OSD_TEXT = "osdtext";
const std::string N_AEWTYPE = "nAEWtype";
const std::string HISTOGRAM = "histogram";
const std::string GBCE = "gbce";
const std::string N_ALG_SENSITIVITY = "nAlgSensitivity";
const std::string MAX_EXPOSURE = "maxexposure";
const std::string MAX_GAIN = "maxgain";
const std::string EXPOSURE = "exposure";
const std::string RS485_CONFIG = "rs485config";
const std::string N_CLICKSNAP_FILENAME = "nClickSnapFilename";
const std::string N_CLICKSNAP_STORAGE = "nClickSnapStorage";
const std::string N_ENCRYPT_VIDEO = "nEncryptVideo";
const std::string N_LOCAL_DISPLAY = "nLocalDisplay";
const std::string N_ALARM_ENABLE = "nAlarmEnable";
const std::string N_EXT_ALARM = "nExtAlarm";
const std::string N_DARKBLANK = "nDarkBlank";
const std::string N_ALARM_AUDIOPLAY = "nAlarmAudioPlay";
const std::string N_ALARM_AUDIOFILE = "nAlarmAudioFile";
const std::string N_SCHEDULE_REPEATENABLE = "nSheduleRepeatEnable";
const std::string N_SCHEDULE_NUMWEEKS = "nScheduleNumWeeks";
const std::string N_SCHEDULE_INFINITEENABLE = "nScheduleInfiniteEnable";
const std::string ALARM_LOCAL = "alarmlocal";
const std::string RECORD_LOCAL = "recordlocal";
const std::string EXP_PRIORITY = "expPriority";
const std::string CODEC_TYPE1 = "codectype1";
const std::string CODEC_TYPE2 = "codectype2";
const std::string CODEC_TYPE3 = "codectype3";
const std::string A_SCHEDULES = "aSchedules";
const std::string SCHED_CURDAY = "schedCurDay";
const std::string SCHED_CURYEAR = "schedCurYear";
const std::string RELOAD_FLAG = "reloadFlag";

const std::string B_STATUS = "bStatus";
const std::string N_DAY = "nDay";

const std::string EVENT_STR = "event";
const std::string TIME_STR = "time";

const std::string TM_SEC = "tm_sec";
const std::string TM_MIN = "tm_min";
const std::string TM_HOUR = "tm_hour";
const std::string TM_MDAY = "tm_mday";
const std::string TM_MON = "tm_mon";
const std::string TM_YEAR = "tm_year";
const std::string TM_WDAY = "tm_wday";
const std::string TM_YDAY = "tm_yday";
const std::string TM_ISDST = "tm_isdst";


const std::string ONVIF_INFO = "onvif_info";
const std::string SERVICES = "services";
const std::string DEVICE_INFORMATION = "device_information";
const std::string TIME_CFG = "time_cfg";
const std::string SCOPES = "scopes";
const std::string PROFILES = "Profiles";
const std::string RECORDING_JOBS = "RecordingJobs";
const std::string RECORDINGS = "Recordings";
const std::string SUBSCRIPTIONS = "Subscriptions";
const std::string VIDEO_ANALYTICS = "VideoAnalytics";
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

////////////////////////////////////////////////////////////////////////////////
#endif //  HTTP_SENDER_BASE_BASEDEFINES_H_
