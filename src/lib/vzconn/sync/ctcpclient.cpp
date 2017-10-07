/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "ctcpclient.h"
#include "vzconn/base/basedefines.h"

#include "vzbase/helper/stdafx.h"

namespace vzconn {

CTcpClient::CTcpClient(const EVT_LOOP *p_loop, CClientInterface *cli_hdl)
  : VSocket(cli_hdl)
  , p_evt_loop_(p_loop)
  , c_evt_recv_()
  , c_recv_data_()
  , c_evt_send_()
  , c_send_data_() {
  //LOG_INFO("%s[%d].0x%x.", __FUNCTION__, __LINE__, (uint32)this);
}

void CTcpClient::StopEvent() {
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

CTcpClient::~CTcpClient() {
  c_evt_recv_.Stop();
  c_evt_send_.Stop();

  Close();
  //LOG_INFO("%s[%d].0x%x.", __FUNCTION__, __LINE__, (uint32)this);
}

bool CTcpClient::Open(SOCKET s, bool is_block) {
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
  if (false == is_block) {
    set_socket_nonblocking(GetSocket());
  } else {
    set_socket_blocking(GetSocket());
  }

  int32 ret = 0;
  // 关联SOCKET的READ事件
  c_evt_recv_.Init(p_evt_loop_, EvtRecv, this);
  ret = c_evt_recv_.Start(GetSocket(), EVT_READ | EVT_PERSIST);
  if (0 != ret) {
    LOG(L_ERROR) << "set recv event failed." << error_no();
    return false;
  }

  // 关联SOCKET的SEND事件
  c_evt_send_.Init(p_evt_loop_, EvtSend, this);
  /*ret = c_evt_send_.Start(GetSocket(), EVT_WRITE);
  if (0 != ret) {
  LOG(L_ERROR) << "set send event failed." << error_no();
  return ret;
  }*/
  return true;
}

bool CTcpClient::Connect(const CInetAddr *remote_addr,
                         bool             is_block,
                         bool             is_reuse,
                         uint32           ms_timeout) {
  if (NULL == p_evt_loop_) {
    LOG(L_ERROR) << "event loop is NULL.";
    return false;
  }
  if (NULL == remote_addr || remote_addr->IsNull()) {
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
  if (is_reuse) {
    int32 val = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(int32));
  }

  set_socket_nonblocking(s);
  int32 ret = connect(s,
                      (sockaddr*)remote_addr->GetAddr(),
                      (socklen_t)sizeof(sockaddr_in));
  if (0 == ret) {
    return Open(s, is_block);
  } else {
    if (XEAGAIN == error_no() ||
        XEINPROGRESS == error_no()) {
#if 1
      fd_set fdw, fdr, efds;
      FD_ZERO(&fdw);
      FD_SET(s, &fdw);

      fdr = efds = fdw;
      struct timeval tv;
      tv.tv_sec  = ms_timeout / 1000;
      tv.tv_usec = (ms_timeout % 1000)*1000;
      ret = select(s + 1, &fdr, &fdw, &efds, &tv);
      if (ret > 0) {
#ifndef WIN32
        int nError = 0;
        socklen_t nLen = sizeof(nError);
        getsockopt(s, SOL_SOCKET, SO_ERROR, (char*)&nError, &nLen);
        if (nError == 0) {
          return Open(s, is_block);
        }
#else  // WIN32
        if (FD_ISSET(s, &fdw)) {
          return Open(s, is_block);
        }
#endif  // WIN32
      }
#else  // 异步链接服务器,暂时不成功,待研究
      c_evt_send_.Init(p_evt_loop_, EvtConnect, this);
      ret = c_evt_send_.Start(s, EVT_WRITE, ms_timeout);
      if (0 != ret) {
        LOG(L_ERROR) << "set connect event failed." << error_no();
        return false;
      }
      return true;
#endif
    }
  }

  closesocket(s);
  LOG(L_ERROR) << "connect return ev_write, but check failed";
  return false;
}

int32 CTcpClient::AsyncWrite(const void  *p_data,
                             uint32       n_data,
                             uint16       e_flag) {
  if (!isOpen() || !cli_hdl_ptr_) {
    LOG(L_ERROR) << "socket is not open or client handle is null.";
    return -1;
  }

  uint32_t n_head = cli_hdl_ptr_->NetHeadSize();
  if (c_send_data_.FreeSize() < (n_data+n_head)) {
    c_send_data_.Recycle();
    if (c_send_data_.FreeSize() < (n_data+n_head)) {
      c_send_data_.ReallocBuffer((n_data+n_head));
    }
  }
  if (c_send_data_.FreeSize() < (n_data+n_head)) {
    return false;
  }

  // 包头
  int32 n_head_size = cli_hdl_ptr_->NetHeadPacket(
                        c_send_data_.GetWritePtr(),
                        c_send_data_.FreeSize(),
                        n_data,
                        e_flag);
  c_send_data_.MoveWritePtr(n_head_size);
  c_send_data_.WriteBytes((uint8_t*)p_data, n_data);

  // 打开事件
  if (c_send_data_.UsedSize() > 0) {
    int32 n_ret = c_evt_send_.Start(GetSocket(), EVT_WRITE | EVT_PERSIST);
    if (n_ret == 0) {
      c_evt_send_.ActiceEvent();
    }
  }
  return (n_data+n_head);
}

int32 CTcpClient::AsyncWrite(struct iovec iov[],
                             uint32       n_iov,
                             uint16       e_flag) {
  if (!isOpen() || !cli_hdl_ptr_) {
    LOG(L_ERROR) << "socket is not open or client handle is null.";
    return -1;
  }

  uint32 n_data = 0;
  for (uint32 i = 0; i < n_iov; i++) {
    n_data += iov[i].iov_len;
  }
  uint32_t n_head = cli_hdl_ptr_->NetHeadSize();
  if (c_send_data_.FreeSize() < (n_data + n_head)) {
    c_send_data_.Recycle();
    if (c_send_data_.FreeSize() < (n_data + n_head)) {
      c_send_data_.ReallocBuffer((n_data + n_head));
    }
  }
  if (c_send_data_.FreeSize() < (n_data + n_head)) {
    return false;
  }

  // 包头
  int32 n_head_size = cli_hdl_ptr_->NetHeadPacket(
                        c_send_data_.GetWritePtr(),
                        c_send_data_.FreeSize(),
                        n_data,
                        e_flag);
  c_send_data_.MoveWritePtr(n_head_size);
  c_send_data_.WriteBytes(iov, n_iov);

  // 打开事件
  if (c_send_data_.UsedSize() > 0) {
    int32 n_ret = c_evt_send_.Start(GetSocket(), EVT_WRITE | EVT_PERSIST);
    if (n_ret == 0) {
      c_evt_send_.ActiceEvent();
    }
  }
  return (n_data + n_head);
}

int32 CTcpClient::SyncWrite(const void *p_data, uint32 n_data, uint16 e_flag) {
  if (!isOpen() || !cli_hdl_ptr_) {
    LOG(L_ERROR) << "socket is not open or client handle is null.";
    return -1;
  }

  uint32_t n_head = cli_hdl_ptr_->NetHeadSize();
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
  uint32_t n_head = cli_hdl_ptr_->NetHeadSize();
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

int32 CTcpClient::SendN(const uint8 *p_data, uint32 n_data) {
  uint32 npos = 0;
  int32 nsend = 0;
  do {
    nsend = VSocket::Send(p_data + npos, n_data - npos);
    if (nsend <= 0) {
      LOG(L_ERROR) << "socket send failed." << nsend;
      return nsend;
    }
    npos += nsend;
    // LOG_INFO("send message %d %d.\n", n_send, n_pos);
  } while (npos < n_data);
  return n_data;
}

int32 CTcpClient::EvtRecv(SOCKET      fd,
                          short       events,
                          const void *p_usr_arg) {
  int32 n_ret = -1;
  if (p_usr_arg) {
    n_ret = ((CTcpClient*)p_usr_arg)->OnRecv();
    if (n_ret < 0) {
      ((CTcpClient*)p_usr_arg)->StopEvent();
    }
  }
  return n_ret;
}

int32 CTcpClient::OnRecv() {
  int32_t n_recv = VSocket::Recv(c_recv_data_.GetWritePtr(),
                                 c_recv_data_.FreeSize());
  if (n_recv > 0) {
    c_recv_data_.MoveWritePtr(n_recv);
    //LOG(L_ERROR) << "recv length "<<n_recv;

    uint16 n_flag = 0;
    uint32_t n_pkg_size = 0;
    do {
      n_pkg_size = cli_hdl_ptr_->NetHeadParse(c_recv_data_.GetReadPtr(),
                                              c_recv_data_.UsedSize(),
                                              &n_flag);
      if (n_pkg_size > 0 && c_recv_data_.UsedSize() >= n_pkg_size) {
        cli_hdl_ptr_->HandleRecvPacket(
          this,
          c_recv_data_.GetReadPtr() + cli_hdl_ptr_->NetHeadSize(),
          n_pkg_size - cli_hdl_ptr_->NetHeadSize(),
          n_flag);
        c_recv_data_.MoveReadPtr(n_pkg_size);
        c_recv_data_.Recycle(); // 保证包头在buffer起始位置
      }
    } while (n_pkg_size > 0 && c_recv_data_.UsedSize() >= n_pkg_size);

    // 空间不足,开新空间
    if ((n_pkg_size - c_recv_data_.UsedSize()) > c_recv_data_.FreeSize()) {
      c_recv_data_.Recycle(); // 每次移动趋近于移动单位为0

      if ((n_pkg_size - c_recv_data_.UsedSize()) > c_recv_data_.FreeSize()) {
        c_recv_data_.ReallocBuffer((n_pkg_size - c_recv_data_.UsedSize()));
      }
    }
    if ((n_pkg_size - c_recv_data_.UsedSize()) > c_recv_data_.FreeSize()) {
      LOG(L_ERROR) << "the packet is large than " << SOCK_MAX_BUFFER_SIZE;
      return -1;
    }
  }

  if (n_recv < 0) {
    if (cli_hdl_ptr_) {
      cli_hdl_ptr_->HandleClose(this);
    }
  }
  return n_recv;
}

int32 CTcpClient::EvtSend(SOCKET      fd,
                          short       events,
                          const void *p_usr_arg) {
  int32 n_ret = -1;
  if (p_usr_arg) {
    n_ret = ((CTcpClient*)p_usr_arg)->OnSend();
    if (n_ret < 0) {
      ((CTcpClient*)p_usr_arg)->StopEvent();
    }
  }
  return n_ret;
}

int32 CTcpClient::OnSend() {
  //int32 need_send = (c_send_data_.UsedSize() > 1024)
  //                  ? 1024 : c_send_data_.UsedSize();
  int32 nsend = VSocket::Send(c_send_data_.GetReadPtr(),
                              c_send_data_.UsedSize());
  if (nsend > 0) {
    c_send_data_.MoveReadPtr(nsend);
    //Recycle();
  }

  int32 nret = 0;
  if (c_send_data_.UsedSize() <= 0) {
    c_send_data_.Recycle();             // 重置读写位置;移动为0
    c_evt_send_.Stop();

    if (cli_hdl_ptr_) {
      nret = cli_hdl_ptr_->HandleSendPacket(this); // 发送完成回调
    }
  }

  if (nret < 0) {
    if (cli_hdl_ptr_) {
      cli_hdl_ptr_->HandleClose(this);
      return nret;
    }
  }
  return 0;
}

int32 CTcpClient::EvtConnect(SOCKET       fd,
                             short        events,
                             const void  *p_usr_arg) {
  int32 n_ret = -1;
  if (p_usr_arg) {
    n_ret = ((CTcpClient*)p_usr_arg)->OnConnect(fd);
    if (n_ret < 0) {
      ((CTcpClient*)p_usr_arg)->StopEvent();
    }
  }
  return n_ret;
}

int32 CTcpClient::OnConnect(SOCKET fd) {
  int32 n_ret = 0;

  int nError = 0;
  socklen_t nLen = sizeof(nError);
  getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&nError, &nLen);
  if (nError == 0) {
    Open(fd, false);
    LOG(L_ERROR) << "connect return ev_write, check ok";
  } else {
    n_ret = -1;
    perror("getsockopt error.\n");
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
