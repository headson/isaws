/************************************************************************/
/* Author      : Sober.Peng 17-06-20
/* Description :
/************************************************************************/
#include "dpclient_c.h"

#include "stdafx.h"
#include "dispatcher/base/dphead.h"

#include "vzconn/async/clibevent.h"
#include "vzconn/sync/ctcpclient.h"

// TLS
#ifdef WIN32
#include <process.h>
typedef DWORD               TlsKey;
#define TLS_NULL            0
#else
#include <pthread.h>
typedef pthread_key_t       TlsKey;
#define TLS_NULL            -1
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

class CClientProcess : public vzconn::CClientInterface {
 public:
  virtual int32 HandleRecvPacket(void       *p_cli,
                                 const void *p_data,
                                 uint32      n_data);
  virtual int32 HandleSendPacket(void *p_cli) {
    return 0;
  }
  virtual void  HandleClose(void *p_cli) {
  }
};

static VTls                 g_cli_tls;                      //
static vzconn::EVT_LOOP     g_evt_loop;                     //
static vzconn::CInetAddr    g_remote_addr;                  //
typedef struct _TagTcpCli {
  vzconn::CTcpClient*       p_cli;
  CClientProcess            c_proc;
  uint32                    n_msg_seq;
  DpClient_MessageCallback  p_callback;
  void                      *p_usr_arg;
} TagTcpCli;
static TagTcpCli            g_tcp_client[MAX_CLIS_PER_PROC];

int32 CClientProcess::HandleRecvPacket(void       *p_cli,
                                       const void *p_data,
                                       uint32      n_data) {
  if (!p_cli || !p_data || n_data == 0) {
    return -1;
  }

  TagTcpCli *p_tcp = (TagTcpCli*)p_cli;

  TagDpMsg dp_msg;
  if (DecDpMsg(&dp_msg, p_data, n_data) < 0) {
    return -2;
  }
  if (dp_msg.id == p_tcp->n_msg_seq) {
  }

  char *p_body = (char*)p_data + NetHeadSize() + sizeof(dp_msg);
  p_tcp->p_callback(&dp_msg, p_body);
  return 0;
}

TagTcpCli* GetTcpCli() {
  TagTcpCli* p_tcp = NULL;
  p_tcp = (TagTcpCli*)g_cli_tls.GetValue();
  if (p_tcp == NULL) {
    for (int32 i = 0; i < MAX_CLIS_PER_PROC; i++) {
      if (g_tcp_client[i].p_cli == NULL) {
        g_tcp_client[i].p_cli = vzconn::CTcpClient::Create(&g_evt_loop,
                                &g_tcp_client[i].c_proc);
        if (g_tcp_client[i].p_cli) {
          int32 n_ret = g_tcp_client[i].p_cli->Connect(&g_remote_addr, false, true, 5000);
          if (n_ret < 0) {
            LOG(L_ERROR) << "connect to server failed "<<g_remote_addr.ToString();
            delete g_tcp_client[i].p_cli;
            g_tcp_client[i].p_cli = NULL;
            return NULL;
          }
        }
        g_tcp_client[i].p_callback = NULL;
        p_tcp = &g_tcp_client[i];
        break;
      }
    }
    g_cli_tls.SetValue(p_tcp);
  }
  
  p_tcp->p_callback = NULL;
  p_tcp->p_usr_arg  = NULL;
  p_tcp->p_cli->ResetEvent();
  return p_tcp;
}

EXPORT_DLL void DpClient_Init(const char* ip_addr, unsigned short port) {
  g_remote_addr.SetIP(ip_addr);
  g_remote_addr.SetPort(port);

  for (int32 i = 0; i < MAX_CLIS_PER_PROC; i++) {
    g_tcp_client[i].p_cli      = NULL;
    g_tcp_client[i].p_callback = NULL;
    g_tcp_client[i].n_msg_seq  = 0;
  }
}

EXPORT_DLL int DpClient_Start(int new_thread) {
  int32 n_ret = g_evt_loop.Start();
  if (n_ret < 0) {
    LOG(L_ERROR) << "dp client's event loop start failed.";
    return VZNETDP_FAILURE;
  }
  TlsKey tk = g_cli_tls.KeyAlloc();
  if (tk == TLS_NULL) {
    LOG(L_ERROR) << "alloc tls key failed.";
    return VZNETDP_FAILURE;
  }

  return VZNETDP_SUCCEED;
}

EXPORT_DLL void DpClient_Stop() {
  for (int32 i = 0; i < MAX_CLIS_PER_PROC; i++) {
    if (g_tcp_client[i].p_cli != NULL) {
      delete g_tcp_client[i].p_cli;
    }
    g_tcp_client[i].p_cli      = NULL;
    g_tcp_client[i].p_callback = NULL;
  }
  g_evt_loop.Stop();
}

