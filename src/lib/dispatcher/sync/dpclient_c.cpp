/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "dpclient_c.h"

#include "vzbase/helper/stdafx.h"
#include "dispatcher/base/pkghead.h"
#include "dispatcher/sync/cdpclient.h"
#include "dispatcher/sync/ckvdbclient.h"

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

CDpClient *DpConnAndGetSessionID() {
  CDpClient *p_tcp = CDpClient::Create();
  if (p_tcp) {
    vzconn::CInetAddr c_addr(g_dp_addr, g_dp_port);
    bool b_ret = p_tcp->Connect(&c_addr, false, true, DEF_TIMEOUT_MSEC);
    if (false == b_ret) {
      LOG(L_ERROR) << "connect to server failed " << c_addr.ToString();
      delete p_tcp;
      p_tcp = NULL;
      return NULL;
    }

#if 1
    p_tcp->SendMessage(TYPE_GET_SESSION_ID,
                       "GET_SEESION_ID",
                       p_tcp->new_msg_id(),
                       "body_data",
                       strlen("body_data"),
                       NULL,
                       0);
    p_tcp->RunLoop(DEF_TIMEOUT_MSEC);
    if (p_tcp->get_session_id() < 0) {
      LOG(L_ERROR) << "get session id failed " << c_addr.ToString();
      delete p_tcp;
      p_tcp = NULL;
      return NULL;
    }
#endif
  }
  return p_tcp;
}

// b_can_reconn 断网时允许重连; poll不允许重连
CDpClient* GetDpCli(bool b_can_reconn =true) {
  CDpClient* p_tcp = NULL;
  p_tcp = (CDpClient*)g_dp_tls.GetValue();
  if (p_tcp) {
    if (p_tcp->isClose()) {        // 断开链接
      g_dp_tls.SetValue(NULL);

      delete p_tcp;
      p_tcp = NULL;
    }
  }

  if (p_tcp == NULL && b_can_reconn) {
    // 新建
    p_tcp = DpConnAndGetSessionID();
    if (p_tcp) {
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
}

EXPORT_DLL int DpClient_Start(int new_thread) {
  TlsKey tk = g_dp_tls.KeyAlloc();
  if (tk == TLS_NULL) {
    LOG(L_ERROR) << "alloc tls key failed.";
    return VZNETDP_FAILURE;
  }

  return VZNETDP_SUCCEED;
}

EXPORT_DLL void DpClient_Stop() {
  for (int32 i = 0; i < MAX_CLIS_PER_PROC; i++) {
    if (g_dp_client[i] != NULL) {
      delete g_dp_client[i];
    }
    g_dp_client[i] = NULL;
  }
}

EXPORT_DLL int DpClient_AddListenMessage(const char *method_set[],
    unsigned int  set_size) {
  int32 n_ret = VZNETDP_FAILURE;
  CDpClient* p_tcp = GetDpCli();
  if (!p_tcp) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }

  n_ret = DpClient_HdlAddListenMessage(p_tcp, method_set, set_size);
  return n_ret;
}

EXPORT_DLL int DpClient_RemoveListenMessage(const char* method_set[],
    unsigned int set_size) {
  int32 n_ret = 0;
  CDpClient* p_tcp = GetDpCli();
  if (!p_tcp) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }

  n_ret = DpClient_HdlRemoveListenMessage(p_tcp, method_set, set_size);
  return n_ret;
}

EXPORT_DLL int DpClient_SendDpMessage(const char    *method,
                                      unsigned char  channel_id,
                                      const char    *data,
                                      int            data_size) {
  int32 n_ret = 0;
  CDpClient* p_tcp = GetDpCli();
  if (!p_tcp) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }

  n_ret = p_tcp->SendMessage(TYPE_MESSAGE,
                             method,
                             p_tcp->get_msg_id(),
                             data,
                             data_size,
                             NULL,
                             NULL);
  if (n_ret <= 0) {
    return VZNETDP_FAILURE;
  }

  p_tcp->RunLoop(DEF_TIMEOUT_MSEC);
  if (p_tcp->get_ret_type() == TYPE_SUCCEED) {
    return VZNETDP_SUCCEED;
  }
  LOG(L_ERROR) << p_tcp->get_ret_type();
  return VZNETDP_FAILURE;
}

