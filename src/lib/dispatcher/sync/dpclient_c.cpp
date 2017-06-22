/************************************************************************/
/* Author      : Sober.Peng 17-06-20
/* Description :
/************************************************************************/
#include "dpclient_c.h"

#include "stdafx.h"
#include "dispatcher/base/pkghead.h"
#include "dispatcher/sync/cdpclient.h"

#include "vzconn/async/clibevent.h"
#include "vzconn/async/cevttcpclient.h"

// TLS
#ifdef WIN32
#include <process.h>
typedef DWORD               TlsKey;
#define TLS_NULL            0
#else
#include <pthread.h>
typedef pthread_key_t       TlsKey;
#define TLS_NULL            -1

#include <netinet/tcp.h>
#endif

namespace dp {

class VTls {
 public:
  VTls()
    : tls_key_(TLS_NULL) {
  }
  ~VTls() {
    KeyFree();
  }

  // ��������ʱ����һ�Σ�����ؼ���
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

  // true����,false������
  bool   IsReady() {
    if (TLS_NULL != tls_key_) {
      return true;
    }
    return false;
  }

  // �߳��е��ã������ؼ��ֺ�˽�����ݶ�Ӧָ��
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
  // �߳��е��ã���ȡ�ؼ��ֹ������߳�˽������
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

static VTls                 g_cli_tls;                      //
static vzconn::CInetAddr    g_remote_addr;                  //
static CTcpClient*          g_tcp_client[MAX_CLIS_PER_PROC];

CTcpClient* GetTcpCli() {
  CTcpClient* p_tcp = NULL;
  p_tcp = (CTcpClient*)g_cli_tls.GetValue();
  if (p_tcp) {
    if (p_tcp->isClose()) {        // �Ͽ�����
      g_cli_tls.SetValue(NULL);

      delete p_tcp;
      p_tcp = NULL;
    }
  }

  if (p_tcp == NULL) {
    // �½�
    p_tcp = CTcpClient::Create();
    if (p_tcp) {
      bool b_ret = p_tcp->Connect(&g_remote_addr, false, true, DEF_TIMEOUT_MSEC);
      if (false == b_ret) {
        LOG(L_ERROR) << "connect to server failed "<<g_remote_addr.ToString();
        delete p_tcp;
        p_tcp = NULL;
        return NULL;
      }

#if 1
      p_tcp->SendMessage(TYPE_GET_SESSION_ID,
                         "GET_SEESION_ID",
                         0,
                         "body_data",
                         strlen("body_data"),
                         NULL,
                         0);
      p_tcp->RunLoop(DEF_TIMEOUT_MSEC);
      if (p_tcp->get_session_id() < 0) {
        LOG(L_ERROR) << "get session id failed " << g_remote_addr.ToString();
        delete p_tcp;
        p_tcp = NULL;
        return NULL;
      }
#endif
      g_cli_tls.SetValue(p_tcp);
    }
  }
  return p_tcp;
}

EXPORT_DLL void DpClient_Init(const char* ip_addr, unsigned short port) {
  g_remote_addr.SetIP(ip_addr);
  g_remote_addr.SetPort(port);

  for (int32 i = 0; i < MAX_CLIS_PER_PROC; i++) {
    g_tcp_client[i] = NULL;
  }
}

EXPORT_DLL int DpClient_Start(int new_thread) {
  TlsKey tk = g_cli_tls.KeyAlloc();
  if (tk == TLS_NULL) {
    LOG(L_ERROR) << "alloc tls key failed.";
    return VZNETDP_FAILURE;
  }

  return VZNETDP_SUCCEED;
}

EXPORT_DLL void DpClient_Stop() {
  for (int32 i = 0; i < MAX_CLIS_PER_PROC; i++) {
    if (g_tcp_client[i] != NULL) {
      delete g_tcp_client[i];
    }
    g_tcp_client[i] = NULL;
  }
}

EXPORT_DLL int DpClient_AddListenMessage(const char   *method_set[],
    unsigned int  set_size) {
  int32 n_ret = VZNETDP_FAILURE;
  CTcpClient* p_tcp = GetTcpCli();
  if (!p_tcp) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }

