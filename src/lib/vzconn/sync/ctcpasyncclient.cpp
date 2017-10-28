/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "ctcpasyncclient.h"
#include "vzconn/base/basedefines.h"

#include "vzbase/helper/stdafx.h"

namespace vzconn {

CTcpAsyncClient::CTcpAsyncClient(const EVT_LOOP *p_loop, CClientInterface *cli_hdl)
  : CEvtTcpClient(p_loop, cli_hdl) {
  //LOG_INFO("%s[%d].0x%x.", __FUNCTION__, __LINE__, (uint32)this);
}

CTcpAsyncClient::~CTcpAsyncClient() {
  c_evt_recv_.Stop();
  c_evt_send_.Stop();

  Close();
  //LOG_INFO("%s[%d].0x%x.", __FUNCTION__, __LINE__, (uint32)this);
}

void CTcpAsyncClient::Remove() {
  c_evt_recv_.Stop();
  c_evt_send_.Stop();
}

CTcpAsyncClient* CTcpAsyncClient::Create(const EVT_LOOP   *p_loop,
    CClientInterface *cli_hdl) {
  //EVT_LOOP *p_evt_loop = const_cast<EVT_LOOP*>(p_loop);
  //if (NULL == p_evt_loop) {
  //  p_evt_loop = new EVT_LOOP();
  //  if (p_evt_loop) {
  //    p_evt_loop->Start();
  //  }
  //}
  if (NULL == cli_hdl || NULL == p_loop) {
    LOG(L_ERROR) << "param is failed.";
    return NULL;
  }

  return (new CTcpAsyncClient(p_loop, cli_hdl));
}

bool CTcpAsyncClient::Connect(const CInetAddr *p_remote_addr,
                              bool             b_block,
                              bool             b_reuse,
                              uint32           n_timeout) {
  if (NULL == p_evt_loop_) {
    LOG(L_ERROR) << "event loop is NULL.";
    return false;
  }
  if (NULL == p_remote_addr || p_remote_addr->IsNull()) {
    LOG(L_ERROR) << "param is error.";
    return false;
  }

  c_evt_recv_.Stop();
  c_recv_data_.Clear();

  c_evt_send_.Stop();
  c_send_data_.Clear();

  SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (INVALID_SOCKET == s) {
    LOG(L_ERROR) << "socket open failed.";
    return false;
  }
  if (b_reuse) {
    int32 val = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(int32));
  }

  set_socket_nonblocking(s);
  int32 ret = connect(s,
                      (sockaddr*)p_remote_addr->GetAddr(),
                      (socklen_t)sizeof(sockaddr_in));
  if (0 == ret) {
    return Open(s, b_block);
  } else {
    if (XEAGAIN == error_no() || XEINPROGRESS == error_no()) {
      c_evt_send_.Init(p_evt_loop_, EvtConnect, this);
      ret = c_evt_send_.Start(s, EVT_WRITE, n_timeout);
      if (0 != ret) {
        LOG(L_ERROR) << "set connect event failed." << error_no();
        return false;
      }
      return true;
    }
  }

  closesocket(s);
  LOG(L_ERROR) << "connect return ev_write, but check failed";
  return false;
}

int32 CTcpAsyncClient::EvtConnect(SOCKET fd, short events, const void *p_usr_arg) {
  int32 n_ret = -1;
  if (p_usr_arg) {
    n_ret = ((CTcpAsyncClient*)p_usr_arg)->OnConnect(fd);
    if (n_ret < 0) {
      delete ((CTcpAsyncClient*)p_usr_arg);
    }
  }
  return n_ret;
}

int32 CTcpAsyncClient::OnConnect(SOCKET fd) {
  int32 n_ret = 0;
  int nError = 0;
  socklen_t nLen = sizeof(nError);
  getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&nError, &nLen);
  if (nError == 0) {
    Open(fd, false);
    LOG(L_INFO) << "connect return ev_write, check ok";

    if (cli_hdl_ptr_) {
      n_ret = cli_hdl_ptr_->HandleConnected(this);
    }
  } else {
    n_ret = -1;
    LOG(L_ERROR) << "connect return ev_write, but check failed";
  }

  if (n_ret < 0) {
    if (cli_hdl_ptr_) {
      cli_hdl_ptr_->HandleClose(this);
    }
  }
  return n_ret;
}

}  // namespace vzconn
