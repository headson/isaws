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

static VTls                 g_cli_tls;                      //
static vzconn::CInetAddr    g_remote_addr;                  //
static CTcpClient*          g_tcp_client[MAX_CLIS_PER_PROC];

//////////////////////////////////////////////////////////////////////////
static int DpClientSendMessage(CTcpClient    *p_tcp,
                               unsigned char  n_type,
                               const char    *method,
                               unsigned char  channel_id,
                               const char    *data,
                               int            data_size,
                               bool           b_wait_recv) {
  // dp head
  DpMessage c_dp_msg;
  int32 n_dp_msg = CTcpClient::EncDpMsg(&c_dp_msg,
                                        n_type,
                                        channel_id,
                                        method,
                                        p_tcp->new_msg_seq(),
                                        data_size);
  if (n_dp_msg < 0) {
    LOG(L_ERROR) << "create dp msg head failed." << n_dp_msg;
    return VZNETDP_FAILURE;
  }

  // body
  iovec iov[2];
  iov[0].iov_base = &c_dp_msg;
  iov[0].iov_len = n_dp_msg;
  iov[1].iov_base = (void*)data;
  iov[1].iov_len = data_size;

  p_tcp->Reset(b_wait_recv);
  int32 n_ret = p_tcp->AsyncWrite(
                  iov, 2, FLAG_DISPATCHER_MESSAGE);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "async write failed " << n_dp_msg + data_size;
    return n_ret;
  }
  return n_ret;
}

CTcpClient* GetTcpCli() {
  CTcpClient* p_tcp = NULL;
  p_tcp = (CTcpClient*)g_cli_tls.GetValue();
  if (p_tcp) {
#ifndef _WIN32
    struct tcp_info info;
    int len = sizeof(info);
    p_tcp->GetOption(IPPROTO_TCP, TCP_INFO, &info, &len);
    if ((info.tcpi_state == TCP_ESTABLISHED)) {
      // 
    } else {
      // 断开链接
      g_cli_tls.SetValue(NULL);
      delete p_tcp; p_tcp = NULL;
    }
#endif
  }

  if (p_tcp == NULL) {
    // 新建
    p_tcp = CTcpClient::Create();
    if (p_tcp) {
      int32 n_ret = p_tcp->Connect(&g_remote_addr, false, true, DEF_TIMEOUT_MSEC);
      if (n_ret < 0) {
        LOG(L_ERROR) << "connect to server failed "<<g_remote_addr.ToString();
        delete p_tcp;
        p_tcp = NULL;
        return NULL;
      }

      DpClientSendMessage(p_tcp,
                          TYPE_GET_SESSION_ID,
                          NULL,
                          0,
                          NULL,
                          0,
                          true);
      p_tcp->RunLoop(DEF_TIMEOUT_MSEC);
      if (p_tcp->get_chn_id() <= 0) {
        LOG(L_ERROR) << "get session id failed " << g_remote_addr.ToString();
        delete p_tcp;
        p_tcp = NULL;
        return NULL;
      }
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
  int32 n_ret = 0;
  CTcpClient* p_tcp = GetTcpCli();
  if (!p_tcp) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }

  uint32 n_data = 0;
  for (uint32 i = 0; i < set_size; i++) {
    if (method_set[i] == NULL) {
      continue;
    }
    if (method_set[i][0] == '\0') {
      continue;
    }
    uint32 n_method = strlen(method_set[i]) + 1;
    if (n_method >= MAX_METHOD_SIZE) {
      continue;
    }
    n_data += MAX_METHOD_SIZE;
  }

  DpMessage c_dp_msg;
  int32 n_dp_msg = CTcpClient::EncDpMsg(&c_dp_msg,
                                        TYPE_ADD_MESSAGE,
                                        0,
                                        NULL,
                                        p_tcp->new_msg_seq(),
                                        n_data);

  p_tcp->Reset();
  n_ret = p_tcp->ListenMessage(&c_dp_msg,
                               method_set,
                               set_size,
                               FLAG_ADDLISTEN_MESSAGE);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "async write failed " << n_ret;
    return n_ret;
  }

  p_tcp->RunLoop(DEF_TIMEOUT_MSEC);
  return p_tcp->get_resp_type();
}

EXPORT_DLL int DpClient_RemoveListenMessage(const char* method_set[],
    unsigned int set_size) {
  int32 n_ret = 0;
  CTcpClient* p_tcp = GetTcpCli();
  if (!p_tcp) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }

  uint32 n_data = 0;
  for (uint32 i = 0; i < set_size; i++) {
    if (method_set[i] == NULL) {
      continue;
    }
    if (method_set[i][0] == '\0') {
      continue;
    }
    uint32 n_method = strlen(method_set[i]) + 1;
    if (n_method >= MAX_METHOD_SIZE) {
      continue;
    }
    n_data += MAX_METHOD_SIZE;
  }

  DpMessage c_dp_msg;
  int32 n_dp_msg = CTcpClient::EncDpMsg(&c_dp_msg,
                                        TYPE_REMOVE_MESSAGE,
                                        0,
                                        NULL,
                                        p_tcp->new_msg_seq(),
                                        n_data);

  p_tcp->Reset();
  n_ret = p_tcp->ListenMessage(&c_dp_msg,
                               method_set,
                               set_size,
                               FLAG_REMOVELISTEN_MESSAGE);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "async write failed " << n_ret;
    return n_ret;
  }

  p_tcp->RunLoop(DEF_TIMEOUT_MSEC);
  return p_tcp->get_resp_type();
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

  n_ret = DpClientSendMessage(p_tcp,
                              TYPE_MESSAGE,
                              method,
                              channel_id,
                              data,
                              data_size,
                              true);
  if (n_ret <= 0) {
    return VZNETDP_FAILURE;
  }

  p_tcp->RunLoop(DEF_TIMEOUT_MSEC);
  return p_tcp->get_resp_type();
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

  n_ret = DpClientSendMessage(p_tcp,
                              TYPE_MESSAGE,
                              method,
                              channel_id,
                              data,
                              data_size,
                              true);
  if (n_ret <= 0) {
    return VZNETDP_FAILURE;
  }

  p_tcp->RunLoop(timeout);
  return p_tcp->get_resp_type();
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

  n_ret = DpClientSendMessage(p_tcp,
                              TYPE_MESSAGE,
                              method,
                              channel_id,
                              data,
                              data_size,
                              false);
  if (n_ret <= 0) {
    return VZNETDP_FAILURE;
  }

  p_tcp->RunLoop(DEF_TIMEOUT_MSEC);
  return p_tcp->get_send_all_buffer()
         ? VZNETDP_SUCCEED : VZNETDP_FAILURE;
}

EXPORT_DLL int DpClient_PollRecvMessage(DpClient_MessageCallback call_back,
                                        void                    *user_data,
                                        unsigned int             timeout) {
  int32 n_ret = 0;
  CTcpClient* p_tcp = GetTcpCli();
  if (!p_tcp) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }

  p_tcp->Reset(false);
  p_tcp->PollRunLoop(DEF_TIMEOUT_MSEC);
  return ((p_tcp->get_recv_one_packet() > 0)
          ? VZNETDP_SUCCEED : VZNETDP_FAILURE);
}