  p_tcp->Reset();
  n_ret = p_tcp->ListenMessage(TYPE_ADD_MESSAGE,
                               method_set,
                               set_size,
                               FLAG_REMOVELISTEN_MESSAGE);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "add message failed " << n_ret;
    return n_ret;
  }
  LOG(L_WARNING) << "add listen message "<<n_ret;

  p_tcp->RunLoop(DEF_TIMEOUT_MSEC);
  LOG(L_WARNING) << "add listen message "<<p_tcp->get_resp_type();
  return p_tcp->get_resp_type() == TYPE_SUCCEED
    ? VZNETDP_SUCCEED : VZNETDP_FAILURE;
}

EXPORT_DLL int DpClient_RemoveListenMessage(const char* method_set[],
    unsigned int set_size) {
  int32 n_ret = 0;
  CTcpClient* p_tcp = GetTcpCli();
  if (!p_tcp) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }

  p_tcp->Reset();
  n_ret = p_tcp->ListenMessage(TYPE_REMOVE_MESSAGE,
                               method_set,
                               set_size,
                               FLAG_REMOVELISTEN_MESSAGE);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "remove message failed " << n_ret;
    return n_ret;
  }

  p_tcp->RunLoop(DEF_TIMEOUT_MSEC);
  LOG(L_WARNING) << "remove listen message "<<p_tcp->get_resp_type();
  return p_tcp->get_resp_type() == TYPE_SUCCEED
    ? VZNETDP_SUCCEED : VZNETDP_FAILURE;
}

EXPORT_DLL int DpClient_SendDpMessage(const char    *method,
                                      unsigned char  channel_id,
                                      const char    *data,
                                      int            data_size) {
  int32 n_ret = 0;
  CTcpClient* p_tcp = GetTcpCli();
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
                             0);
  if (n_ret <= 0) {
    return VZNETDP_FAILURE;
  }

  p_tcp->RunLoop(DEF_TIMEOUT_MSEC);
  LOG(L_WARNING) << "send message" << p_tcp->get_resp_type();
  return p_tcp->get_resp_type() == TYPE_SUCCEED
    ? VZNETDP_SUCCEED : VZNETDP_FAILURE;
}

EXPORT_DLL unsigned int DpClient_SendDpRequest(const char *method,
    unsigned char             channel_id,
    const char               *data,
    int                       data_size,
    DpClient_MessageCallback  call_back,
    void                     *user_data,
    unsigned int              timeout) {
  int32 n_ret = 0;
  CTcpClient* p_tcp = GetTcpCli();
  if (!p_tcp) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }

  n_ret = p_tcp->SendMessage(TYPE_REQUEST,
                             method,
                             p_tcp->new_msg_id(),
                             data,
                             data_size,
                             NULL,
                             0);
  if (n_ret <= 0) {
    return VZNETDP_FAILURE;
  }
  p_tcp->SetCallback(call_back, user_data);

  p_tcp->RunLoop(timeout);
  LOG(L_WARNING) << "send request "<<p_tcp->get_resp_type();
  return p_tcp->get_resp_type() == TYPE_SUCCEED
    ? VZNETDP_SUCCEED : VZNETDP_FAILURE;
}

EXPORT_DLL int DpClient_SendDpReply(const char    *method,
                                    unsigned char  channel_id,
                                    unsigned int   id,
                                    const char    *data,
                                    int            data_size) {
  int32 n_ret = 0;
  CTcpClient* p_tcp = GetTcpCli();
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
                             0);
  if (n_ret <= 0) {
    return VZNETDP_FAILURE;
  }

  p_tcp->RunLoop(DEF_TIMEOUT_MSEC);
  LOG(L_WARNING) << "send replay "<<p_tcp->get_resp_type();
  return p_tcp->get_resp_type() == TYPE_SUCCEED 
    ? VZNETDP_SUCCEED : VZNETDP_FAILURE;
}

EXPORT_DLL int DpClient_PollDpMessage(DpClient_MessageCallback call_back,
                                        void                    *user_data,
                                        unsigned int             timeout) {
  int32 n_ret = 0;
  CTcpClient* p_tcp = GetTcpCli();
  if (!p_tcp) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }

  p_tcp->Reset();
  p_tcp->SetCallback(call_back, user_data);

  p_tcp->RunLoop(DEF_TIMEOUT_MSEC);
  return VZNETDP_SUCCEED;
}

}  // namespace dp