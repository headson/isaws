/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : dispatch\kvdb C���Կͻ��˽ӿ�
************************************************************************/
#ifndef LIBDISPATCH_DPCLIENT_C_H_
#define LIBDISPATCH_DPCLIENT_C_H_

#ifdef WIN32
#define EXPORT_DLL _declspec(dllexport)
#else
#define EXPORT_DLL
#endif

#include "dispatcher/base/pkghead.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void* DpEvtService;
typedef void* DPPollHandle;
#define DP_POLL_HANDLE_NULL NULL

// �ڻص���,����ʹ��ͬһ��socket send\recv,��ɵݹ�evt loop
typedef void(*DpClient_MessageCallback)(
  DPPollHandle p_hdl, const DpMessage *dmp, void* p_usr_arg);

#define DP_CLIENT_DISCONNECT  101   // �Ͽ�����,��Ҫ����
typedef void(*DpClient_PollStateCallback)(
  DPPollHandle p_hdl, uint32 n_state, void* p_usr_arg);


typedef void* EventSignal;   // �ź���
// libevent����signal�¼�
typedef int (*Event_SignalCallback)(int         n_signal,
                                    short       events,
                                    const void *p_usr_arg);

/************************************************************************
*Description : ��ʼ��Dispatcher�ķ�����[��ַ:�˿�]
*Parameters  :
*Return      :
************************************************************************/
EXPORT_DLL void DpClient_Init(const char* ip_addr, unsigned short port);

EXPORT_DLL int  DpClient_Start(int new_thread);

/************************************************************************
*Description : ����Dispatcher�ͻ���
*Parameters  :
*Return      :
************************************************************************/
EXPORT_DLL void DpClient_Stop();

/************************************************************************
*Description : DpClient������Ϣ
*Parameters  : p_method[IN] ��Ϣ����
*              n_session_id[IN]
*              p_data[IN] ��������
*              n_data[IN] �������ݴ�С
*Return      : VZNETDP_FAILURE ʧ��; VZNETDP_SUCCEED �ɹ�
************************************************************************/
EXPORT_DLL int  DpClient_SendDpMessage(const char    *p_method,
                                       unsigned char  n_session_id,
                                       const char    *p_data,
                                       int            n_data);

/************************************************************************
*Description : DpClient��������
*Parameters  : p_method[IN] ���󷽷�
*              n_session_id[IN]
*              p_data[IN] ��������
*              n_data[IN] �������ݴ�С
*              p_callback[IN] �����ִ�ص�
*              p_user_arg[IN] �����ִ�ص�����
*              n_timeout[IN] �������ó�ʱʱ��
*Return      : VZNETDP_FAILURE ʧ��; VZNETDP_SUCCEED �ɹ�
************************************************************************/
EXPORT_DLL int DpClient_SendDpRequest(const char                *p_method,
                                      unsigned char              n_session_id,
                                      const char                *p_data,
                                      int                        n_data,
                                      DpClient_MessageCallback   p_callback,
                                      void                      *p_user_arg,
                                      unsigned int               n_timeout);

/************************************************************************
*Description : DpClient���ͻ�ִ
*Parameters  : p_method[IN] ��ִ��Ϣ����
*              n_session_id[IN] DpMessage->channel_id
*              n_message_id[IN] DpMessage->id
*              p_data[IN] ��������
*              n_data[IN] �������ݴ�С
*Return      : VZNETDP_FAILURE ʧ��; VZNETDP_SUCCEED �ɹ�
************************************************************************/
EXPORT_DLL int DpClient_SendDpReply(const char      *p_method,
                                    unsigned char    n_session_id,
                                    unsigned int     n_message_id,
                                    const char      *p_data,
                                    int              n_data);

///Poll///////////////////////////////////////////////////////////////////
/************************************************************************
*Description : ������ѯ���
*Parameters  : p_msg_cb[IN] ��Ϣ�ص�
*              p_msg_usr_arg[IN] ��Ϣ�ص�����
*              p_state_cb[IN] ״̬�ص�:����,����,��ע�������Ϣ
*              p_msg_usr_arg[IN] ״̬�ص�����
*              p_evt_loop[IN] EVT_LOOPָ��
*Return      : NULL ʧ��, !=NULL�ɹ�
************************************************************************/
EXPORT_DLL DPPollHandle DpClient_CreatePollHandle(
  DpClient_MessageCallback   p_msg_cb,
  void                      *p_msg_usr_arg,
  DpClient_PollStateCallback p_state_cb,
  void                      *p_state_usr_arg,
  DpEvtService               p_evt_service = NULL);

/************************************************************************
*Description : ������ѯ���,��ʱ���EVT_LOOPһ������,ע������ʹ�ô�EVT_LOOP�ĵط�
*Parameters  :
*Return      :
************************************************************************/
EXPORT_DLL void  DpClient_ReleasePollHandle(DPPollHandle p_poll_handle);

