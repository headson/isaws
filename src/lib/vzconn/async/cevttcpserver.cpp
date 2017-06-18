/************************************************************************/
/* Author      : Sober.Peng 17-06-15
/* Description :
/************************************************************************/
#include "cevttcpserver.h"

#include "stdafx.h"
#include "cevttcpclient.h"
#include "vzconn/base/pkghead.h"

CEvtTcpServer::CEvtTcpServer(const EVT_LOOP      *p_loop,
                             CClientInterface    *cli_hdl,
                             CTcpServerInterface *srv_hdl)
  : VSocket()
  , p_evt_loop_(p_loop)
  , cli_handle_ptr_(cli_hdl)
  , srv_handle_ptr_(srv_hdl)
  , c_evt_accept_() {
  SetNetHeadParseCallback(vz_head_parse);
  SetNetHeadPacketCallback(vz_head_packet);
}

CEvtTcpServer *CEvtTcpServer::Create(const EVT_LOOP      *p_loop,
                                     CClientInterface    *cli_hdl,
                                     CTcpServerInterface *srv_hdl) {
  if (NULL == p_loop || p_loop->get_event() == NULL) {
    LOG(L_ERROR) << "param is failed.";
    return NULL;
  }
  if (NULL == cli_hdl || NULL == srv_hdl) {
    LOG(L_ERROR) << "param is failed.";
    return NULL;
  }

  return (new CEvtTcpServer(p_loop, cli_hdl, srv_hdl));
}

CEvtTcpServer::~CEvtTcpServer() {
  c_evt_accept_.Stop();

  Close();
}

int32 CEvtTcpServer::Open(const CInetAddr*  p_addr,
                          bool              b_block,
                          bool              b_reuse) {
  if (NULL == p_evt_loop_ ) {
    LOG(L_ERROR) << "event loop is NULL.";
    return -1;
  }
  if (NULL == p_addr) {
    LOG(L_ERROR) << "param is error.";
    return -2;
  }

  SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (INVALID_SOCKET == s) {
    LOG(L_ERROR) << "socket failed.";
    return -1;
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
      return ret;
    }
  }

  ret = listen(GetSocket(), 10);
  if (0 != ret) {
    LOG(L_ERROR) << "listen failed." << error_no();
    return ret;
  }

  // 关联SOCKET的READ事件
  c_evt_accept_.Init(p_evt_loop_, EvtAccept, this);
  ret = c_evt_accept_.Start(GetSocket(), EVT_READ | EVT_PERSIST);
  if (0 != ret) {
    LOG(L_ERROR) << "set recv event failed." << error_no();
    return ret;
  }

  return ret;
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
      srv_handle_ptr_->HandleClose(this);
    }
    return -1;
  }

  CEvtTcpClient *cli_ptr = CEvtTcpClient::Create(p_evt_loop_,
                           cli_handle_ptr_);
  if (cli_ptr) {
    cli_ptr->SetNetHeadParseCallback(GetNetHeadParseCallback());
    cli_ptr->SetNetHeadPacketCallback(GetNetHeadPacketCallback());
    cli_ptr->Open(s, true);

    if (srv_handle_ptr_) {
      srv_handle_ptr_->HandleNewConnection(this, cli_ptr);
    }
  }
  return 0;
}
