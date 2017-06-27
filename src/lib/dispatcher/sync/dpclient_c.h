/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : dispatch\kvdb C语言客户端接口
************************************************************************/
#ifndef LIBDISPATCH_DPCLIENT_C_H_
#define LIBDISPATCH_DPCLIENT_C_H_

#ifdef WIN32
#define EXPORT_DLL _declspec(dllexport)
#else
#define EXPORT_DLL
#endif

typedef struct _TagDpMsg DpMessage;

#ifdef __cplusplus
extern "C" {
#endif

// 在回调中,避免使用同一个socket send\recv,造成递归evt loop
typedef void(*DpClient_MessageCallback)(const DpMessage *dmp, void* p_usr_arg);

// Not thread safe
EXPORT_DLL void DpClient_Init(const char* ip_addr, unsigned short port);

// 暂时不支持多线程
// Only Call once
// new_thread = 1, create a new thread to run the dp client
// net_thread = 0, used this current thread to run the dp client
// return VZNETDP_FAILURE / or VZNETDP_SUCCEED
EXPORT_DLL int  DpClient_Start(int new_thread);

// Only call once
EXPORT_DLL void DpClient_Stop();

// return VZNETDP_FAILURE / or VZNETDP_SUCCEED
EXPORT_DLL int  DpClient_AddListenMessage(const char *methodset[],
    unsigned int set_size);

// return VZNETDP_FAILURE / or VZNETDP_SUCCEED
EXPORT_DLL int  DpClient_RemoveListenMessage(const char  *methodset[],
    unsigned int set_size);

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
EXPORT_DLL void *DpClient_CreatePollHandle();
EXPORT_DLL void  DpClient_ReleasePollHandle(void *p_poll_handle);
EXPORT_DLL int   DpClient_HdlAddListenMessage(const void  *p_poll_handle,
    const char   *method_set[],
    unsigned int  set_size);

EXPORT_DLL int   DpClient_HdlRemoveListenMessage(const void  *p_poll_handle,
    const char   *method_set[],
    unsigned int  set_size);

// return VZNETDP_FAILURE / or VZNETDP_SUCCEED
// VZNETDP_FAILURE时需要重新创建handle[重连],重发AddListenMessage
EXPORT_DLL int DpClient_PollDpMessage(const void                *p_poll_handle,
                                      DpClient_MessageCallback   call_back,
                                      void                      *user_data,
                                      unsigned int               timeout);


//////////////////////////////////////////////////////////////////////////
// GetKey callback function
typedef void(*Kvdb_GetKeyCallback)(const char *key,
                                   int         key_size,
                                   const char *value,
                                   int         value_size,
                                   void       *user_data);

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
