/************************************************************************/
/* Author      : Sober.Peng 17-06-20
/* Description :
/************************************************************************/
#ifndef LIBDISPATCH_DPCLIENT_C_H_
#define LIBDISPATCH_DPCLIENT_C_H_

#ifdef WIN32
#define EXPORT_DLL _declspec(dllexport)
#else
#define EXPORT_DLL
#endif

#ifdef __cplusplus
extern "C" {
#endif
typedef struct _TagDpMsg DpMessage;
typedef void(*DpClient_MessageCallback)(const DpMessage *dmp, void* p_usr_arg);

// Not thread safe
EXPORT_DLL void DpClient_Init(const char* ip_addr, unsigned short port);

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

// return VZNETDP_FAILURE / or VZNETDP_SUCCEED
EXPORT_DLL int DpClient_PollRecvMessage(DpClient_MessageCallback   call_back,
                                        void                      *user_data,
                                        unsigned int               timeout);


#ifdef __cplusplus
}
#endif

#endif  // LIBDISPATCH_DPCLIENT_C_H_
