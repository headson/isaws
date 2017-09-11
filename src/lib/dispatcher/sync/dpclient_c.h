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

#include "dispatcher/base/pkghead.h"

#ifdef __cplusplus
extern "C" {
#endif

#if 1
typedef void* DpEvtService;
typedef void* DPPollHandle;
#define DP_POLL_HANDLE_NULL NULL

// 在回调中,避免使用同一个socket send\recv,造成递归evt loop
typedef void(*DpClient_MessageCallback)(
  DPPollHandle p_hdl, const DpMessage *dmp, void* p_usr_arg);

#define DP_CLIENT_DISCONNECT    101   // 断开链接,需要重连
#define DP_POLL_ISNOT_REG_MSG   102   // 没注册消息,需要重连
typedef void(*DpClient_PollStateCallback)(
  DPPollHandle p_hdl, uint32 n_state, void* p_usr_arg);

#endif
/************************************************************************
*Description : 初始化Dispatcher的服务器[地址:端口]
*Parameters  :
*Return      :
************************************************************************/
EXPORT_DLL void DpClient_Init(const char* ip_addr, unsigned short port);

EXPORT_DLL int  DpClient_Start(int new_thread);

/************************************************************************
*Description : 销毁Dispatcher客户端
*Parameters  :
*Return      :
************************************************************************/
EXPORT_DLL void DpClient_Stop();

/************************************************************************
*Description : DpClient发送消息
*Parameters  : p_method[IN] 消息方法
*              n_session_id[IN]
*              p_data[IN] 发送数据
*              n_data[IN] 发送数据大小
*Return      : VZNETDP_FAILURE 失败; VZNETDP_SUCCEED 成功
************************************************************************/
EXPORT_DLL int  DpClient_SendDpMessage(const char    *p_method,
                                       unsigned char  n_session_id,
                                       const char    *p_data,
                                       int            n_data);

/************************************************************************
*Description : DpClient发送请求
*Parameters  : p_method[IN] 请求方法
*              n_session_id[IN]
*              p_data[IN] 发送数据
*              n_data[IN] 发送数据大小
*              p_callback[IN] 请求回执回调
*              p_user_arg[IN] 请求回执回调参数
*              n_timeout[IN] 函数调用超时时间
*Return      : VZNETDP_FAILURE 失败; VZNETDP_SUCCEED 成功
************************************************************************/
EXPORT_DLL int DpClient_SendDpRequest(const char                *p_method,
                                      unsigned char              n_session_id,
                                      const char                *p_data,
                                      int                        n_data,
                                      DpClient_MessageCallback   p_callback,
                                      void                      *p_user_arg,
                                      unsigned int               n_timeout);

#ifdef __cplusplus
// p_reply std::string*
EXPORT_DLL int DpClient_SendDpReqToString(const char *p_method,
    unsigned char  n_session_id,
    const char    *p_data,
    int            n_data,
    void          *p_reply,
    unsigned int   n_timeout);
#endif  // __cplusplus
/************************************************************************
*Description : DpClient发送回执
*Parameters  : p_method[IN] 回执消息方法
*              n_session_id[IN] DpMessage->channel_id
*              n_message_id[IN] DpMessage->id
*              p_data[IN] 发送数据
*              n_data[IN] 发送数据大小
*Return      : VZNETDP_FAILURE 失败; VZNETDP_SUCCEED 成功
************************************************************************/
EXPORT_DLL int DpClient_SendDpReply(const char      *p_method,
                                    unsigned char    n_session_id,
                                    unsigned int     n_message_id,
                                    const char      *p_data,
                                    int              n_data);

///Poll///////////////////////////////////////////////////////////////////
/************************************************************************
*Description : 创建轮询句柄
*Parameters  : p_msg_cb[IN] 消息回调
*              p_msg_usr_arg[IN] 消息回调参数
*              p_state_cb[IN] 状态回调:断网,重连,重注册监听消息
*              p_msg_usr_arg[IN] 状态回调参数
*              p_evt_loop[IN] EVT_LOOP指针
*Return      : NULL 失败, !=NULL成功
************************************************************************/
EXPORT_DLL DPPollHandle DpClient_CreatePollHandle(
  DpClient_MessageCallback   p_msg_cb,
  void                      *p_msg_usr_arg,
  DpClient_PollStateCallback p_state_cb,
  void                      *p_state_usr_arg,
  DpEvtService               p_evt_service = NULL);

/************************************************************************
*Description : 销毁轮询句柄,此时会把EVT_LOOP一起销毁,注意其他使用此EVT_LOOP的地方
*Parameters  :
*Return      :
************************************************************************/
EXPORT_DLL void  DpClient_ReleasePollHandle(DPPollHandle p_poll_handle);

/************************************************************************
*Description : 轮询重连;当DpClient_PollDpMessage返回错误时,调用此函数重连服务器
*Parameters  : p_poll_handle[IN] 轮询句柄
*Return      : VZNETDP_FAILURE失败 VZNETDP_SUCCEED成功
************************************************************************/
EXPORT_DLL int DpClient_HdlReConnect(const DPPollHandle p_poll_handle);

/************************************************************************
*Description : 轮询注册监听消息;
*              DpClient_HdlReConnect立即执行,如果传入空消息会一直重连
*Parameters  : p_poll_handle[IN]
*              a_method_set[IN] 消息数组,每一个消息32Byte
*              n_method_set_cnt[IN] 消息个数
*Return      : VZNETDP_FAILURE失败 VZNETDP_SUCCEED成功
************************************************************************/
EXPORT_DLL int   DpClient_HdlAddListenMessage(
  const DPPollHandle p_poll_handle,
  const char        *a_method_set[],
  unsigned int       n_method_set_cnt);

/************************************************************************
*Description : 轮询取消监听消息
*Parameters  : p_poll_handle[IN]
*              a_method_set[IN] 消息数组,每一个消息32Byte
*              n_method_set_cnt[IN] 消息个数
*Return      : VZNETDP_FAILURE失败 VZNETDP_SUCCEED成功
************************************************************************/
EXPORT_DLL int   DpClient_HdlRemoveListenMessage(
  const DPPollHandle p_poll_handle,
  const char        *a_method_set[],
  unsigned int       n_method_set_cnt);

// 当使用vzbase::Thread设置轮询的EVT_LOOP时,可不调用此函数分发
// 请尽量使用vzbase::Thread::Current()->Run()来替换此函数,
// 因为线程可投递消息来实现定时器
/************************************************************************
*Description : 轮询,消息分发
*Parameters  : p_poll_handle[IN]
*              n_timeout[IN] 轮询超时时间
*Return      : VZNETDP_FAILURE失败 VZNETDP_SUCCEED成功
************************************************************************/
EXPORT_DLL int DpClient_PollDpMessage(
  const DPPollHandle         p_poll_handle,
  unsigned int               n_timeout);

/************************************************************************
*Description : 从轮询中获取EVT_LOOP指针
*Parameters  : p_poll_handle[IN]
*Return      : NULL 失败, !=NULL成功
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
#define KVDB_RET_SUCCEED    0
#define KVDB_RET_FAILURE   -1

// Default kvdb server:port is 127.0.0.1:5299
EXPORT_DLL int Kvdb_Start(const char *server, unsigned short port);
EXPORT_DLL int Kvdb_Stop();

/************************************************************************
*Description : 存储参数;if the key not exist, then create the key
*Parameters  : p_key[IN] 存储参数名称
*              n_key[IN] 存储参数名称长度
*              p_value[IN] 存储参数数据
*              n_value[IN] 存储参数数据长度
*Return      : KVDB_RET_FAILURE, KVDB_RET_SUCCEED
************************************************************************/
EXPORT_DLL int Kvdb_SetKey(const char *p_key, int n_key,
                           const char *p_value, int n_value);

/************************************************************************
*Description : 通过键值获取数据
*Parameters  : p_key[IN] 获取参数名称
*              n_key[IN] 获取参数名称长度
*              p_callback[IN] 获取参数到的参数从回调中返回
*              p_usr_arg[IN] 回调是返回此用户参数
*Return      : >0 成功(获取数据长度); KVDB_RET_FAILURE 失败
************************************************************************/
EXPORT_DLL int Kvdb_GetKey(const char *p_key, int n_key,
                           Kvdb_GetKeyCallback p_callback,
                           void *p_usr_arg);

/************************************************************************
*Description : 通过键值获取数据
*Parameters  : p_key[IN] 获取参数名称
*              n_key[IN] 获取参数名称长度
*              p_callback[IN] 获取参数到的参数从回调中返回
*              p_usr_arg[IN] 回调是返回此用户参数
*Return      : >0 成功(获取数据长度); KVDB_RET_FAILURE 失败
************************************************************************/
EXPORT_DLL int Kvdb_GetKeyAbsolutely(const char *p_key, int n_key,
                                     Kvdb_GetKeyCallback p_callback,
                                     void *p_usr_arg);
/************************************************************************
*Description : 通过键值获取数据
*Parameters  : p_key[IN] 获取参数名称
*              n_key[IN] 获取参数名称长度
*              p_value[IN] 获取参数到的参数存入此buffer
*              n_value[IN] 参数存入此buffer长度
*Return      : >0 成功(获取数据长度); KVDB_RET_FAILURE 失败
************************************************************************/
EXPORT_DLL int Kvdb_GetKeyToBuffer(const char *p_key, int n_key,
                                   char *p_value, unsigned int n_value);

/************************************************************************
*Description : 通过键值获取数据
*Parameters  : p_key[IN] 获取参数名称
*              n_key[IN] 获取参数名称长度
*              p_string[IN] 获取参数到的参数存入此std:string*
*Return      : >0 成功(获取数据长度); KVDB_RET_FAILURE 失败
************************************************************************/
#ifdef __cplusplus
EXPORT_DLL int Kvdb_GetKeyToString(const char *p_key, int n_key, void *p_string);
#endif  // __cplusplus

/************************************************************************
*Description : 通过键值获取数据
*Parameters  : p_key[IN] 获取参数名称
*              n_key[IN] 获取参数名称长度
*              p_value[IN] 获取参数到的参数存入此buffer
*              n_value[IN] 参数存入此buffer长度
*Return      : >0 成功(获取数据长度); KVDB_RET_FAILURE 失败
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

// database use transaction
// Return KVDB_RET_FAILURE, KVDB_RET_SUCCEED
EXPORT_DLL int Kvdb_Transaction(int e_trans);

//////////////////////////////////////////////////////////////////////////
// Default kvdb server:port is 127.0.0.1:5299
EXPORT_DLL int SKvdb_Start(const char *server, unsigned short port);
EXPORT_DLL int SKvdb_Stop();

// SetKey, if the key not exist, then create the key
// Return KVDB_RET_FAILURE, KVDB_RET_SUCCEED
EXPORT_DLL int SKvdb_SetKey(const char *p_key, int n_key,
                            const char *p_value, int n_value);

/************************************************************************
*Description : 通过键值获取数据
*Parameters  :
*Return      : >0 成功(获取数据长度); KVDB_RET_FAILURE 失败
************************************************************************/
EXPORT_DLL int SKvdb_GetKey(const char *p_key, int n_key,
                            Kvdb_GetKeyCallback p_callback,
                            void *p_usr_arg);

/************************************************************************
*Description : 通过键值获取数据
*Parameters  :
*Return      : >0 成功(获取数据长度); KVDB_RET_FAILURE 失败
************************************************************************/
EXPORT_DLL int SKvdb_GetKeyAbsolutely(const char *p_key, int n_key,
                                      Kvdb_GetKeyCallback p_callback,
                                      void *p_usr_arg);

/************************************************************************
*Description : 通过键值获取数据
*Parameters  :
*Return      : >0 成功(获取数据长度); KVDB_RET_FAILURE 失败
************************************************************************/
EXPORT_DLL int SKvdb_GetKeyToBuffer(const char *p_key, int n_key,
                                    char *p_value, unsigned int n_value);

/************************************************************************
*Description : 通过键值获取数据
*Parameters  :
*Return      : >0 成功(获取数据长度); KVDB_RET_FAILURE 失败
************************************************************************/
#ifdef __cplusplus
EXPORT_DLL int SKvdb_GetKeyToString(const char *p_key, int n_key, void *p_string);
#endif  // __cplusplus

/************************************************************************
*Description : 通过键值获取数据
*Parameters  :
*Return      : >0 成功(获取数据长度); KVDB_RET_FAILURE 失败
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

// database use transaction
// Return KVDB_RET_FAILURE, KVDB_RET_SUCCEED
EXPORT_DLL int SKvdb_Transaction(int e_trans);

#ifdef __cplusplus
}
#endif

#endif  // LIBDISPATCH_DPCLIENT_C_H_
