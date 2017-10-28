/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "cevttcpclient.h"
#include "vzconn/base/basedefines.h"

#include "vzbase/helper/stdafx.h"

namespace vzconn {

CEvtTcpClient::CEvtTcpClient(const EVT_LOOP *p_loop, CClientInterface *cli_hdl)
  : VSocket(cli_hdl)
  , p_evt_loop_(p_loop)
  , c_evt_recv_()
  , c_recv_data_()
  , n_wait_size_(0)
  , c_evt_send_()
  , c_send_data_() {
  //LOG_INFO("%s[%d].0x%x.", __FUNCTION__, __LINE__, (uint32)this);
}

CEvtTcpClient* CEvtTcpClient::Create(const EVT_LOOP   *p_loop,
                                     CClientInterface *cli_hdl) {
  if (NULL == p_loop || p_loop->get_event() == NULL) {
    LOG(L_ERROR) << "param is failed.";
    return NULL;
  }
  //if (NULL == cli_hdl) {
  //  LOG(L_ERROR) << "param is failed.";
  //  return NULL;
  //}

  return (new CEvtTcpClient(p_loop, cli_hdl));
}

CEvtTcpClient::~CEvtTcpClient() {
  c_evt_recv_.Stop();
  c_evt_send_.Stop();

  Close();

  //LOG_INFO("%s[%d].0x%x.", __FUNCTION__, __LINE__, (uint32)this);
}

void CEvtTcpClient::Remove() {
  /*c_evt_recv_.Stop();
  c_evt_send_.Stop();*/

  Close();

  c_evt_recv_.ActiceEvent();  // �������Ѷ��¼�;�����ѹر�,������,���ٴ�����
}

bool CEvtTcpClient::Open(SOCKET s, bool b_block) {
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

  //�����첽ģʽ
  if (false == b_block) {
    set_socket_nonblocking(GetSocket());
  } else {
    set_socket_blocking(GetSocket());
  }

  int32 ret = 0;
  // ����SOCKET��READ�¼�
  c_evt_recv_.Init(p_evt_loop_, EvtRecv, this);
  ret = c_evt_recv_.Start(GetSocket(), EVT_READ | EVT_PERSIST);
  if (0 != ret) {
    LOG(L_ERROR) << "set recv event failed." << error_no();
    return false;
  }

  // ����SOCKET��SEND�¼�
  c_evt_send_.Init(p_evt_loop_, EvtSend, this);
  /*ret = c_evt_send_.Start(GetSocket(), EVT_WRITE);
  if (0 != ret) {
  LOG(L_ERROR) << "set send event failed." << error_no();
  return ret;
  }*/
  return true;
}

bool CEvtTcpClient::Connect(const CInetAddr *p_remote_addr,
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
  SetRemoteAddr(*p_remote_addr);
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
    LOG(L_INFO) << "Open the socket " << p_remote_addr->ToString();
    return Open(s, b_block);
  } else {
    if (XEAGAIN == error_no() || XEINPROGRESS == error_no()) {
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

int32 CEvtTcpClient::AsyncWrite(const void  *p_data,
                                uint32       n_data,
                                uint16       e_flag) {
  if (isOpen() && cli_hdl_ptr_) {
    uint32 n_head = cli_hdl_ptr_->NetHeadSize();
    if (c_send_data_.FreeSize() < (n_data+n_head)) {
      c_send_data_.Recycle();
      if (c_send_data_.FreeSize() < (n_data+n_head)) {
        c_send_data_.ReallocBuffer((n_data+n_head));
      }
    }
    if (c_send_data_.FreeSize() < (n_data+n_head)) {
      return 0;
    }

    // ��ͷ
    int32 n_head_size = cli_hdl_ptr_->NetHeadPacket(
                          c_send_data_.GetWritePtr(),
                          c_send_data_.FreeSize(),
                          n_data,
                          e_flag);
    c_send_data_.MoveWritePtr(n_head_size);
    c_send_data_.WriteBytes((uint8*)p_data, n_data);

    // ���¼�
    if (c_send_data_.UsedSize() > 0) {
      int32 n_ret = c_evt_send_.Start(GetSocket(), EVT_WRITE | EVT_PERSIST);
      if (n_ret == 0) {
        c_evt_send_.ActiceEvent();
      }
    }
    return (n_data+n_head);
  }
  return -1;
}

int32 CEvtTcpClient::AsyncWrite(struct iovec iov[],
                                uint32       n_iov,
                                uint16       e_flag) {
  if (isOpen() && cli_hdl_ptr_) {
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
      return 0;
    }

    // ��ͷ
    int32 n_head_size = cli_hdl_ptr_->NetHeadPacket(
                          c_send_data_.GetWritePtr(),
                          c_send_data_.FreeSize(),
                          n_data,
                          e_flag);
    c_send_data_.MoveWritePtr(n_head_size);
    c_send_data_.WriteBytes(iov, n_iov);

    // ���¼�
    if (c_send_data_.UsedSize() > 0) {
      int32 n_ret = c_evt_send_.Start(GetSocket(), EVT_WRITE | EVT_PERSIST);
      if (n_ret == 0) {
        c_evt_send_.ActiceEvent();
      }
    }
    return (n_data + n_head);
  }
  return -1;
}

int32 CEvtTcpClient::EvtRecv(SOCKET      fd,
                             short       events,
                             const void *p_usr_arg) {
  int32 n_ret = -1;
  if (p_usr_arg) {
    n_ret = ((CEvtTcpClient*)p_usr_arg)->OnRecv();
    if (n_ret < 0) {
      delete ((CEvtTcpClient*)p_usr_arg);
    }
  }
  return n_ret;
}

int32 CEvtTcpClient::OnRecv() {
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

  int32 n_recv = VSocket::Recv(c_recv_data_.GetWritePtr(),
                               n_wait_size_);
  if (0 < n_recv) {
    c_recv_data_.MoveWritePtr(n_recv);
    n_wait_size_ -= n_recv;

    if (0 == n_wait_size_) {
      uint16 eflag = 0;
      uint32 pkg_size = 0;
      pkg_size = cli_hdl_ptr_->NetHeadParse(c_recv_data_.GetReadPtr(),
                                            c_recv_data_.UsedSize(),
                                            &eflag);
      if (pkg_size > 0) {
        if (pkg_size == c_recv_data_.UsedSize()) {
          // ���յ�������
          cli_hdl_ptr_->HandleRecvPacket(this,
                                         c_recv_data_.GetReadPtr() + cli_hdl_ptr_->NetHeadSize(),
                                         pkg_size - cli_hdl_ptr_->NetHeadSize(),
                                         eflag);
          c_recv_data_.Clear();
        } else {
          n_wait_size_ = pkg_size - cli_hdl_ptr_->NetHeadSize();
        }
      } else {
        LOG(L_ERROR) << "parse packet is failed.";
        n_recv = -1;
      }
    }
  }

  if (n_recv < 0) {
    cli_hdl_ptr_->HandleClose(this);
  }
  return n_recv;
}

int32 CEvtTcpClient::EvtSend(SOCKET      fd,
                             short       events,
                             const void *p_usr_arg) {
  int32 n_ret = -1;
  if (p_usr_arg) {
    n_ret = ((CEvtTcpClient*)p_usr_arg)->OnSend();
    if (n_ret < 0) {
      delete ((CEvtTcpClient*)p_usr_arg);
    }
  }
  return n_ret;
}

int32 CEvtTcpClient::OnSend() {
  int32 nsend = VSocket::Send(c_send_data_.GetReadPtr(),
                              c_send_data_.UsedSize());
  if (nsend > 0) {
    c_send_data_.MoveReadPtr(nsend);
    //Recycle();
  }

  int32 nret = 0;
  if (c_send_data_.UsedSize() <= 0) {
    c_send_data_.Recycle();             // ���ö�дλ��;�ƶ�Ϊ0
    c_evt_send_.Stop();

    if (cli_hdl_ptr_) {
      nret = cli_hdl_ptr_->HandleSendPacket(this); // ������ɻص�
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
}  // namespace vzconn