EXPORT_DLL unsigned int DpClient_SendDpRequest(const char *method,
    unsigned char             channel_id,
    const char               *data,
    int                       data_size,
    DpClient_MessageCallback  call_back,
    void                     *user_data,
    unsigned int              timeout) {
  int32 n_ret = 0;
  CDpClient* p_tcp = GetDpCli();
  if (!p_tcp) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }

  n_ret = p_tcp->SendMessage(TYPE_REQUEST,
                             method,
                             p_tcp->new_msg_id(),
                             data,
                             data_size,
                             call_back,
                             user_data);
  if (n_ret <= 0) {
    return VZNETDP_FAILURE;
  }

  p_tcp->RunLoop(timeout);
  if ((p_tcp->get_ret_type() == TYPE_REPLY) ||
      (p_tcp->get_ret_type() == TYPE_SUCCEED)) {
    return VZNETDP_SUCCEED;
  }
  LOG(L_ERROR) << p_tcp->get_ret_type();
  return VZNETDP_FAILURE;
}

EXPORT_DLL int DpClient_SendDpReply(const char    *method,
                                    unsigned char  channel_id,
                                    unsigned int   id,
                                    const char    *data,
                                    int            data_size) {
  int32 n_ret = 0;
  CDpClient* p_tcp = GetDpCli();
  if (!p_tcp) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }

  n_ret = p_tcp->SendMessage(TYPE_REPLY,
                             method,
                             id,
                             data,
                             data_size,
                             NULL,
                             NULL);
  if (n_ret <= 0) {
    return VZNETDP_FAILURE;
  }

  p_tcp->RunLoop(DEF_TIMEOUT_MSEC);
  if (p_tcp->get_ret_type() == TYPE_SUCCEED) {
    return VZNETDP_SUCCEED;
  }
  LOG(L_ERROR) << p_tcp->get_ret_type();
  return VZNETDP_FAILURE;
}

//////////////////////////////////////////////////////////////////////////
EXPORT_DLL DPPollHandle DpClient_CreatePollHandle() {
  return (DPPollHandle*)DpConnAndGetSessionID();
}

EXPORT_DLL void DpClient_ReleasePollHandle(DPPollHandle p_poll_handle) {
  if (p_poll_handle) {
    delete ((CDpClient*)p_poll_handle);
    p_poll_handle = NULL;
  }
}

EXPORT_DLL int DpClient_HdlAddListenMessage(const DPPollHandle p_poll_handle,
    const char *method_set[],
    unsigned int set_size) {
  int32 n_ret = VZNETDP_FAILURE;
  CDpClient* p_tcp = (CDpClient*)p_poll_handle;
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

  p_tcp->RunLoop(DEF_TIMEOUT_MSEC);
  if (p_tcp->get_ret_type() == TYPE_SUCCEED) {
    return VZNETDP_SUCCEED;
  }
  LOG(L_ERROR) << p_tcp->get_ret_type();
  return VZNETDP_FAILURE;
}

EXPORT_DLL int DpClient_HdlRemoveListenMessage(const DPPollHandle p_poll_handle,
    const char *method_set[],
    unsigned int set_size) {
  int32 n_ret = VZNETDP_FAILURE;
  CDpClient* p_tcp = (CDpClient*)p_poll_handle;
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
  if (p_tcp->get_ret_type() == TYPE_SUCCEED) {
    return VZNETDP_SUCCEED;
  }
  LOG(L_ERROR) << p_tcp->get_ret_type();
  return VZNETDP_FAILURE;
}

