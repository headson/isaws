/************************************************************************/
/* Author      : SoberPeng 2017-06-13
/* Description :
/************************************************************************/
#ifndef VMESSAGE_H
#define VMESSAGE_H

#define SW_VERSION_ "V100.00.00 "
#define HW_VERSION_ "V100.00.00 "

#define DEF_SHM_VIDEO_0       "/dev/shm/video_0"
#define DEF_SHM_VIDEO_0_SIZE  ((352*288*3)/2 + 1024)

#define DEF_SHM_AUDIO_0       "/dev/shm/audio_0"
#define DEF_SHM_AUDIO_0_SIZE  (1024)

// DpClient_Init����dispatcher_server dp��ַ
#define DEF_DP_SRV_IP         "127.0.0.1"
#define DEF_DP_SRV_PORT       5291

// KvdbClient_Init����dispatcher_server kvdb��ַ
#define DEF_KVDB_SRV_IP       "127.0.0.1"
#define DEF_KVDB_SRV_PORT     5299

// WEB Server �����˿ں���ҳ�洢·��
#define DEF_WEB_SRV_PORT      8000
#ifdef WIN32
#define DEF_WEB_SRV_PATH      "E:\\workspace\\git_work\\isaws\\src\\web"
#else
#define DEF_WEB_SRV_PATH      "mnt/etc/web"
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
  RET_SUCCESS       = 0,
  RET_FAILED        = -1000,
  RET_JSON_PARSE,
  RET_USERNAME,
  RET_PASSWORD,
} MSG_RET;
//////////////////////////////////////////////////////////////////////////

#define DP_SYS_CONFIG         "dp_sys_config"     // ϵͳ����

#define MSG_SYSC_GET_INFO     "get_devinfo"       // ��ȡ�豸��Ϣ
#define MSG_SYSC_SET_INFO     "set_devinfo"       // �����豸��Ϣ
#define MSG_SYSC_ADDR_CHANGE  "addr_change"       // ��ַ�ı�

#define MSG_SYSC_HWCLOCK      "set_hwclock"       // ����Ӳ��ʱ��
#define MSG_SYSC_TIME_CHANGE  "time_change"       // ʱ��ı�

#define DP_NET_MODIFY         "dp_net_modify"     // �����޸���Ϣ

//////////////////////////////////////////////////////////////////////////
#define DP_IVA_CONFIG         "dp_iva_config"     // �㷨����
#define MSG_IVAC_GET_INFO     "get_iva_info"      // 

#define DP_IVA_EVENT          "dp_iva_event"      // �㷨�¼�

//////////////////////////////////////////////////////////////////////////
#define KVDB_KEY_USER         "key_user"     // kvdb��ȡ�û�
#endif  // VMESSAGE_H
