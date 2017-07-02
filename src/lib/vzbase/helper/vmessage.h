/************************************************************************/
/* Author      : SoberPeng 2017-06-13
/* Description :
/************************************************************************/
#ifndef VMESSAGE_H
#define VMESSAGE_H

#define DEF_SHM_VIDEO_0       "/dev/shm/video_0"
#define DEF_SHM_VIDEO_0_SIZE  ((352*288*3)/2)

#define DEF_SHM_AUDIO_0       "/dev/shm/audio_0"
#define DEF_SHM_AUDIO_0_SIZE  1024

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
#define MSG_SYS_PARAM         "msg_sys_param"     // ϵͳ����

#define MSG_IVA_PARAM         "msg_iva_param"     // �㷨����
#define MSG_IVA_EVENT         "msg_iva_event"     // �㷨�¼�



#endif  // VMESSAGE_H
