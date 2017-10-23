/************************************************************************/
/* Author      : SoberPeng 2017-06-13
/* Description :
/************************************************************************/
#ifndef VMESSAGE_H
#define VMESSAGE_H

#define SW_VERSION_ "V100.00.00 "
#define HW_VERSION_ "V100.00.00 "
#ifdef _WIN32
// DpClient_Init����dispatcher_server dp��ַ
#define DEF_DP_SRV_IP         "192.168.1.11"
#define DEF_DP_SRV_PORT       5291

// KvdbClient_Init����dispatcher_server kvdb��ַ
#define DEF_KVDB_SRV_IP       "192.168.1.11"
#define DEF_KVDB_SRV_PORT     5299

#else  // _LINUX
	
// DpClient_Init����dispatcher_server dp��ַ
#define DEF_DP_SRV_IP         "127.0.0.1"
#define DEF_DP_SRV_PORT       5291

// KvdbClient_Init����dispatcher_server kvdb��ַ
#define DEF_KVDB_SRV_IP       "127.0.0.1"
#define DEF_KVDB_SRV_PORT     5299
#endif

// WEB Server �����˿ں���ҳ�洢·��
#define DEF_WEB_SRV_PORT      80
#ifdef WIN32
#define DEF_WEB_SRV_PATH      "c:\\tools\\html\\htmldata"
#define DEF_SYS_LOG_PATH      "c:\\tools\\html\\htmldata\\log"
#define DEF_HARDWARE_FILE     "c:\\tools\\hardware.json"
#define DEF_SOFTWARE_FILE     "c:\\tools\\software.ver"
#define SYS_SYSTEM_CONFIG     "c:\\tools\\system.json"
#define DB_PCOUNT_FILEPATH    "c:\\tools\\pcount.db"
#define UPLINK_ADDR_CONFIG    "c:\\tools\\remote_address.json"
#define DEF_ALG_CONFIG_FILE   "c:\\tools\\iva.json"
#define DEF_AUX_CONFIG_FILE   "c:\\tools\\aux.json"
#define DEF_UPLOAD_FILENAME   "c:\\tools\\upload.tar"
#else
#define DEF_WEB_SRV_PATH      "/tmp/app/web/"
#define DEF_SYS_LOG_PATH      "/tmp/app/web/log"
#define SYS_SYSTEM_CONFIG     "/mnt/usr/system.json"
#define DEF_HARDWARE_FILE     "/etc/hardware.json"
#define DEF_SOFTWARE_FILE     "/mnt/app/exec/software.ver"
#define DB_PCOUNT_FILEPATH    "/mnt/usr/pcount.db"
#define UPLINK_ADDR_CONFIG    "/mnt/usr/remote_address.json"
#define DEF_ALG_CONFIG_FILE   "/mnt/usr/iva.json"
#define DEF_AUX_CONFIG_FILE   "/mnt/usr/aux.json"
#define DEF_UPLOAD_FILENAME   "/tmp/upload.tar"
#endif

#define DEF_MCAST_IP          "228.5.6.2"
#define DEF_MCAST_DEV_PORT    20003     // �豸�����˿�,�޸�IP��ַ
#define DEF_MCAST_CLI_PORT    20004     // �ͻ��˼����˿�,��ȡ�豸��Ϣ

//////////////////////////////////////////////////////////////////////////
/*
�����Э��
{
  "cmd":"xxx",
  "id":xxx,
  "body": {
  }
}
cmd  �ֶα�����Ϣ�����ͣ��ɷ������ͻ���Լ��������һ���ַ�������Ҫ������������
id   �ֶ���һ���ɿͻ��˷�����ֶΣ����ÿһ�����󣬶�������һ��Ψһ������id��idֻ�ܹ���һ�����֣�����˻Ὣid�ڻظ��з��أ����һ������û��id�ֶΣ������Ӧ��ֱ�ӷ���ʧ�ܵ����
body ���ɿͻ��˷��͵ľ�����������ݣ����ݲ�ͬ�������в�ͬ��`body`���ݣ�ԭ���ϣ�`body`��������ݲ�Ӧ���г���3���JSONǶ�ס�

�ظ���Э��
{
  "cmd":"xxx",
  "id":xxx,
  "state":200,
  "body": {
  }
}
ע��:�����ֶζ�ΪСд
*/
#define MSG_CMD               "cmd"
#define MSG_ID                "id"
#define MSG_STATE             "state"
#define MSG_BODY              "body"

