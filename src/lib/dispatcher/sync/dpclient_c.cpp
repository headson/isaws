/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "dpclient_c.h"

#include "vzbase/helper/stdafx.h"
#include "dispatcher/base/pkghead.h"
#include "dispatcher/sync/cdpclient.h"
#include "dispatcher/sync/ckvdbclient.h"
#include "dispatcher/sync/cdppollclient.h"

#include "vzconn/base/clibevent.h"
#include "vzconn/sync/ctcpclient.h"

// TLS
#ifdef WIN32
#include <process.h>
typedef DWORD               TlsKey;
#define TLS_NULL            0
#else
#include <pthread.h>
typedef pthread_key_t       TlsKey;
#define TLS_NULL            (pthread_key_t)-1

#include <netinet/tcp.h>
#endif

class VTls {
 public:
  VTls()
    : tls_key_(TLS_NULL) {
  }
  ~VTls() {
    KeyFree();
  }

  // 启动进程时调用一次，分配关键字
  TlsKey KeyAlloc() {
    if (!IsReady()) {
#ifdef WIN32
      tls_key_ = TlsAlloc();
#else
      pthread_key_create(&tls_key_, NULL);
#endif
    }
    return tls_key_;
  }

  void KeyFree() {
    if (tls_key_ != TLS_NULL) {
#ifdef WIN32
      TlsFree(tls_key_);
#else
      pthread_key_delete(tls_key_);
#endif
      tls_key_ = TLS_NULL;
    }
  }

  // true可用,false不可用
  bool   IsReady() {
    if (TLS_NULL != tls_key_) {
      return true;
    }
    return false;
  }

  // 线程中调用，关联关键字和私有数据对应指针
  bool   SetValue(void* value) {
    if (!IsReady()) {
      return false;
    }
#ifdef WIN32
    return TlsSetValue(tls_key_, value) == TRUE;
#else
    return pthread_setspecific(tls_key_, value);
#endif
  }
  // 线程中调用，获取关键字关联的线程私有数据
  void*  GetValue() {
    if (!IsReady()) {
      return NULL;
    }
#ifdef WIN32
    return TlsGetValue(tls_key_);
#else
    return pthread_getspecific(tls_key_);
#endif
  }

 private:
  TlsKey  tls_key_;
};

static VTls                 g_dp_tls;                      //
static char                 g_dp_addr[64];
static unsigned short       g_dp_port;
static CDpClient*           g_dp_client[MAX_CLIS_PER_PROC];

// b_can_reconn 断网时允许重连; poll不允许重连
CDpClient* GetDpCli(bool b_can_reconn =true) {
  CDpClient* p_tcp = NULL;
  p_tcp = (CDpClient*)g_dp_tls.GetValue();
  if (p_tcp == NULL) {
    p_tcp = CDpClient::Create(g_dp_addr, g_dp_port);
    if (p_tcp) {
      for (int32 i = 0; i < MAX_CLIS_PER_PROC; i++) {
        if (g_dp_client[i] == NULL) {
          g_dp_client[i] = p_tcp;
          break;
        }
      } 
      g_dp_tls.SetValue(p_tcp);
    }
  }
  return p_tcp;
}

EXPORT_DLL void DpClient_Init(const char* ip_addr, unsigned short port) {
  g_dp_port = port;

  memset(g_dp_addr, 0, 63);
  strncpy(g_dp_addr, ip_addr, 63);

  for (int32 i = 0; i < MAX_CLIS_PER_PROC; i++) {
    g_dp_client[i] = NULL;
  }

  TlsKey tk = g_dp_tls.KeyAlloc();
  if (tk == TLS_NULL) {
    LOG(L_ERROR) << "alloc tls key failed.";
  }
}

EXPORT_DLL int DpClient_Start(int new_thread) {
  return VZNETDP_SUCCEED;
}

EXPORT_DLL void DpClient_Stop() {
  g_dp_tls.KeyFree();

  for (int32 i = 0; i < MAX_CLIS_PER_PROC; i++) {
    if (g_dp_client[i] != NULL) {
      delete g_dp_client[i];
    }
    g_dp_client[i] = NULL;
  }
}

EXPORT_DLL int DpClient_SendDpMessage(const char    *method,
                                      unsigned char  channel_id,
                                      const char    *data,
                                      int            data_size) {
  // LOG(L_INFO) << "send dp message "<<method;

  int32 n_ret = 0;
  CDpClient* p_tcp = GetDpCli();
  if (!p_tcp) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }

  n_ret = p_tcp->SendDpMessage(method,
                               channel_id,
                               data,
                               data_size);
  return n_ret;
}

