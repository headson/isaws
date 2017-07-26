/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "cevttcpserver.h"

#include "vzbase/helper/stdafx.h"
#include "cevttcpclient.h"
#include "vzconn/base/connhead.h"

namespace vzconn {

CEvtTcpServer::CEvtTcpServer(const EVT_LOOP      *p_loop,
                             CClientInterface    *cli_hdl,
                             CTcpServerInterface *srv_hdl)
  : VSocket(cli_hdl)
  , p_evt_loop_(p_loop)
  , c_evt_accept_()
  , cli_hdl_ptr_(cli_hdl)
  , srv_handle_ptr_(srv_hdl) {
}

CEvtTcpServer *CEvtTcpServer::Create(const EVT_LOOP      *p_loop,
                                     CClientInterface    *cli_hdl,
                                     CTcpServerInterface *srv_hdl) {
  if (NULL == p_loop || p_loop->get_event() == NULL) {
    LOG(L_ERROR) << "param is failed.";
    return NULL;
  }
  if (NULL == srv_hdl) {
    LOG(L_ERROR) << "param is failed.";
    return NULL;
  }

  return (new CEvtTcpServer(p_loop, cli_hdl, srv_hdl));
}

CEvtTcpServer::~CEvtTcpServer() {
  c_evt_accept_.Stop();

  Close();
}

bool CEvtTcpServer::Open(const CInetAddr *p_addr,
                         bool            b_block,
                         bool            b_reuse) {
  if (NULL == p_evt_loop_ ) {
    LOG(L_ERROR) << "event loop is NULL.";
    return false;
  }
  if (NULL == p_addr || p_addr->IsNull()) {
    LOG(L_ERROR) << "param is error.";
    return false;
  }

  SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
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

  int32 ret = -1;
  // 绑定端口
  if (false == p_addr->IsNull()) {
    ret = ::bind(GetSocket(),
                 (sockaddr *)p_addr->GetAddr(),
                 (socklen_t)sizeof(sockaddr));
    if (0 != ret) {
      LOG(L_ERROR) << "bind failed." << error_no();
      return false;
    }
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
  LOG(L_INFO) << "open tcp server success "<<p_addr->ToString();
  return true;
}

int32 CEvtTcpServer::EvtAccept(SOCKET          fd,
                               short           events,
                               const void      *p_usr_arg) {
  int32 n_ret = -1;
  if (p_usr_arg) {
    n_ret = ((CEvtTcpServer*)p_usr_arg)->OnAccept();
    if (n_ret < 0) {
    }
  }
  return n_ret;
}

int32 CEvtTcpServer::OnAccept() {
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

  CEvtTcpClient *cli_ptr = CEvtTcpClient::Create(p_evt_loop_,
                           cli_hdl_ptr_);
  if (cli_ptr) {
    bool b_open = false;
    if (srv_handle_ptr_) {
      b_open = srv_handle_ptr_->HandleNewConnection(this, cli_ptr);
    }

    if (b_open) {
      cli_ptr->Open(s, true);
    } else {
      delete cli_ptr;
      evutil_closesocket(s);
      s = INVALID_SOCKET;
    }
  }
  return 0;
}

}  // namespace vzconn
