//�ڵ��߳��µ���
#ifndef _ONVIF_DISPATCHER_H_
#define _ONVIF_DISPATCHER_H_

#define IPC_DISP_FAILED     -1
#define IPC_DISP_SUCCESS    0

typedef int IPC_DISP_RET;


#ifdef __cplusplus
extern "C"
{
#endif
//�����ַ�����
IPC_DISP_RET OnvifDisp_Start();

//ֹͣ�ַ�����
void OnvifDisp_Stop();

int OnvifDisp_SendRequestSync(const char* msg_type,const char *buffer,int len,void *rbuffer,int rbuffer_len);

#ifdef __cplusplus
}
#endif
#endif
