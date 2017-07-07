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

// �ڻص���,����ʹ��ͬһ��socket send\recv,��ɵݹ�evt loop
typedef void(*DpClient_MessageCallback)(
  DPPollHandle p_hdl, const DpMessage *dmp, void* p_usr_arg);

#define DP_CLIENT_DISCONNECT  101   // �Ͽ�����,��Ҫ����
typedef void(*DpClient_PollStateCallback)(
  DPPollHandle p_hdl, uint32 n_state, void* p_usr_arg);

#define DP_POLL_HANDLE_NULL NULL

// Not thread safe
EXPORT_DLL void DpClient_Init(const char* ip_addr, unsigned short port);

// ��ʱ��֧�ֶ��߳�
// Only Call once
// new_thread = 1, create a new thread to run the dp client
// net_thread = 0, used this current thread to run the dp client
// return VZNETDP_FAILURE / or VZNETDP_SUCCEED
EXPORT_DLL int  DpClient_Start(int new_thread);

// Only call once
EXPORT_DLL void DpClient_Stop();

// return VZNETDP_FAILURE / or VZNETDP_SUCCEED
EXPORT_DLL int  DpClient_SendDpMessage(const char    *method,
                                       unsigned char  channel_id,
                                       const char    *data,
                                       int            data_size);

// return VZNETDP_FAILURE, ERROR
// return > 0, request id
EXPORT_DLL unsigned int DpClient_SendDpRequest(
  const char                *method,
  unsigned char              channel_id,
  const char                *data,
  int                        data_size,
  DpClient_MessageCallback   call_back,
  void                      *user_data,
  unsigned int               timeout);

// return VZNETDP_FAILURE / or VZNETDP_SUCCEED
EXPORT_DLL int DpClient_SendDpReply(const char      *method,
                                    unsigned char    channel_id,
                                    unsigned int     id,
                                    const char      *data,
                                    int              data_size);

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
*Description : ��ѯע�������Ϣ
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
/************************************************************************
*Description : ��ѯ,�ȴ�������Ϣ,ͨ���ص����ظ�Ӧ�ò�
*              VZNETDP_FAILUREʱ��Ҫ���´���handle[����],�ط�AddListenMessage
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

//////////////////////////////////////////////////////////////////////////
// GetKey callback function
typedef void(*Kvdb_GetKeyCallback)(const char *p_key,
                                   int         n_key,
                                   const char *p_value,
                                   int         n_value,
                                   void       *p_user_data);

// Return of Key Value database Interface
#define KVDB_RET_SUCCEED                     0
#define KVDB_RET_FAILURE                    -1

// Default kvdb server:port is 127.0.0.1:5299
EXPORT_DLL int Kvdb_Start(const char *server, unsigned short port);
EXPORT_DLL int Kvdb_Stop();

// SetKey, if the key not exist, then create the key
// Return KV_SUCCEED, KV_DATABASE_NOT_EXIST
EXPORT_DLL int Kvdb_SetKey(const char *p_key, int n_key,
                           const char *p_value, int n_value);

// Get the key
// Return KV_SUCCEED KV_DATABASE_NOT_EXIST, KV_KEY_NOT_EXIST
EXPORT_DLL int Kvdb_GetKey(const char *p_key, int n_key,
                           Kvdb_GetKeyCallback p_callback,
                           void *p_usr_arg);

// Return KV_SUCCEED KV_DATABASE_NOT_EXIST, KV_KEY_NOT_EXIST
EXPORT_DLL int Kvdb_GetKeyAbsolutely(const char *p_key, int n_key,
                                     Kvdb_GetKeyCallback p_callback,
                                     void *p_usr_arg);
// Return the result size
// If the buffer_size small than result size,
//    then return KVDB_BUFFER_SMALL_THAN_RESULT
// If the reuslt size == 0, then return KVDB_FAILURE;
// If the key not found then return KVDB_KEY_NOT_EXIST
EXPORT_DLL int Kvdb_GetKeyToBuffer(const char *p_key, int n_key,
                                   char *p_value, unsigned int n_value);
// Return the result size
// If the buffer_size small than result size,
//    then return KVDB_BUFFER_SMALL_THAN_RESULT
// If the reuslt size == 0, then return KVDB_FAILURE;
// If the key not found then return KVDB_KEY_NOT_EXIST
EXPORT_DLL int Kvdb_GetKeyAbsolutelyToBuffer(const char *p_key, int n_key,
    char *p_value, unsigned int n_value);

// Delete the key
// Return KV_SUCCEED, KV_DATABASE_NOT_EXIST, KV_KEY_NOT_EXIST
EXPORT_DLL int Kvdb_DeleteKey(const char *p_key, int n_key);

// backup the kvdb database
// Return KV_SUCCEED, KVDB_FAILURE
EXPORT_DLL int Kvdb_BackupDatabase();

// restore the kvdb database
// Return KV_SUCCEED, KVDB_FAILURE
EXPORT_DLL int Kvdb_RestoreDatabase();

//////////////////////////////////////////////////////////////////////////
// Default kvdb server:port is 127.0.0.1:5299
EXPORT_DLL int SKvdb_Start(const char *server, unsigned short port);
EXPORT_DLL int SKvdb_Stop();

// SetKey, if the key not exist, then create the key
// Return KV_SUCCEED, KV_DATABASE_NOT_EXIST
EXPORT_DLL int SKvdb_SetKey(const char *p_key, int n_key,
                            const char *p_value, int n_value);

// Get the key
// Return KV_SUCCEED KV_DATABASE_NOT_EXIST, KV_KEY_NOT_EXIST
EXPORT_DLL int SKvdb_GetKey(const char *p_key, int n_key,
                            Kvdb_GetKeyCallback p_callback,
                            void *p_usr_arg);

// Return KV_SUCCEED KV_DATABASE_NOT_EXIST, KV_KEY_NOT_EXIST
EXPORT_DLL int SKvdb_GetKeyAbsolutely(const char *p_key, int n_key,
                                      Kvdb_GetKeyCallback p_callback,
                                      void *p_usr_arg);
// Return the result size
// If the buffer_size small than result size,
//    then return KVDB_BUFFER_SMALL_THAN_RESULT
// If the reuslt size == 0, then return KVDB_FAILURE;
// If the key not found then return KVDB_KEY_NOT_EXIST
EXPORT_DLL int SKvdb_GetKeyToBuffer(const char *p_key, int n_key,
                                    char *p_value, unsigned int n_value);
// Return the result size
// If the buffer_size small than result size,
//    then return KVDB_BUFFER_SMALL_THAN_RESULT
// If the reuslt size == 0, then return KVDB_FAILURE;
// If the key not found then return KVDB_KEY_NOT_EXIST
EXPORT_DLL int SKvdb_GetKeyAbsolutelyToBuffer(const char *p_key, int n_key,
    char *p_value, unsigned int n_value);

// Delete the key
// Return KV_SUCCEED, KV_DATABASE_NOT_EXIST, KV_KEY_NOT_EXIST
EXPORT_DLL int SKvdb_DeleteKey(const char *p_key, int n_key);

// backup the kvdb database
// Return KV_SUCCEED, KVDB_FAILURE
EXPORT_DLL int SKvdb_BackupDatabase();

// restore the kvdb database
// Return KV_SUCCEED, KVDB_FAILURE
EXPORT_DLL int SKvdb_RestoreDatabase();


#ifdef __cplusplus
}
#endif

#endif  // LIBDISPATCH_DPCLIENT_C_H_