/************************************************************************
*Description : ��ѯ����;��DpClient_PollDpMessage���ش���ʱ,���ô˺�������������
*Parameters  : p_poll_handle[IN] ��ѯ���
*Return      : VZNETDP_FAILUREʧ�� VZNETDP_SUCCEED�ɹ�
************************************************************************/
EXPORT_DLL int DpClient_HdlReConnect(const DPPollHandle p_poll_handle);

/************************************************************************
*Description : ��ѯע�������Ϣ;
*              DpClient_HdlReConnect����ִ��,����������Ϣ��һֱ����
*Parameters  : p_poll_handle[IN]
*              a_method_set[IN] ��Ϣ����,ÿһ����Ϣ32Byte
*              n_method_set_cnt[IN] ��Ϣ����
*Return      : VZNETDP_FAILUREʧ�� VZNETDP_SUCCEED�ɹ�
************************************************************************/
EXPORT_DLL int   DpClient_HdlAddListenMessage(
  const DPPollHandle p_poll_handle,
  const char        *a_method_set[],
  unsigned int       n_method_set_cnt);

/************************************************************************
*Description : ��ѯȡ��������Ϣ
*Parameters  : p_poll_handle[IN]
*              a_method_set[IN] ��Ϣ����,ÿһ����Ϣ32Byte
*              n_method_set_cnt[IN] ��Ϣ����
*Return      : VZNETDP_FAILUREʧ�� VZNETDP_SUCCEED�ɹ�
************************************************************************/
EXPORT_DLL int   DpClient_HdlRemoveListenMessage(
  const DPPollHandle p_poll_handle,
  const char        *a_method_set[],
  unsigned int       n_method_set_cnt);

// ��ʹ��vzbase::Thread������ѯ��EVT_LOOPʱ,�ɲ����ô˺����ַ�
// �뾡��ʹ��vzbase::Thread::Current()->Run()���滻�˺���,
// ��Ϊ�߳̿�Ͷ����Ϣ��ʵ�ֶ�ʱ��
/************************************************************************
*Description : ��ѯ,��Ϣ�ַ�
*Parameters  : p_poll_handle[IN]
*              n_timeout[IN] ��ѯ��ʱʱ��
*Return      : VZNETDP_FAILUREʧ�� VZNETDP_SUCCEED�ɹ�
************************************************************************/
EXPORT_DLL int DpClient_PollDpMessage(
  const DPPollHandle         p_poll_handle,
  unsigned int               n_timeout);

/************************************************************************
*Description : ����ѯ�л�ȡEVT_LOOPָ��
*Parameters  : p_poll_handle[IN]
*Return      : NULL ʧ��, !=NULL�ɹ�
************************************************************************/
EXPORT_DLL DpEvtService DpClient_GetEvtLoopFromPoll(
  const DPPollHandle p_poll_handle);

///SIGNAL/////////////////////////////////////////////////////////////////
/************************************************************************
*Description : �����źż���
*Parameters  : p_evt_service[IN] �¼��ַ���ָ��
*              n_signal_no[IN] �ź� SIGINT\SIGTERM\SIGKILL
*              p_callback[IN] �źŴ���ʱ�ص�����
*              p_user_arg[IN] �źŴ��������ص������û�ָ��
*Return      : !=NULL �ɹ�, ==NULLʧ��
************************************************************************/
EXPORT_DLL EventSignal Event_CreateSignalHandle(
  const DpEvtService   p_evt_service,
  int                  n_signal_no,
  Event_SignalCallback p_callback,
  void                *p_user_arg);

/************************************************************************
*Description : �ͷ��źż���
*Parameters  : p_evt_sig[IN] �źż������
*Return      : 
************************************************************************/
EXPORT_DLL void Event_ReleaseSignalHandle(EventSignal p_evt_sig);

//////////////////////////////////////////////////////////////////////////
// GetKey callback function
typedef void(*Kvdb_GetKeyCallback)(const char *p_key,
                                   int         n_key,
                                   const char *p_value,
                                   int         n_value,
                                   void       *p_user_data);

// Return of Key Value database Interface
#define KVDB_RET_SUCCEED    0
#define KVDB_RET_FAILURE   -1

// Default kvdb server:port is 127.0.0.1:5299
EXPORT_DLL int Kvdb_Start(const char *server, unsigned short port);
EXPORT_DLL int Kvdb_Stop();

// SetKey, if the key not exist, then create the key
// Return KVDB_RET_FAILURE, KVDB_RET_SUCCEED
EXPORT_DLL int Kvdb_SetKey(const char *p_key, int n_key,
                           const char *p_value, int n_value);

/************************************************************************
*Description : ͨ����ֵ��ȡ����
*Parameters  : 
*Return      : >0 �ɹ�(��ȡ���ݳ���); KVDB_RET_FAILURE ʧ��
************************************************************************/
EXPORT_DLL int Kvdb_GetKey(const char *p_key, int n_key,
                           Kvdb_GetKeyCallback p_callback,
                           void *p_usr_arg);

