/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "ctcpclient.h"

#include "vzbase/helper/stdafx.h"
#include "vzconn/base/connhead.h"

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
  if (NULL == p_loop || p_loop->get_event() == NULL) {
    LOG(L_ERROR) << "param is failed.";
    return NULL;
  }
  if (NULL == cli_hdl) {
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
          return Open(s, b_block);
        }
#else  // WIN32
        if (FD_ISSET(s, &fdw)) {
          return Open(s, b_block);
        }
#endif  // WIN32
      }
#else  // 异步链接服务器,暂时不成功,待研究
      c_evt_send_.Init(p_evt_loop_, EvtConnect, this);
      ret = c_evt_send_.Start(s, EVT_WRITE, n_timeout);
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
  if (isOpen()) {
    int32_t n_pkg = c_send_data_.DataCacheToSendBuffer(this,
                    p_data,
                    n_data,
                    e_flag);
    if (n_pkg < 0) {
      LOG(L_ERROR) << "not enough buffer to packet the data";
      return n_pkg;
    }
    // 打开事件
    if (c_send_data_.UsedSize() > 0) {
      int32 n_ret = c_evt_send_.Start(GetSocket(), EVT_WRITE | EVT_PERSIST);
      if (n_ret == 0) {
        c_evt_send_.ActiceEvent();
      }
    }
    return n_pkg;
  }
  return -1;
}

int32 CTcpClient::AsyncWrite(struct iovec iov[],
                             uint32       n_iov,
                             uint16       e_flag) {
  if (isOpen()) {
    int32_t n_pkg = c_send_data_.DataCacheToSendBuffer(this,
                    iov,
                    n_iov,
                    e_flag);
    if (n_pkg < 0) {
      LOG(L_ERROR) << "not enough buffer to packet the data";
      return n_pkg;
    }
    // 打开事件
    if (c_send_data_.UsedSize() > 0) {
      int32 n_ret = c_evt_send_.Start(GetSocket(), EVT_WRITE | EVT_PERSIST);
      if (n_ret == 0) {
        c_evt_send_.ActiceEvent();
      }
    }
    return n_pkg;
  }
  return -1;
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
  int32 n_ret = c_recv_data_.RecvData(this);
  if (n_ret > 0) {
    n_ret = c_recv_data_.ParseSplitData(this); // 通过回调反馈给用户层
  }

  if (n_ret < 0) {
    if (cli_hdl_ptr_) {
      cli_hdl_ptr_->HandleClose(this);
    }
  }
  return n_ret;
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
  int32 n_ret = 0;

  // 发送完成回调
  if (cli_hdl_ptr_) {
    n_ret = cli_hdl_ptr_->HandleSendPacket(this);
  }

  if (n_ret >= 0) {
    // 发送数据
    n_ret = c_send_data_.SendData(this);
    if (c_send_data_.UsedSize() <= 0) {
      c_evt_send_.Stop();
    }
  }

  if (n_ret < 0) {
    if (cli_hdl_ptr_) {
      cli_hdl_ptr_->HandleClose(this);
    }
  }
  return n_ret;
}

int32 CTcpClient::EvtConnect(SOCKET       fd,
                             short        events,
                             const void  *p_usr_arg) {
  int32 n_ret = -1;
  if (p_usr_arg) {
    n_ret = ((CTcpClient*)p_usr_arg)->OnConnect(fd);
    if (n_ret < 0) {
      delete ((CTcpClient*)p_usr_arg);
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
