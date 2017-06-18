/************************************************************************/
/* Author      : SoberPeng 2017-06-18
/* Description :
/************************************************************************/
#include "cdpserver.h"

#include "stdafx.h"

CDpServer::CDpServer()
  : c_event_()
  , p_ipc_srv_(NULL)
  , p_tcp_srv_(NULL) {
}

CDpServer::~CDpServer() {
}

int32 CDpServer::Start(const char *s_ip, uint16 n_port) {
  int32 n_ret = c_event_.Start();
  if (0 != n_ret) {
    LOG(L_ERROR) << "event start failed.";
    return n_ret;
  }

  CInetAddr c_addr(s_ip, n_port);
  p_ipc_srv_ = CEvtIpcServer::Create(&c_event_, this, this);
  if (NULL == p_ipc_srv_) {
    LOG(L_ERROR) << "create ipc server failed.";
    return -1;
  }

  p_tcp_srv_ = CEvtTcpServer::Create(&c_event_, this, this);
  if (NULL == p_tcp_srv_) {
    LOG(L_ERROR) << "create tcp server failed.";
    return -1;
  }
  return 0;
}

void CDpServer::Stop() {
  if (p_ipc_srv_) {
    delete p_ipc_srv_;
    p_ipc_srv_ = NULL;
  }
  if (p_tcp_srv_) {
    delete p_tcp_srv_;
    p_tcp_srv_ = NULL;
  }
  c_event_.Stop();
}

int32 CDpServer::RunLoop() {
  return c_event_.RunLoop();
}

int32 CDpServer::HandleNewConnection(void *p_srv, VSocket *new_sock) {
  return 0;
}

int32 CDpServer::HandleRecvPacket(void        *p_cli,
                                  const void  *p_data,
                                  uint32       n_data) {
  return 0;
}

int32 CDpServer::HandleSendPacket(void *p_cli) {
  return 0;
}

void CDpServer::HandleClose(void *p_cli) {

}
