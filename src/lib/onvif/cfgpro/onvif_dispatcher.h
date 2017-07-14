//在单线程下调用
#ifndef _ONVIF_DISPATCHER_H_
#define _ONVIF_DISPATCHER_H_

#define IPC_DISP_FAILED     -1
#define IPC_DISP_SUCCESS    0

typedef int IPC_DISP_RET;


#ifdef __cplusplus
extern "C"
{
#endif
//启动分发机制
IPC_DISP_RET OnvifDisp_Start();

//停止分发机制
void OnvifDisp_Stop();

int OnvifDisp_SendRequestSync(const char* msg_type,const char *buffer,int len,void *rbuffer,int rbuffer_len);

#ifdef __cplusplus
}
#endif
#endif