#define MSG_CMD_SIZE          32

typedef enum _MSG_RET {
  RET_SUCCESS       = 200,
  RET_FAILED        = -1000,
  RET_JSON_PARSE,
  RET_ERROR_HDL,
  RET_USERNAME,
  RET_PASSWORD,
  RET_DP_REPLY_FAILED,
  RET_KVDB_READ_FAILED,
} MSG_RET;

//////////////////////////////////////////////////////////////////////////
#define DP_CMD_DEV            "DP_CMD_DEV"        // dev get or set
#define DP_CMD_NET            "DP_CMD_NET"        // net get or set
#define DP_CMD_TIME           "DP_CMD_TIME"       // time get or set
#define DP_CMD_ALG            "DP_CMD_ALG"        // alg get or set
#define DP_CMD_ENC            "DP_CMD_ENC"        // image & video & audio encode get or set
#define DP_CMD_REC            "DP_CMD_REC"        // record get or set
#define DP_CMD_BUS            "DP_CMD_BUS"        // business get or set
#define DP_CMD_EXT            "DP_CMD_EXT"        // extern device control

#define DP_TIME_CHANGE        "DP_TIME_CHANGE"    // boardcast time is changed

///DP_CMD_DEV/////////////////////////////////////////////////////////////
#define MSG_SYS_REBOOT        "sys_reboot"        // �����豸

#define MSG_GET_DEVINFO       "get_devinfo"       // ��ȡ�豸��Ϣ
#define MSG_SET_DEVINFO       "set_devinfo"       // �����豸��Ϣ

#define MSG_SET_DEVTIME       "set_devtime"       // �����豸ʱ��
#define MSG_GET_TIMEINFO      "get_timeinfo"      // ��ȡʱ����Ϣ
#define MSG_SET_TIMEINFO      "set_timeinfo"      // ����ʱ����Ϣ

///DP_CMD_NET/////////////////////////////////////////////////////////////
#define MSG_GET_NETWORK       "get_network"       // ��ȡ������Ϣ
#define MSG_SET_NETWORK       "set_network"       // ����������Ϣ

///DP_CMD_ENC/////////////////////////////////////////////////////////////
#define MSG_GET_I_FRAME       "get_i_frame"       // ����I֡
#define MSG_GET_STR_URL       "get_str_url"       // ��ȡ��URL
#define MSG_GET_VDO_ENC       "get_vdo_enc"       // ��ȡ��Ƶ����
#define MSG_SET_VDO_ENC       "set_vdo_enc"       // ������Ƶ����
#define MSG_GET_VDO_OSD       "get_vdo_osd"       // ������ƵOSD
#define MSG_SET_VDO_OSD       "set_vdo_osd"       // ������ƵOSD
#define MSG_VDO_ISP_CTRL      "vdo_isp_ctrl"      // ��ƵISP����

///DP_CMD_EXT/////////////////////////////////////////////////////////////
#define MSG_IRCUT_CTRLS       "ircut_ctrls"       // IRCUT����
#define MSG_REMOTE_5_IR       "remote_5_ir"       // Զ��5��IR

///DP_CMD_ALG/////////////////////////////////////////////////////////////
#define MSG_GET_ALGINFO       "get_alginfo"       // ��ȡ�㷨��Ϣ
#define MSG_SET_ALGINFO       "set_alginfo"       // �����㷨��Ϣ

#define MSG_RESET_PCNUM       "reset_pcnum"       // ���ü���
#define MSG_EVENT_RESULT      "event_result"      // �¼����

//////////////////////////////////////////////////////////////////////////
#define KVDB_HW_INFO          "hw_info"           // Ӳ����Ϣ
#define KVDB_NETWORK          "network"           // �������
#define KVDB_ENC_OSD          "enc_osd"           // ����OSD
#define KVDB_TIME_INFO        "time_info"         // ʱ����Ϣ
#define KVDB_USER_LIST        "user_list"         // ��¼�û�;username\password

#endif  // VMESSAGE_H