EXPORT_DLL int DpClient_PollDpMessage(const DPPollHandle       p_poll_handle,
                                      DpClient_MessageCallback call_back,
                                      void                    *user_data,
                                      unsigned int             timeout) {
  int32 n_ret = 0;
  // 此处判断断开不重连,目的是为了让注册method重连
  CDpClient* p_tcp = (CDpClient*)p_poll_handle;
  if (!p_tcp) {
    // LOG(L_ERROR) << "get client failed.";
    return VZNETDP_FAILURE;
  }
  if (p_tcp->isClose()) {        // 断开链接
    return VZNETDP_FAILURE;
  }

  p_tcp->Reset(call_back, user_data);
  n_ret = p_tcp->PollRunLoop(timeout);
  return VZNETDP_SUCCEED;
}

EXPORT_DLL void * DpClient_GetEvtLoopFromPoll(const DPPollHandle p_poll_handle) {
  CDpClient* p_tcp = (CDpClient*)p_poll_handle;
  if (!p_tcp) {
    LOG(L_ERROR) << "get client failed.";
    return NULL;
  }

  return p_tcp->GetEvtLoop();
}


//////////////////////////////////////////////////////////////////////////
static char                 g_kvdb_addr[64] = {0};
static unsigned short       g_kvdb_port     = 0;
static CKvdbClient         *g_kvdb_client   = NULL;

CKvdbClient* GetKvdbCli(bool b_can_reconn = true) {
  if (g_kvdb_client) {
    if (g_kvdb_client->isClose()) {        // 断开链接
      delete g_kvdb_client;
      g_kvdb_client = NULL;
    }
  }

  if (b_can_reconn && g_kvdb_client == NULL) {
    // 新建
    g_kvdb_client = CKvdbClient::Create();
    if (g_kvdb_client == NULL) {
      LOG(L_ERROR) << "create client failed.";
      return NULL;
    }

    // 链接
    vzconn::CInetAddr c_addr(g_kvdb_addr, g_kvdb_port);
    bool b_ret = g_kvdb_client->Connect(&c_addr,
                                        false,
                                        true,
                                        DEF_TIMEOUT_MSEC);
    if (false == b_ret) {
      LOG(L_ERROR) << "connect to server failed " << c_addr.ToString();
      delete g_kvdb_client;
      g_kvdb_client = NULL;
      return NULL;
    }
  }
  return g_kvdb_client;
}