EXPORT_DLL int DpClient_SendDpRequest(const char *method,
                                      unsigned char             channel_id,
                                      const char               *data,
                                      int                       data_size,
                                      DpClient_MessageCallback  call_back,
                                      void                     *user_data,
                                      unsigned int              timeout) {
  LOG(L_INFO) << "send dp request " << method;

  int32 n_ret = 0;
  CDpClient* p_tcp = GetDpCli();
  if (!p_tcp) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }

  n_ret = p_tcp->SendDpRequest(method,
                               channel_id,
                               data,
                               data_size,
                               call_back,
                               user_data,
                               timeout);
  LOG(L_INFO) << "send dp request " << method;
  return n_ret;
}

EXPORT_DLL int DpClient_SendDpReqToString(const char *p_method,
    unsigned char n_session_id,
    const char *p_data,
    int n_data,
    void *p_reply,
    unsigned int n_timeout) {
  int32 n_ret = 0;
  CDpClient* p_tcp = GetDpCli();
  if (!p_tcp) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }

  n_ret = p_tcp->SendDpRequest(p_method,
                               n_session_id,
                               p_data,
                               n_data,
                               (std::string*)p_reply,
                               n_timeout);
  return n_ret;
}

EXPORT_DLL int DpClient_SendDpReply(const char    *method,
                                    unsigned char  channel_id,
                                    unsigned int   id,
                                    const char    *data,
                                    int            data_size) {
  // LOG(L_INFO) << "send dp reply "<<method;

  int32 n_ret = 0;
  CDpClient* p_tcp = GetDpCli();
  if (!p_tcp) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }

  n_ret = p_tcp->SendDpReply(method,
                             channel_id,
                             id,
                             data,
                             data_size);
  return n_ret;
}

//////////////////////////////////////////////////////////////////////////
EXPORT_DLL DPPollHandle DpClient_CreatePollHandle(
  DpClient_MessageCallback   p_msg_cb,
  void                      *p_msg_usr_arg,
  DpClient_PollStateCallback p_state_cb,
  void                      *p_state_usr_arg,
  DpEvtService               p_evt_service) {

  if (!p_msg_cb || !p_state_cb) {
    LOG(L_ERROR) << "param is null";
    return NULL;
  }

  vzconn::EVT_LOOP *p_evt_loop = (vzconn::EVT_LOOP*)p_evt_service;
  if (p_evt_loop == NULL) {
    p_evt_loop = new vzconn::EVT_LOOP();
    if (p_evt_loop) {
      if (p_evt_loop->Start() != 0) {
        delete p_evt_loop;
        p_evt_loop = NULL;
      }
    }
  }

  if (p_evt_loop == NULL) {
    LOG(L_ERROR) << "evt loop is null.";
    return NULL;
  }

  CDpPollClient *p_tcp = CDpPollClient::Create(g_dp_addr, g_dp_port,
                         p_msg_cb, p_msg_usr_arg,
                         p_state_cb, p_state_usr_arg,
                         p_evt_loop);
  if (NULL == p_tcp) {
    LOG(L_ERROR) << "create dp client failed.";
  }

  DpClient_HdlReConnect(p_tcp);
  return (DPPollHandle*)p_tcp;
}

EXPORT_DLL void DpClient_ReleasePollHandle(DPPollHandle p_poll_handle) {
  if (p_poll_handle) {
    delete ((CDpPollClient*)p_poll_handle);
    p_poll_handle = NULL;
  }
}

EXPORT_DLL int DpClient_HdlReConnect(const DPPollHandle p_poll_handle) {
  CDpPollClient* p_tcp = (CDpPollClient*)p_poll_handle;
  if (!p_tcp) {
    LOG(L_ERROR) << "client failed.";
    return VZNETDP_FAILURE;
  }

  bool b_ret = p_tcp->CheckAndConnected();
  if (b_ret) {
    return VZNETDP_SUCCEED;
  }
  return VZNETDP_FAILURE;
}

EXPORT_DLL int DpClient_HdlAddListenMessage(const DPPollHandle p_poll_handle,
    const char *method_set[],
    unsigned int set_size) {
  int32 n_ret = VZNETDP_FAILURE;
  CDpPollClient* p_tcp = (CDpPollClient*)p_poll_handle;
  if (!p_tcp) {
    LOG(L_ERROR) << "client failed.";
    return VZNETDP_FAILURE;
  }

  n_ret = p_tcp->ListenMessage(TYPE_ADD_MESSAGE,
                               method_set,
                               set_size,
                               FLAG_ADDLISTEN_MESSAGE);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "add message failed " << n_ret;
    return n_ret;
  }

  // LOG(L_WARNING) << "add listen message " << n_ret;
  return VZNETDP_SUCCEED;
}

