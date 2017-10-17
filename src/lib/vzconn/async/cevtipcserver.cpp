/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : 
************************************************************************/
#include "cevtipcserver.h"

#include <stddef.h>

#include "vzbase/helper/stdafx.h"
#include "cevtipcclient.h"

namespace vzconn {

CEvtIpcServer::CEvtIpcServer(const EVT_LOOP       *p_loop,
                             CClientInterface     *cli_hdl,
                             CTcpServerInterface  *srv_hdl)
  : CEvtTcpServer(p_loop, cli_hdl, srv_hdl) {
}

CEvtIpcServer *CEvtIpcServer::Create(const EVT_LOOP *p_loop,
                                     CClientInterface *cli_hdl,
                                     CTcpServerInterface *srv_hdl) {
  if (NULL == p_loop || p_loop->get_event() == NULL) {
    LOG(L_ERROR) << "param is failed.";
    return NULL;
  }
  if (NULL == cli_hdl || NULL == srv_hdl) {
    LOG(L_ERROR) << "param is failed.";
    return NULL;
  }

  return (new CEvtIpcServer(p_loop, cli_hdl, srv_hdl));
}

CEvtIpcServer::~CEvtIpcServer() {
}

bool CEvtIpcServer::Open(const CInetAddr *p_addr,
                          bool             b_block,
                          bool             b_reuse) {
  if (NULL == p_evt_loop_ ) {
    LOG(L_ERROR) << "event loop is NULL.";
    return false;
  }
  if (NULL == p_addr || p_addr->IsNull()) {
    LOG(L_ERROR) << "param is error.";
    return false;
  }

  int32 ret = -1;
  socklen_t len = 0;
#ifdef _LINUX
  SOCKET s = socket(AF_UNIX, SOCK_STREAM, 0);
  if (INVALID_SOCKET == s) {
    LOG(L_ERROR) << "socket failed.";
    return false;
  }
  SetSocket(s);

  //设置异步模式
  if (false == b_block) {
    set_socket_nonblocking(GetSocket());
  }

  char ipc_addr[32] = {0};
  p_addr->ToIpcAddr(ipc_addr, 32);

  struct sockaddr_un un;
  memset(&un, 0, sizeof(un));
  LOG(L_ERROR) << "use ipc to create server."<<ipc_addr;

  unlink(ipc_addr);           /* in case it already exists */
  un.sun_family = AF_UNIX;
  strcpy(un.sun_path, ipc_addr);
  len = offsetof(struct sockaddr_un, sun_path)
                  + strlen(ipc_addr);

  ret = ::bind(s, (struct sockaddr*)&un, len);
#else  // AF_UNIX
  SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
  if (INVALID_SOCKET == s) {
    LOG(L_ERROR) << "socket failed.";
    return false;
  }
  SetSocket(s);

  //设置异步模式
  if (false == b_block) {
    set_socket_nonblocking(GetSocket());
  }

  //设置端口复用
  if (b_reuse) {
    int32 val = 1;
    SetOption(SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int32));
  }

  // 绑定端口
  len = sizeof(sockaddr);
  ret = ::bind(GetSocket(), (sockaddr *)p_addr->GetAddr(), len);
#endif  // AF_UNIX
  if (0 != ret) {
    perror("bind failed.\n");
    LOG(L_ERROR) << "bind failed." << error_no();
    return false;
  }

  ret = listen(GetSocket(), 10);
  if (0 != ret) {
    LOG(L_ERROR) << "listen failed." << error_no();
    return false;
  }

  // 关联SOCKET的READ事件
  c_evt_accept_.Init(p_evt_loop_, EvtAccept, this);
  ret = c_evt_accept_.Start(GetSocket(), EVT_READ | EVT_PERSIST);
  if (0 != ret) {
    LOG(L_ERROR) << "set recv event failed." << error_no();
    return false;
  }
  return true;
}

int32 CEvtIpcServer::OnAccept() {
  CInetAddr c_addr;
  socklen_t n_addr = sizeof(sockaddr);
  SOCKET s = accept(GetSocket(),
                    (sockaddr*)c_addr.GetAddr(),
                    &n_addr);
  if (INVALID_SOCKET == s)  {
    LOG(L_ERROR) << "accept new socket failed.";
    if (srv_handle_ptr_) {
      srv_handle_ptr_->HandleServerClose(this);
    }
    return -1;
  }

  CEvtIpcClient *cli_ptr = CEvtIpcClient::Create(p_evt_loop_,
                           cli_hdl_ptr_);
  if (cli_ptr) {
    bool b_open = false;
    if (srv_handle_ptr_) {
      b_open = srv_handle_ptr_->HandleNewConnection(this, cli_ptr);
    }

    if (b_open) {
      cli_ptr->Open(s, true);
    } else {
      closesocket(s);
      s = INVALID_SOCKET;
    }
  }
  return 0;
}

}  // namespace vzconn
