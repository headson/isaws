/************************************************************************/
/* Author      : SoberPeng 2017-06-13
/* Description :
/************************************************************************/
#ifndef VMESSAGE_H
#define VMESSAGE_H

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
#define DEF_WEB_SRV_PATH      "c:/tools/web"

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
//////////////////////////////////////////////////////////////////////////

#define DP_SYS_CONFIG         "dp_sys_config"   // ϵͳ����

#define MSG_SYSC_GET_DEVINFO  "get_devinfo"     // ��ȡ�豸��Ϣ
#define MSG_SYSC_SET_DEVINFO  "set_devinfo"     // �����豸��Ϣ

#define MSG_SYSC_SET_HWCLOCK  "set_hwclock"     // ����Ӳ��ʱ��

#define DP_NET_MODIFY         "dp_net_modify"   // �����޸���Ϣ

//////////////////////////////////////////////////////////////////////////
#define DP_IVA_CONFIG         "dp_iva_config"   // �㷨����

#define DP_IVA_EVENT          "dp_iva_event"    // �㷨�¼�

//////////////////////////////////////////////////////////////////////////

#endif  // VMESSAGE_H
