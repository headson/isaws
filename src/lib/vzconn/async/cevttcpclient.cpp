/************************************************************************/
/* Author      : Sober.Peng 17-06-15
/* Description :
/************************************************************************/
#include "cevttcpclient.h"

#include "stdafx.h"
#include "vzconn/base/pkghead.h"

CEvtTcpClient::CEvtTcpClient(const EVT_LOOP *p_loop, CClientInterface *cli_hdl)
  : VSocket()
  , p_evt_loop_(p_loop)
  , c_evt_recv_()
  , c_recv_data_()
  , c_evt_send_()
  , c_send_data_()
  , cli_handle_ptr_(cli_hdl) {
  SetNetHeadParseCallback(vz_head_parse);
  SetNetHeadPacketCallback(vz_head_packet);
  LOG_INFO("%s[%d].0x%x.", __FUNCTION__, __LINE__, (uint32)this);
}

CEvtTcpClient* CEvtTcpClient::Create(const EVT_LOOP   *p_loop,
                                     CClientInterface *cli_hdl) {
  if (NULL == p_loop || p_loop->get_event() == NULL) {
    LOG(L_ERROR) << "param is failed.";
    return NULL;
  }
  if (NULL == cli_hdl) {
    LOG(L_ERROR) << "param is failed.";
    return NULL;
  }

  return (new CEvtTcpClient(p_loop, cli_hdl));
}

CEvtTcpClient::~CEvtTcpClient() {
  c_evt_recv_.Stop();
  c_evt_send_.Stop();

  Close();

  LOG_INFO("%s[%d].0x%x.", __FUNCTION__, __LINE__, (uint32)this);
}

void CEvtTcpClient::Remove() {
  /*c_evt_recv_.Stop();
  c_evt_send_.Stop();*/

  Close();

  c_evt_recv_.ActiceEvent();  // 主动唤醒读事件;链接已关闭,读错误,销毁此链接
}

int32 CEvtTcpClient::Open(SOCKET s, bool b_block) {
  if (NULL == p_evt_loop_) {
    LOG(L_ERROR) << "event loop is NULL.";
    return -1;
  }
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

  int32 ret = 0;
  // 关联SOCKET的READ事件
  c_evt_recv_.Init(p_evt_loop_, EvtRecv, this);
  ret = c_evt_recv_.Start(GetSocket(), EVT_READ | EVT_PERSIST);
  if (0 != ret) {
    LOG(L_ERROR) << "set recv event failed." << error_no();
    return ret;
  }

  // 关联SOCKET的SEND事件
  c_evt_send_.Init(p_evt_loop_, EvtSend, this);
  /*ret = c_evt_send_.Start(GetSocket(), EVT_WRITE);
  if (0 != ret) {
  LOG(L_ERROR) << "set send event failed." << error_no();
  return ret;
  }*/
  return ret;
}

int32 CEvtTcpClient::Connect(const CInetAddr *p_remote_addr,
                             bool             b_block,
                             bool             b_reuse,
                             uint32           n_timeout) {
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

int32 CEvtTcpClient::SendPacket(const void  *p_data,
                                uint32       n_data,
                                const void  *p_param) {
  if ((n_data + 8) > c_send_data_.FreeSize()) {
    return -1;
  }

  if (IsOpen()) {
    // 包头
    int32 n_head_size = 0;
    if (NULL != net_head_parse_) {
      n_head_size = net_head_packet_(c_send_data_.GetWritePtr(),
                                     c_send_data_.FreeSize(),
                                     n_data, p_param);
    }
    c_send_data_.MoveWritePtr(n_head_size);

    // body
    memcpy(c_send_data_.GetWritePtr(), p_data, n_data);
    c_send_data_.MoveWritePtr(n_data);

    // 打开事件
    if (c_send_data_.UsedSize() > 0) {
      c_evt_send_.Start(GetSocket(), EVT_WRITE | EVT_PERSIST);

      c_evt_send_.ActiceEvent();
    }
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
  int32 n_ret = -1;

  int32 n_data = Recv(c_recv_data_.GetWritePtr(), c_recv_data_.FreeSize());
  if (n_data > 0) {
    c_recv_data_.MoveWritePtr(n_data);
    n_ret = n_data;

    // 解析包头,获取整包数据长度
    uint32 n_offset   = 0;  // 解析时,发现起始数据无包头,矫正包头的偏移
    int32  n_Pkg_size = 0;  // 一整包数据长度;head+body
    if (NULL != net_head_parse_) {
      n_Pkg_size = net_head_parse_(c_recv_data_.GetReadPtr(),
                                   c_recv_data_.UsedSize(),
                                   &n_offset);
    }
    if (n_offset > 0) {
      c_recv_data_.MoveReadPtr(n_offset);
    }

    // 回调
    if (n_Pkg_size > 0 && cli_handle_ptr_) {
      n_ret = cli_handle_ptr_->HandleRecvPacket(this,
              c_recv_data_.GetReadPtr(),
              n_Pkg_size);
      c_recv_data_.MoveReadPtr(n_Pkg_size);
      c_recv_data_.Recycle();
    }

  } else {
    n_ret = -1;
  }

  if (n_ret < 0) {
    if (cli_handle_ptr_) {
      cli_handle_ptr_->HandleClose(this);
    }
  }
  return n_ret;
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
  int32 n_ret = 0;

  // 发送完成回调
  if (cli_handle_ptr_) {
    n_ret = cli_handle_ptr_->HandleSendPacket(this);
  }

  // 发送数据
  if (n_ret >= 0) {
    if (c_send_data_.UsedSize() > 0) {
      int32 n_send = Send(c_send_data_.GetReadPtr(), c_send_data_.UsedSize());
      if (n_send > 0) {
        c_send_data_.MoveReadPtr(n_send);
        c_send_data_.Recycle();
      } else {
        n_ret = n_send;
      }
    } else {
      c_evt_send_.Stop();
    }
  }

  if (n_ret < 0) {
    if (cli_handle_ptr_) {
      cli_handle_ptr_->HandleClose(this);
    }
  }
  return n_ret;
}

int32 CEvtTcpClient::EvtConnect(SOCKET       fd,
                                short        events,
                                const void  *p_usr_arg) {
  int32 n_ret = -1;
  if (p_usr_arg) {
    n_ret = ((CEvtTcpClient*)p_usr_arg)->OnConnect(fd);
    if (n_ret < 0) {
      delete ((CEvtTcpClient*)p_usr_arg);
    }
  }
  return n_ret;
}

int32 CEvtTcpClient::OnConnect(SOCKET fd) {
  int32 n_ret = 0;

  int nError = 0;
  socklen_t nLen = sizeof(nError);
  getsockopt(fd, SOL_SOCKET, SO_ERROR, (char*)&nError, &nLen);
  if (nError == 0) {
    Open(fd, false);
    LOG(L_ERROR) << "connect return ev_write, check ok";
  } else {
    n_ret = -1;
    LOG(L_ERROR) << "connect return ev_write, but check failed";
  }

  if (n_ret < 0) {
    if (cli_handle_ptr_) {
      cli_handle_ptr_->HandleClose(this);
    }
  }
  return n_ret;
}
