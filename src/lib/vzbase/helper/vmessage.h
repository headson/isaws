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
#define DEF_WEB_SRV_PORT      "8000"
#define DEF_WEB_SRV_PATH      "c:/tools/web"

#define DEF_MCAST_IP          "228.5.6.2"
#define DEF_MCAST_DEV_PORT    20003     // �豸�����˿�,�޸�IP��ַ
#define DEF_MCAST_CLI_PORT    20004     // �ͻ��˼����˿�,��ȡ�豸��Ϣ

//////////////////////////////////////////////////////////////////////////
/*
�����Э��
{
  "type":"set_xxx",
  "body":{
    ......
  }
}

�ظ���Э��
{
  "type":"set_xxx",
  "state":200,
  "err_msg":"all done",
  "body":{
    ......
  }
}
body:�ڲ��Ǹ��ֲ�ͬ��Э��Ĳ�ͬ����
type:�Ǳ���ֵ������ͻظ��Ķ�Ӧ��Я�����ֵ
state:�ظ���״̬�룬�������������Ϊ200��Ҳ��һ������Ķ��塣������http�Ĵ��������ơ�
err_msg:��state��ֵ��Ϊ200������£�Ӧ�ý�error_msg��ֵ��ֵ���Ա��û��ܹ����õ����״�����ʲô�ط���
          �����������棬ֻ�ܹ���Ӣ�ģ����ܹ������ġ�

ע��:�����ֶζ�ΪСд
*/
#define MSG_TYPE              "type"
#define MSG_STATE             "state"
#define MSG_ERR_MSG           "err_msg"
#define MSG_BODY              "body"

#define MSG_TYPE_MAX          32

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
#define KVDB_
#endif  // VMESSAGE_H