EXPORT_DLL int Kvdb_Start(const char *server, unsigned short port) {
  g_kvdb_port = port;

  memset(g_kvdb_addr, 0, 63);
  strncpy(g_kvdb_addr, server, 63);

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
  CKvdbClient* p_cli = GetKvdbCli();
  if (p_cli == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (p_cli->SetKey(p_key, n_key, p_value, n_value)) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int Kvdb_GetKey(const char          *p_key,
                           int                  n_key,
                           Kvdb_GetKeyCallback  p_callback,
                           void                *p_usr_arg) {
  CKvdbClient* p_cli = GetKvdbCli();
  if (p_cli == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (p_cli->GetKey(p_key, n_key, p_callback, p_usr_arg, false)) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int Kvdb_GetKeyAbsolutely(const char           *p_key,
                                     int                   n_key,
                                     Kvdb_GetKeyCallback   p_callback,
                                     void                 *p_usr_arg) {
  CKvdbClient* p_cli = GetKvdbCli();
  if (p_cli == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (p_cli->GetKey(p_key, n_key, p_callback, p_usr_arg, true)) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int Kvdb_GetKeyToBuffer(const char   *p_key,
                                   int           n_key,
                                   char         *p_value,
                                   unsigned int  n_value) {
  CKvdbClient* p_cli = GetKvdbCli();
  if (p_cli == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (p_cli->GetKey(p_key, n_key, p_value, n_value, false)) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int Kvdb_GetKeyAbsolutelyToBuffer(const char   *p_key,
    int           n_key,
    char         *p_value,
    unsigned int  n_value) {
  CKvdbClient* p_cli = GetKvdbCli();
  if (p_cli == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (p_cli->GetKey(p_key, n_key, p_value, n_value, true)) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int Kvdb_DeleteKey(const char *p_key, int n_key) {
  CKvdbClient* p_cli = GetKvdbCli();
  if (p_cli == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (p_cli->Delete(p_key, n_key)) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int Kvdb_BackupDatabase() {
  CKvdbClient* p_cli = GetKvdbCli();
  if (p_cli == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (p_cli->BackupDatabase()) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int Kvdb_RestoreDatabase() {
  CKvdbClient* p_cli = GetKvdbCli();
  if (p_cli == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (p_cli->RestoreDatabase()) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

//////////////////////////////////////////////////////////////////////////
static char                 g_skvdb_addr[64] = { 0 };
static unsigned short       g_skvdb_port = 0;
static CKvdbClient         *g_skvdb_client = NULL;

CKvdbClient* GetSKvdbCli(bool b_can_reconn = true) {
  if (g_skvdb_client) {
    if (g_skvdb_client->isClose()) {        // 断开链接
      delete g_skvdb_client;
      g_skvdb_client = NULL;
    }
  }

  if (b_can_reconn && g_skvdb_client == NULL) {
    // 新建
    g_skvdb_client = CKvdbClient::Create();
    if (g_skvdb_client == NULL) {
      LOG(L_ERROR) << "create client failed.";
      return NULL;
    }

    // 链接
    vzconn::CInetAddr c_addr(g_skvdb_addr, g_skvdb_port);
    bool b_ret = g_skvdb_client->Connect(&c_addr,
                                         false,
                                         true,
                                         DEF_TIMEOUT_MSEC);
    if (false == b_ret) {
      LOG(L_ERROR) << "connect to server failed " << c_addr.ToString();
      delete g_skvdb_client;
      g_skvdb_client = NULL;
      return NULL;
    }
  }
  return g_skvdb_client;
}


EXPORT_DLL int SKvdb_Start(const char *server, unsigned short port) {
  g_skvdb_port = port;

  memset(g_skvdb_addr, 0, 63);
  strncpy(g_skvdb_addr, server, 63);

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
  CKvdbClient* p_cli = GetSKvdbCli();
  if (p_cli == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (p_cli->SetKey(p_key, n_key, p_value, n_value)) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int SKvdb_GetKey(const char          *p_key,
                            int                  n_key,
                            Kvdb_GetKeyCallback  p_callback,
                            void                *p_usr_arg) {
  CKvdbClient* p_cli = GetSKvdbCli();
  if (p_cli == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (p_cli->GetKey(p_key, n_key, p_callback, p_usr_arg, false)) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int SKvdb_GetKeyAbsolutely(const char           *p_key,
                                      int                   n_key,
                                      Kvdb_GetKeyCallback   p_callback,
                                      void                 *p_usr_arg) {
  CKvdbClient* p_cli = GetSKvdbCli();
  if (p_cli == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (p_cli->GetKey(p_key, n_key, p_callback, p_usr_arg, true)) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int SKvdb_GetKeyToBuffer(const char   *p_key,
                                    int           n_key,
                                    char         *p_value,
                                    unsigned int  n_value) {
  CKvdbClient* p_cli = GetSKvdbCli();
  if (p_cli == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (p_cli->GetKey(p_key, n_key, p_value, n_value, false)) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int SKvdb_GetKeyAbsolutelyToBuffer(const char   *p_key,
    int           n_key,
    char         *p_value,
    unsigned int  n_value) {
  CKvdbClient* p_cli = GetSKvdbCli();
  if (p_cli == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (p_cli->GetKey(p_key, n_key, p_value, n_value, true)) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int SKvdb_DeleteKey(const char *p_key, int n_key) {
  CKvdbClient* p_cli = GetSKvdbCli();
  if (p_cli == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (p_cli->Delete(p_key, n_key)) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int SKvdb_BackupDatabase() {
  CKvdbClient* p_cli = GetSKvdbCli();
  if (p_cli == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (p_cli->BackupDatabase()) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}

EXPORT_DLL int SKvdb_RestoreDatabase() {
  CKvdbClient* p_cli = GetSKvdbCli();
  if (p_cli == NULL) {
    return KVDB_RET_FAILURE;
  }

  if (p_cli->RestoreDatabase()) {
    return KVDB_RET_SUCCEED;
  }
  return KVDB_RET_FAILURE;
}
