/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "client_hs/client/ctcpasyncclient.h"

#include "vzbase/helper/stdafx.h"

#include "client_hs/base/basedefine.h"

namespace cli {

CTcpAsyncClient::CTcpAsyncClient(const vzconn::EVT_LOOP *ploop,
                                 vzconn::CClientInterface *cli_hdl)
  : CEvtTcpClient(ploop, cli_hdl) {
  LOG_INFO("%s[%d].0x%x.", __FUNCTION__, __LINE__, (uint32)this);
}

CTcpAsyncClient::~CTcpAsyncClient() {
  c_evt_recv_.Stop();
  c_evt_send_.Stop();

  Close();
  LOG_INFO("%s[%d].0x%x.", __FUNCTION__, __LINE__, (uint32)this);
}

int32 CTcpAsyncClient::AsyncWriteReq(int32 eCmd, int32 nMinor,
                                     const char* pData, uint32 nData,
                                     const char* pSrc, const char* pDst) {
  int32 pkg_size = nData + HEAD_LEN_REQ;
  if (pkg_size > c_send_data_.FreeSize()) {
    c_send_data_.ReallocBuffer(pkg_size);
  }
  if (pkg_size > c_send_data_.FreeSize()) {
    LOG(L_ERROR) << "not enough buffer to save send data.";
    return -1;
  }
  int32 res = PkgHeadReq((int8*)c_send_data_.GetWritePtr(),
                         eCmd, nMinor, pData, nData, pSrc, pDst);
  c_send_data_.MoveWritePtr(res);
  if (c_send_data_.UsedSize() > 0) {
    int32 nret = c_evt_send_.Start(GetSocket(), EVT_WRITE | EVT_PERSIST);
    if (nret == 0) {
      c_evt_send_.ActiceEvent();
    }
  }
  return res;
}

int32 CTcpAsyncClient::AsyncWriteRet(int32 eCmd,
                                     const char* pData, uint32 nData,
                                     int32 nRet, int32 eMinor) {
  int32 pkg_size = nData + HEAD_LEN_RET;
  if (pkg_size > c_send_data_.FreeSize()) {
    c_send_data_.ReallocBuffer(pkg_size);
  }
  if (pkg_size > c_send_data_.FreeSize()) {
    LOG(L_ERROR) << "not enough buffer to save send data.";
    return -1;
  }
  int32 res = PkgHeadRet((int8*)c_send_data_.GetWritePtr(),
                         eCmd, pData, nData, nRet, eMinor);
  c_send_data_.MoveWritePtr(res);
  if (c_send_data_.UsedSize() > 0) {
    int32 nret = c_evt_send_.Start(GetSocket(), EVT_WRITE | EVT_PERSIST);
    if (nret == 0) {
      c_evt_send_.ActiceEvent();
    }
  }
  return res;
}

CTcpAsyncClient* CTcpAsyncClient::Create(const vzconn::EVT_LOOP *ploop,
    vzconn::CClientInterface *cli_hdl) {
  //EVT_LOOP *p_evt_loop = const_cast<EVT_LOOP*>(p_loop);
  //if (NULL == p_evt_loop) {
  //  p_evt_loop = new EVT_LOOP();
  //  if (p_evt_loop) {
  //    p_evt_loop->Start();
  //  }
  //}
  if (NULL == ploop || NULL == cli_hdl) {
    LOG(L_ERROR) << "param is failed.";
    return NULL;
  }

  return (new CTcpAsyncClient(ploop, cli_hdl));
}

bool CTcpAsyncClient::Connect(const vzconn::CInetAddr *p_remote_addr,
                              bool bblock, bool breuse,
                              uint32 ms_timeout) {
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
  if (breuse) {
    int32 val = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(int32));
  }

  set_socket_nonblocking(s);
  int32 ret = connect(s,
                      (sockaddr*)p_remote_addr->GetAddr(),
                      (socklen_t)sizeof(sockaddr_in));
  if (0 == ret) {
    return Open(s, bblock);
  } else {
    if (XEAGAIN == error_no() || XEINPROGRESS == error_no()) {
      c_evt_send_.Init(p_evt_loop_, EvtConnect, this);
      ret = c_evt_send_.Start(s, EVT_WRITE, ms_timeout);
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

int32 CTcpAsyncClient::EvtConnect(SOCKET fd, short events, const void *usr_arg) {
  int32 nret = -1;
  if (usr_arg) {
    nret = ((CTcpAsyncClient*)usr_arg)->OnConnect(fd);
    if (nret < 0) {
      delete ((CTcpAsyncClient*)usr_arg);
    }
  }
  return nret;
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

int32 CTcpAsyncClient::OnRecv() {
  if (NULL == cli_hdl_ptr_) {
    LOG(L_ERROR) << "cli_hdl_ptr is null";
    return -1;
  }
  if (0 == n_wait_size_) {
    n_wait_size_ = cli_hdl_ptr_->NetHeadSize();
  }

  if (n_wait_size_ > c_recv_data_.FreeSize()) {
    c_recv_data_.ReallocBuffer(n_wait_size_);
    if (n_wait_size_ > c_recv_data_.FreeSize()) {
      LOG(L_ERROR) << "ReallocBuffer failed." << n_wait_size_;
      return -2;
    }
  }

  int32 nrecv = VSocket::Recv(c_recv_data_.GetWritePtr(),
                               n_wait_size_);
  // LOG(L_INFO) << "socket recv length " << nrecv;
  if (0 < nrecv) {
    c_recv_data_.MoveWritePtr(nrecv);
    n_wait_size_ -= nrecv;

    if (0 == n_wait_size_) {
      uint32 pkg_size = 0;
      uint16 head_size = 0;
      pkg_size = cli_hdl_ptr_->NetHeadParse(c_recv_data_.GetReadPtr(),
                                            c_recv_data_.UsedSize(),
                                            &head_size);
      if (pkg_size > 0) {
        if (pkg_size == c_recv_data_.UsedSize()) {
          // 接收到完整包
          cli_hdl_ptr_->HandleRecvPacket(this,
                                         c_recv_data_.GetReadPtr(), pkg_size,
                                         head_size);
          c_recv_data_.Clear();
        } else {
          n_wait_size_ = pkg_size - cli_hdl_ptr_->NetHeadSize();
        }
      } else {
        LOG(L_ERROR) << "parse packet is failed.";
        nrecv = -1;
      }
    }
  }

  if (nrecv < 0) {
    cli_hdl_ptr_->HandleClose(this);
  }
  return nrecv;
}

}  // namespace vzconn