/************************************************************************
*Description : ͨ����ֵ��ȡ����
*Parameters  :
*Return      : >0 �ɹ�(��ȡ���ݳ���); KVDB_RET_FAILURE ʧ��
************************************************************************/
EXPORT_DLL int Kvdb_GetKeyAbsolutely(const char *p_key, int n_key,
                                     Kvdb_GetKeyCallback p_callback,
                                     void *p_usr_arg);
/************************************************************************
*Description : ͨ����ֵ��ȡ����
*Parameters  :
*Return      : >0 �ɹ�(��ȡ���ݳ���); KVDB_RET_FAILURE ʧ��
************************************************************************/
EXPORT_DLL int Kvdb_GetKeyToBuffer(const char *p_key, int n_key,
                                   char *p_value, unsigned int n_value);

/************************************************************************
*Description : ͨ����ֵ��ȡ����
*Parameters  :
*Return      : >0 �ɹ�(��ȡ���ݳ���); KVDB_RET_FAILURE ʧ��
************************************************************************/
EXPORT_DLL int Kvdb_GetKeyToString(const char *p_key, int n_key, void *p_string);

/************************************************************************
*Description : ͨ����ֵ��ȡ����
*Parameters  :
*Return      : >0 �ɹ�(��ȡ���ݳ���); KVDB_RET_FAILURE ʧ��
************************************************************************/
EXPORT_DLL int Kvdb_GetKeyAbsolutelyToBuffer(const char *p_key, int n_key,
    char *p_value, unsigned int n_value);

// Delete the key
// Return KVDB_RET_FAILURE, KVDB_RET_SUCCEED
EXPORT_DLL int Kvdb_DeleteKey(const char *p_key, int n_key);

// backup the kvdb database
// Return KVDB_RET_FAILURE, KVDB_RET_SUCCEED
EXPORT_DLL int Kvdb_BackupDatabase();

// restore the kvdb database
// Return KVDB_RET_FAILURE, KVDB_RET_SUCCEED
EXPORT_DLL int Kvdb_RestoreDatabase();

//////////////////////////////////////////////////////////////////////////
// Default kvdb server:port is 127.0.0.1:5299
EXPORT_DLL int SKvdb_Start(const char *server, unsigned short port);
EXPORT_DLL int SKvdb_Stop();

// SetKey, if the key not exist, then create the key
// Return KVDB_RET_FAILURE, KVDB_RET_SUCCEED
EXPORT_DLL int SKvdb_SetKey(const char *p_key, int n_key,
                            const char *p_value, int n_value);

/************************************************************************
*Description : ͨ����ֵ��ȡ����
*Parameters  :
*Return      : >0 �ɹ�(��ȡ���ݳ���); KVDB_RET_FAILURE ʧ��
************************************************************************/
EXPORT_DLL int SKvdb_GetKey(const char *p_key, int n_key,
                            Kvdb_GetKeyCallback p_callback,
                            void *p_usr_arg);

/************************************************************************
*Description : ͨ����ֵ��ȡ����
*Parameters  :
*Return      : >0 �ɹ�(��ȡ���ݳ���); KVDB_RET_FAILURE ʧ��
************************************************************************/
EXPORT_DLL int SKvdb_GetKeyAbsolutely(const char *p_key, int n_key,
                                      Kvdb_GetKeyCallback p_callback,
                                      void *p_usr_arg);

/************************************************************************
*Description : ͨ����ֵ��ȡ����
*Parameters  :
*Return      : >0 �ɹ�(��ȡ���ݳ���); KVDB_RET_FAILURE ʧ��
************************************************************************/
EXPORT_DLL int SKvdb_GetKeyToBuffer(const char *p_key, int n_key,
                                    char *p_value, unsigned int n_value);

/************************************************************************
*Description : ͨ����ֵ��ȡ����
*Parameters  :
*Return      : >0 �ɹ�(��ȡ���ݳ���); KVDB_RET_FAILURE ʧ��
************************************************************************/
EXPORT_DLL int SKvdb_GetKeyToString(const char *p_key, int n_key, void *p_string);

/************************************************************************
*Description : ͨ����ֵ��ȡ����
*Parameters  :
*Return      : >0 �ɹ�(��ȡ���ݳ���); KVDB_RET_FAILURE ʧ��
************************************************************************/
EXPORT_DLL int SKvdb_GetKeyAbsolutelyToBuffer(const char *p_key, int n_key,
    char *p_value, unsigned int n_value);

// Delete the key
// Return KVDB_RET_FAILURE, KVDB_RET_SUCCEED
EXPORT_DLL int SKvdb_DeleteKey(const char *p_key, int n_key);

// backup the kvdb database
// Return KVDB_RET_FAILURE, KVDB_RET_SUCCEED
EXPORT_DLL int SKvdb_BackupDatabase();

// restore the kvdb database
// Return KVDB_RET_FAILURE, KVDB_RET_SUCCEED
EXPORT_DLL int SKvdb_RestoreDatabase();


#ifdef __cplusplus
}
#endif

#endif  // LIBDISPATCH_DPCLIENT_C_H_