EXPORT_DLL int DpClient_HdlRemoveListenMessage(const DPPollHandle p_poll_handle,
    const char *method_set[],
    unsigned int set_size) {
  int32 n_ret = VZNETDP_FAILURE;
  CDpPollClient* p_tcp = (CDpPollClient*)p_poll_handle;
  if (!p_tcp) {
    LOG(L_ERROR) << "client failed.";
    return VZNETDP_FAILURE;
  }

  n_ret = p_tcp->ListenMessage(TYPE_REMOVE_MESSAGE,
                               method_set,
                               set_size,
                               FLAG_REMOVELISTEN_MESSAGE);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "remove message failed " << n_ret;
    return n_ret;
  }

  p_tcp->RunLoop(DEF_TIMEOUT_MSEC);
  return VZNETDP_SUCCEED;
}

EXPORT_DLL int DpClient_PollDpMessage(const DPPollHandle p_poll_handle,
                                      unsigned int timeout) {
  int32 n_ret = 0;
  // 此处判断断开不重连,目的是为了让注册method重连
  CDpPollClient* p_tcp = (CDpPollClient*)p_poll_handle;
  if (!p_tcp) {
    // LOG(L_ERROR) << "get client failed.";
    return VZNETDP_FAILURE;
  }

  n_ret = p_tcp->RunLoop(timeout);
  return VZNETDP_SUCCEED;
}

EXPORT_DLL DpEvtService DpClient_GetEvtLoopFromPoll(
  const DPPollHandle p_poll_handle) {

  CDpPollClient* p_tcp = (CDpPollClient*)p_poll_handle;
  if (!p_tcp) {
    LOG(L_ERROR) << "get client failed.";
    return NULL;
  }

  return p_tcp->GetEvtLoop();
}

///SIGNAL/////////////////////////////////////////////////////////////////
EXPORT_DLL EventSignal Event_CreateSignalHandle(
  const DpEvtService   p_evt_service,
  int                  n_signal_no,
  Event_SignalCallback p_callback,
  void                *p_user_arg) {
  vzconn::EVT_IO *p_evt_io = new vzconn::EVT_IO();
  if (NULL == p_evt_io) {
    LOG(L_ERROR) << "new evt_io failed.";
    return NULL;
  }

  p_evt_io->Init((vzconn::EVT_LOOP*)p_evt_service,
                 (vzconn::EVT_FUNC)p_callback, p_user_arg);
  int32 n_ret = p_evt_io->Start(n_signal_no, EV_SIGNAL | EVT_PERSIST);
  if (n_ret != 0) {
    LOG(L_ERROR) << "listening signal failed.";

    delete p_evt_io;
    p_evt_io = NULL;
  }
  return p_evt_io;
}

EXPORT_DLL void Event_ReleaseSignalHandle(EventSignal p_evt_handle) {
  vzconn::EVT_IO *p_evt_io = (vzconn::EVT_IO*)p_evt_handle;
  if (p_evt_io == NULL) {
    LOG(L_ERROR) << "param is null.";
    return;
  }

  p_evt_io->Stop();

  delete p_evt_io;
  p_evt_io = NULL;
}

//////////////////////////////////////////////////////////////////////////
static CKvdbClient         *g_kvdb_client   = NULL;

EXPORT_DLL int Kvdb_Start(const char *server, unsigned short port) {
  if (g_kvdb_client == NULL) {
    g_kvdb_client = CKvdbClient::Create(server, port);
  }
  if (g_kvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }
  return KVDB_RET_SUCCEED;
}

