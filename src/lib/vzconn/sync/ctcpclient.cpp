/************************************************************************/
/* Author      : SoberPeng 2017-06-17
/* Description :
/************************************************************************/
#include "ctcpclient.h"

#include "stdafx.h"
#include "vzconn/base/pkghead.h"

CTcpClient::CTcpClient()
  : VSocket()
  , c_sock_data_() {
  SetNetHeadParseCallback(vz_head_parse);
  SetNetHeadPacketCallback(vz_head_packet);
}

CTcpClient::~CTcpClient() {
  Close();
}

CTcpClient* CTcpClient::Create() {
  return (new CTcpClient());
}

int32 CTcpClient::Open(SOCKET s, bool b_block) {
  if (INVALID_SOCKET == s) {
    LOG(L_ERROR) << "param is error.";
    return -2;
  }

  SetSocket(s);

  //设置异步模式
  if (false == b_block) {
    set_socket_nonblocking(GetSocket());
  } else {
    set_socket_blocking(GetSocket());
  }
  return 0;
}

int32 CTcpClient::Connect(const CInetAddr *p_remote_addr,
                          bool             b_block,
                          bool             b_reuse,
                          uint32           n_timeout) {
  if (NULL == p_remote_addr || p_remote_addr->IsNull()) {
    LOG(L_ERROR) << "param is error.";
    return -1;
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
    }
  }

  closesocket(s);
  LOG(L_ERROR) << "connect return ev_write, but check failed";
  return -1;
}

int32 CTcpClient::SendReqWithResp(const void *p_req,
                                  uint32      n_req,
                                  const void *p_req_arg,
                                  void       *p_resp,
                                  uint32      n_resp) {
  int32 n_ret = 0;

  // 发送head
  if (net_head_packet_) {
    int32 n_head_size = 0;
    n_head_size = net_head_packet_(c_sock_data_.GetWritePtr(),
                                   c_sock_data_.FreeSize(),
                                   n_req, 
                                   p_req_arg);
    c_sock_data_.MoveWritePtr(n_head_size);
  }
  n_ret = Send(c_sock_data_.GetReadPtr(), c_sock_data_.UsedSize());
  if (n_ret < 0) {
    return n_ret;
  }
  c_sock_data_.MoveReadPtr(n_ret);
  c_sock_data_.Recycle();

  // 发送body
  n_ret = Send(p_req, n_req);
  if (n_ret < 0) {
    return n_ret;
  }

  // 接收数据

  return 0;
}
