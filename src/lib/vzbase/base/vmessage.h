/************************************************************************/
/* Author      : SoberPeng 2017-06-13
/* Description :
/************************************************************************/
#ifndef VMESSAGE_H
#define VMESSAGE_H

#define SW_VERSION_ "V100.00.00 "
#define HW_VERSION_ "V100.00.00 "

// DpClient_Init����dispatcher_server dp��ַ
#define DEF_DP_SRV_IP         "127.0.0.1"
#define DEF_DP_SRV_PORT       5291

// KvdbClient_Init����dispatcher_server kvdb��ַ
#define DEF_KVDB_SRV_IP       "127.0.0.1"
#define DEF_KVDB_SRV_PORT     5299

// WEB Server �����˿ں���ҳ�洢·��
#define DEF_WEB_SRV_PORT      8000
#ifdef WIN32
#define DEF_WEB_SRV_PATH      "c:\\tools\\web"
#define DEF_SYS_LOG_PATH      "c:\\tools\\web\log"
#else
#define DEF_WEB_SRV_PATH      "/mnt/web"
#define DEF_SYS_LOG_PATH      "/mnt/web/log"
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
} MSG_RET;
//////////////////////////////////////////////////////////////////////////
#define MSG_ADDR_CHANGE       "addr_change"       // ��ַ�ı�
#define MSG_TIME_CHANGE       "time_change"       // ʱ��ı�

#define MSG_GET_DEVINFO       "get_devinfo"       // ��ȡ�豸��Ϣ
#define MSG_SET_DEVINFO       "set_devinfo"       // �����豸��Ϣ

#define MSG_SET_HWCLOCK       "set_hwclock"       // ����Ӳ��ʱ��

#define MSG_GET_I_FRAME       "get_i_frame"       // ����I֡
#define MSG_GET_ENC_CFG       "get_enc_cfg"       // ��ȡ��������
#define MSG_SET_ENC_CFG       "set_enc_cfg"       // ���ñ�������

//////////////////////////////////////////////////////////////////////////
#define MSG_GET_IVAINFO       "get_ivainfo"       // ��ȡ�㷨��Ϣ
#define MSG_SET_IVAINFO       "set_ivainfo"       // �����㷨��Ϣ

#define MSG_RESET_COUNT       "reset_count"       // ���ü���

#define MSG_CATCH_EVENT       "catch_event"       // �㷨�¼�

#define MSG_GET_PCOUNTS       "get_pcounts"       // ��ȡ������
#define MSG_CLEAR_PCOUNT      "clear_pcount"      // ���pcount���ݿ�

#define MSG_SYSTEM_UPDATE     "system_update"     // ϵͳ����

//////////////////////////////////////////////////////////////////////////
#define KVDB_KEY_USER         "key_user"     // kvdb��ȡ�û�
#endif  // VMESSAGE_H