EXPORT_DLL int Kvdb_Stop() {
  if (g_kvdb_client) {
    delete g_kvdb_client;
    g_kvdb_client = NULL;
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int Kvdb_SetKey(const char *p_key,
                           int         n_key,
                           const char *p_value,
                           int         n_value) {
  if (g_kvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (g_kvdb_client->SetKey(p_key, n_key, p_value, n_value)) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int Kvdb_GetKey(const char          *p_key,
                           int                  n_key,
                           Kvdb_GetKeyCallback  p_callback,
                           void                *p_usr_arg) {
  if (g_kvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  int32 ret = g_kvdb_client->GetKey(p_key, n_key, p_callback, p_usr_arg, false);
  if (ret > 0) {
    return ret;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int Kvdb_GetKeyAbsolutely(const char           *p_key,
                                     int                   n_key,
                                     Kvdb_GetKeyCallback   p_callback,
                                     void                 *p_usr_arg) {
  if (g_kvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  int32 ret = g_kvdb_client->GetKey(p_key, n_key, p_callback, p_usr_arg, true);
  if (ret > 0) {
    return ret;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int Kvdb_GetKeyToBuffer(const char   *p_key,
                                   int           n_key,
                                   char         *p_value,
                                   unsigned int  n_value) {
  if (g_kvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  int32 ret = g_kvdb_client->GetKey(p_key, n_key, p_value, n_value, false);
  if (ret > 0) {
    return ret;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int Kvdb_GetKeyToString(const char *p_key, int n_key, void *p_string) {
  if (g_kvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  int32 ret = g_kvdb_client->GetKey(p_key, n_key, (std::string*)p_string, false);
  if (ret > 0) {
    return ret;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int Kvdb_GetKeyAbsolutelyToBuffer(const char   *p_key,
    int           n_key,
    char         *p_value,
    unsigned int  n_value) {
  if (g_kvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  int32 ret = g_kvdb_client->GetKey(p_key, n_key, p_value, n_value, true);
  if (ret > 0) {
    return ret;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int Kvdb_DeleteKey(const char *p_key, int n_key) {
  if (g_kvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (g_kvdb_client->Delete(p_key, n_key)) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int Kvdb_BackupDatabase() {
  if (g_kvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (g_kvdb_client->BackupDatabase()) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int Kvdb_RestoreDatabase() {
  if (g_kvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (g_kvdb_client->RestoreDatabase()) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int Kvdb_Transaction(int e_trans) {
  if (g_kvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (g_kvdb_client->Transaction(e_trans)) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

//////////////////////////////////////////////////////////////////////////
static CKvdbClient         *g_skvdb_client = NULL;

EXPORT_DLL int SKvdb_Start(const char *server, unsigned short port) {
  if (g_skvdb_client == NULL) {
    g_skvdb_client = CKvdbClient::Create(server, port);
  }
  if (g_skvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }
  return KVDB_RET_SUCCEED;
}

EXPORT_DLL int SKvdb_Stop() {
  if (g_skvdb_client) {
    delete g_skvdb_client;
    g_skvdb_client = NULL;
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int SKvdb_SetKey(const char *p_key,
                            int         n_key,
                            const char *p_value,
                            int         n_value) {
  if (g_skvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (g_skvdb_client->SetKey(p_key, n_key, p_value, n_value)) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int SKvdb_GetKey(const char          *p_key,
                            int                  n_key,
                            Kvdb_GetKeyCallback  p_callback,
                            void                *p_usr_arg) {
  if (g_skvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  int32 ret = g_skvdb_client->GetKey(p_key, n_key, p_callback, p_usr_arg, false);
  if (ret > 0) {
    return ret;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int SKvdb_GetKeyAbsolutely(const char           *p_key,
                                      int                   n_key,
                                      Kvdb_GetKeyCallback   p_callback,
                                      void                 *p_usr_arg) {
  if (g_skvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  int32 ret = g_skvdb_client->GetKey(p_key, n_key, p_callback, p_usr_arg, true);
  if (ret > 0) {
    return ret;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int SKvdb_GetKeyToBuffer(const char   *p_key,
                                    int           n_key,
                                    char         *p_value,
                                    unsigned int  n_value) {
  if (g_skvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  int32 ret = g_skvdb_client->GetKey(p_key, n_key, p_value, n_value, false);
  if (ret > 0) {
    return ret;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int SKvdb_GetKeyToString(const char *p_key, int n_key, void *p_string) {
  if (g_skvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  int32 ret = g_skvdb_client->GetKey(p_key, n_key, (std::string*)p_string, false);
  if (ret > 0) {
    return ret;
  }
  return KVDB_RET_FAILURE;
}


EXPORT_DLL int SKvdb_GetKeyAbsolutelyToBuffer(const char   *p_key,
    int           n_key,
    char         *p_value,
    unsigned int  n_value) {
  if (g_skvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  int32 ret = g_skvdb_client->GetKey(p_key, n_key, p_value, n_value, true);
  if (ret > 0) {
    return ret;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int SKvdb_DeleteKey(const char *p_key, int n_key) {
  if (g_skvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (g_skvdb_client->Delete(p_key, n_key)) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int SKvdb_BackupDatabase() {
  if (g_skvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (g_skvdb_client->BackupDatabase()) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int SKvdb_RestoreDatabase() {
  if (g_skvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (g_skvdb_client->RestoreDatabase()) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int SKvdb_Transaction(int e_trans) {
  if (g_skvdb_client == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (g_skvdb_client->Transaction(e_trans)) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}
