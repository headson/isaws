/************************************************************************/
/* Author      : SoberPeng 2017-06-17
/* Description :
/************************************************************************/
#include "cevtipcclient.h"

#include "stdafx.h"
#include "vzconn/base/pkghead.h"

CEvtIpcClient::CEvtIpcClient(const EVT_LOOP *p_loop,
                             CClientInterface *cli_hdl)
  : CEvtTcpClient(p_loop, cli_hdl) {
  LOG_INFO("%s[%d].0x%x.", __FUNCTION__, __LINE__, (uint32)this);
}

CEvtIpcClient::~CEvtIpcClient() {
  LOG_INFO("%s[%d].0x%x.", __FUNCTION__, __LINE__, (uint32)this);
}

CEvtIpcClient* CEvtIpcClient::Create(const EVT_LOOP *p_loop,
                                     CClientInterface *cli_hdl) {
  if (NULL == p_loop || p_loop->get_event() == NULL) {
    LOG(L_ERROR) << "param is failed.";
    return NULL;
  }
  if (NULL == cli_hdl) {
    LOG(L_ERROR) << "param is failed.";
    return NULL;
  }

  return (new CEvtIpcClient(p_loop, cli_hdl));
}

int32 CEvtIpcClient::Connect(const CInetAddr *p_remote_addr,
                             bool             b_block,
                             bool             b_reuse,
                             uint32           n_timeout/*=5000*/) {
  if (NULL == p_evt_loop_) {
    LOG(L_ERROR) << "event loop is NULL.";
    return -1;
  }
  if (NULL == p_remote_addr || p_remote_addr->IsNull()) {
    LOG(L_ERROR) << "param is error.";
    return -2;
  }

  SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (INVALID_SOCKET == s) {
    LOG(L_ERROR) << "socket open failed.";
    return -1;
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
    Open(s, b_block);
    return 0;
  } else {
    if (error_no() == XEINPROGRESS) {
#if 1
      fd_set fdw, fdr, efds;
      FD_ZERO(&fdw);
      FD_SET(s, &fdw);

      fdr = efds = fdw;
      struct timeval tv;
      tv.tv_sec  = n_timeout / 1000;
      tv.tv_usec = (n_timeout % 1000)*1000;
      ret = select(s + 1, &fdr, &fdw, &efds, &tv);
      if (ret > 0) {
#ifndef WIN32
        int nError = 0;
        socklen_t nLen = sizeof(nError);
        getsockopt(s, SOL_SOCKET, SO_ERROR, (char*)&nError, &nLen);
        if (nError == 0) {
          Open(s, b_block);
          return 0;
        }
#else  // WIN32
        if (FD_ISSET(s, &fdw)) {
          Open(s, b_block);
          return 0;
        }
#endif  // WIN32
      }
#else  // 异步链接服务器,暂时不成功,待研究
      c_evt_send_.Init(p_evt_loop_, EvtConnect, this);
      ret = c_evt_send_.Start(s, EVT_WRITE, n_timeout);
      if (0 != ret) {
        LOG(L_ERROR) << "set connect event failed." << error_no();
        return ret;
      }
      return 0;
#endif
    }
  }

  closesocket(s);
  LOG(L_ERROR) << "connect return ev_write, but check failed";
  return -1;
}