EXPORT_DLL int DpClient_SendDpMessage(const char    *method,
                                      unsigned char  channel_id,
                                      const char    *data,
                                      int            data_size) {
  int32 n_ret = 0;
  TagTcpCli* p_tcp = GetTcpCli();
  if (!p_tcp || !p_tcp->p_cli) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }
  // dp head
  TagDpMsg c_dp_msg;
  int32 n_dp_msg = EncDpMsg(&c_dp_msg,
                            TYPE_MESSAGE,
                            channel_id,
                            method,
                            ++p_tcp->n_msg_seq,
                            data_size);
  if (n_dp_msg < 0) {
    LOG(L_ERROR) << "create dp msg head failed." << n_dp_msg;
    return VZNETDP_FAILURE;
  }

  iovec iov[2];
  iov[0].iov_base = &c_dp_msg;
  iov[0].iov_len  = n_dp_msg;
  iov[1].iov_base = (void*)data;
  iov[1].iov_len  = data_size;

  n_ret = p_tcp->p_cli->AsyncWrite(
            iov, 2, FLAG_DISPATCHER_MESSAGE);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "async write failed "<<n_dp_msg+data_size;
    return n_ret;
  }

  for (uint32 i = 0; i < 1000; i+=2) {
    g_evt_loop.RunLoop(EVT_LOOP_NOBLOCK);
    if (p_tcp->p_cli->isSendAllBuffer()
        && p_tcp->p_cli->isRecvOnePacket()) {
      return VZNETDP_SUCCEED;
    }
    usleep(2 * 1000);
  }
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
  TagTcpCli* p_tcp = GetTcpCli();
  if (!p_tcp || !p_tcp->p_cli) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }
  // dp head
  TagDpMsg c_dp_msg;
  int32 n_dp_msg = EncDpMsg(&c_dp_msg,
                            TYPE_MESSAGE,
                            channel_id,
                            method,
                            ++p_tcp->n_msg_seq,
                            data_size);
  if (n_dp_msg < 0) {
    LOG(L_ERROR) << "create dp msg head failed." << n_dp_msg;
    return VZNETDP_FAILURE;
  }

  iovec iov[2];
  iov[0].iov_base = &c_dp_msg;
  iov[0].iov_len  = n_dp_msg;
  iov[1].iov_base = (void*)data;
  iov[1].iov_len  = data_size;

  p_tcp->p_callback = call_back;
  p_tcp->p_usr_arg  = user_data;
  n_ret = p_tcp->p_cli->AsyncWrite(
            iov, 2, FLAG_DISPATCHER_MESSAGE);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "async write failed "<<n_dp_msg+data_size;
    return n_ret;
  }

  for (uint32 i = 0; i < timeout; i+=2) {
    g_evt_loop.RunLoop(EVT_LOOP_NOBLOCK);
    if (p_tcp->p_cli->isSendAllBuffer()
        && p_tcp->p_cli->isRecvOnePacket()) {
      return VZNETDP_SUCCEED;
    }
    usleep(2 * 1000);
  }
  return VZNETDP_FAILURE;
}

EXPORT_DLL int DpClient_SendDpReply(const char    *method,
                                    unsigned char  channel_id,
                                    unsigned int   id,
                                    const char    *data,
                                    int            data_size) {
  int32 n_ret = 0;
  TagTcpCli* p_tcp = GetTcpCli();
  if (!p_tcp || !p_tcp->p_cli) {
    LOG(L_ERROR) << "get tls client failed.";
    return VZNETDP_FAILURE;
  }
  // dp head
  TagDpMsg c_dp_msg;
  int32 n_dp_msg = EncDpMsg(&c_dp_msg,
                            TYPE_MESSAGE,
                            channel_id,
                            method,
                            ++p_tcp->n_msg_seq,
                            data_size);
  if (n_dp_msg < 0) {
    LOG(L_ERROR) << "create dp msg head failed." << n_dp_msg;
    return VZNETDP_FAILURE;
  }

  iovec iov[2];
  iov[0].iov_base = &c_dp_msg;
  iov[0].iov_len  = n_dp_msg;
  iov[1].iov_base = (void*)data;
  iov[1].iov_len  = data_size;

  n_ret = p_tcp->p_cli->AsyncWrite(
            iov, 2, FLAG_DISPATCHER_MESSAGE);
  if (n_ret <= 0) {
    LOG(L_ERROR) << "async write failed "<<n_dp_msg+data_size;
    return n_ret;
  }

  for (uint32 i = 0; i < 1000; i+=2) {
    g_evt_loop.RunLoop(EVT_LOOP_NOBLOCK);
    if (p_tcp->p_cli->isSendAllBuffer()) {
      return VZNETDP_SUCCEED;
    }
    usleep(2 * 1000);
  }
  return VZNETDP_FAILURE;
}

