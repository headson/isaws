/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "ctcpclient.h"
#include "vzconn/base/basedefines.h"

#include "vzbase/helper/stdafx.h"

namespace vzconn {

CTcpClient::CTcpClient(const EVT_LOOP *p_loop, CClientInterface *cli_hdl)
  : CEvtTcpClient(p_loop, cli_hdl) {
  //LOG_INFO("%s[%d].0x%x.", __FUNCTION__, __LINE__, (uint32)this);
}

CTcpClient::~CTcpClient() {
  c_evt_recv_.Stop();
  c_evt_send_.Stop();

  Close();
  //LOG_INFO("%s[%d].0x%x.", __FUNCTION__, __LINE__, (uint32)this);
}

bool CTcpClient::Open(SOCKET s, bool b_block) {
  if (NULL == p_evt_loop_) {
    LOG(L_ERROR) << "event loop is NULL.";
    return false;
  }
  if (INVALID_SOCKET == s) {
    LOG(L_ERROR) << "param is error.";
    return false;
  }

  SetSocket(s);

  const char chOpt = 1;
  SetOption(IPPROTO_TCP, TCP_NODELAY, (char*)&chOpt, sizeof(char));

  //设置异步模式
  if (false == b_block) {
    set_socket_nonblocking(GetSocket());
  } else {
    set_socket_blocking(GetSocket());
  }

  int32 ret = 0;
  // 关联SOCKET的READ事件
  c_evt_recv_.Init(p_evt_loop_, SubEvtRecv, this);
  ret = c_evt_recv_.Start(GetSocket(), EVT_READ | EVT_PERSIST);
  if (0 != ret) {
    LOG(L_ERROR) << "set recv event failed." << error_no();
    return false;
  }

  // 关联SOCKET的SEND事件
  c_evt_send_.Init(p_evt_loop_, SubEvtSend, this);
  /*ret = c_evt_send_.Start(GetSocket(), EVT_WRITE);
  if (0 != ret) {
  LOG(L_ERROR) << "set send event failed." << error_no();
  return ret;
  }*/
  return true;
}

void CTcpClient::Remove() {
  c_evt_recv_.Stop();
  c_evt_send_.Stop();
}

CTcpClient* CTcpClient::Create(const EVT_LOOP   *p_loop,
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

  return (new CTcpClient(p_loop, cli_hdl));
}

bool CTcpClient::Connect(const CInetAddr *p_remote_addr,
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
    if (XEAGAIN == error_no() ||
        XEINPROGRESS == error_no()) {
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
          return Open(s, b_block);
        }
#else  // WIN32
        if (FD_ISSET(s, &fdw)) {
          return Open(s, b_block);
        }
#endif  // WIN32
      }
    }
  }

  closesocket(s);
  LOG(L_ERROR) << "connect return ev_write, but check failed";
  return false;
}

int32 CTcpClient::SyncWrite(const void *p_data, uint32 n_data, uint16 e_flag) {
  if (!isOpen() || !cli_hdl_ptr_) {
    LOG(L_ERROR) << "socket is not open or client handle is null.";
    return -1;
  }

  uint32 n_head = cli_hdl_ptr_->NetHeadSize();
  if (c_send_data_.FreeSize() < (n_data + n_head)) {
    c_send_data_.Recycle();
    if (c_send_data_.FreeSize() < (n_data + n_head)) {
      c_send_data_.ReallocBuffer((n_data + n_head));
    }
  }
  if (c_send_data_.FreeSize() < (n_data + n_head)) {
    return 0;
  }

  set_socket_blocking(GetSocket());

  // 包头
  int32 n_head_size = cli_hdl_ptr_->NetHeadPacket(
                        c_send_data_.GetWritePtr(),
                        c_send_data_.FreeSize(),
                        n_data,
                        e_flag);
  c_send_data_.MoveWritePtr(n_head_size);
  c_send_data_.WriteBytes((uint8_t*)p_data, n_data);

  n_data = SendN(c_send_data_.GetReadPtr(),
                 c_send_data_.UsedSize());
  if (n_data > 0) {
    c_send_data_.MoveReadPtr(n_data);
    c_send_data_.Recycle();
  } else {
    LOG(L_ERROR) << "send n failed.";
  }

  set_socket_nonblocking(GetSocket());
  return n_data;
}

int32 CTcpClient::SyncWrite(struct iovec iov[], uint32 n_iov, uint16 e_flag) {
  if (!isOpen() || !cli_hdl_ptr_) {
    LOG(L_ERROR) << "socket is not open or client handle is null.";
    return -1;
  }

  uint32 n_data = 0;
  for (uint32 i = 0; i < n_iov; i++) {
    n_data += iov[i].iov_len;
  }
  uint32 n_head = cli_hdl_ptr_->NetHeadSize();
  if (c_send_data_.FreeSize() < (n_data + n_head)) {
    c_send_data_.Recycle();
    if (c_send_data_.FreeSize() < (n_data + n_head)) {
      c_send_data_.ReallocBuffer((n_data + n_head));
    }
  }
  if (c_send_data_.FreeSize() < (n_data + n_head)) {
    return false;
  }

  set_socket_blocking(GetSocket());

  // 包头
  int32 n_head_size = cli_hdl_ptr_->NetHeadPacket(
                        c_send_data_.GetWritePtr(),
                        c_send_data_.FreeSize(),
                        n_data,
                        e_flag);
  c_send_data_.MoveWritePtr(n_head_size);
  c_send_data_.WriteBytes(iov, n_iov);

  n_data = SendN(c_send_data_.GetReadPtr(),
                 c_send_data_.UsedSize());
  if (n_data > 0) {
    c_send_data_.MoveReadPtr(n_data);
    c_send_data_.Recycle();
  } else {
    LOG(L_ERROR) << "send n failed.";
  }
  LOG(L_INFO) << "send buffer length "<<n_data;
  set_socket_nonblocking(GetSocket());
  return n_data;
}

int32 CTcpClient::SubEvtRecv(SOCKET fd, short events, const void *usr_arg) {
  int32 n_ret = -1;
  if (usr_arg) {
    n_ret = ((CTcpClient*)usr_arg)->OnRecv();
    if (n_ret < 0) {
      ((CTcpClient*)usr_arg)->Remove();
    }
  }
  return n_ret;
}

int32 CTcpClient::SubEvtSend(SOCKET fd, short events, const void *p_usr_arg) {
  int32 n_ret = -1;
  if (p_usr_arg) {
    n_ret = ((CTcpClient*)p_usr_arg)->OnSend();
    if (n_ret < 0) {
      ((CTcpClient*)p_usr_arg)->Remove();
    }
  }
  return n_ret;
}

int32 CTcpClient::SendN(const uint8 *p_data, uint32 n_data) {
  uint32 n_pos = 0;
  int32 n_send = 0;
  do {
    n_send = VSocket::Send(p_data + n_pos, n_data - n_pos);
    if (n_send <= 0) {
      LOG(L_ERROR) << "socket send failed." << n_send;
      return n_send;
    }
    n_pos += n_send;
    // LOG_INFO("send message %d %d.\n", n_send, n_pos);
  } while (n_pos < n_data);
  return n_data;
}
}  // namespace vzconn
