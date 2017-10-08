/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "cevttcpclient.h"
#include "vzconn/base/basedefines.h"

#include "vzbase/helper/stdafx.h"

namespace vzconn {

CEvtTcpClient::CEvtTcpClient(const EVT_LOOP *evt_loop,
                             CClientInterface *cli_hdl)
  : VSocket(cli_hdl)
  , evt_loop_(evt_loop)
  , evt_recv_()
  , evt_send_() {
  recv_data_.reset();
  head_size_ = 0;
  head_data_ = NULL;
  if (cli_hdl_ptr_) {
    wait_recv_size_ = cli_hdl_ptr_->NetHeadSize();
  }

  send_list_.clear();
  send_data_.reset();
  //LOG_INFO("%s[%d].0x%x.", __FUNCTION__, __LINE__, (uint32)this);
}

CEvtTcpClient* CEvtTcpClient::Create(const EVT_LOOP   *evt_loop,
                                     CClientInterface *cli_hdl) {
  if (NULL == evt_loop || evt_loop->get_event() == NULL) {
    LOG(L_ERROR) << "param is failed.";
    return NULL;
  }
  //if (NULL == cli_hdl) {
  //  LOG(L_ERROR) << "param is failed.";
  //  return NULL;
  //}

  return (new CEvtTcpClient(evt_loop, cli_hdl));
}

CEvtTcpClient::~CEvtTcpClient() {
  evt_recv_.Stop();
  evt_send_.Stop();

  Close();

  //LOG_INFO("%s[%d].0x%x.", __FUNCTION__, __LINE__, (uint32)this);
}

void CEvtTcpClient::Remove() {
  /*c_evt_recv_.Stop();
  c_evt_send_.Stop();*/

  Close();

  evt_recv_.ActiceEvent();  // 主动唤醒读事件;链接已关闭,读错误,销毁此链接
}

bool CEvtTcpClient::Open(SOCKET s, bool is_block) {
  if (NULL == evt_loop_) {
    LOG(L_ERROR) << "event loop is NULL.";
    return false;
  }
  if (INVALID_SOCKET == s) {
    LOG(L_ERROR) << "param is error.";
    return false;
  }

  SetSocket(s);

  const char chOpt = 0;
  SetOption(IPPROTO_TCP, TCP_NODELAY, (char*)&chOpt, sizeof(char));

  //设置异步模式
  if (false == is_block) {
    set_socket_nonblocking(GetSocket());
  } else {
    set_socket_blocking(GetSocket());
  }

  int32 ret = 0;
  // 关联SOCKET的READ事件
  evt_recv_.Init(evt_loop_, EvtRecv, this);
  ret = evt_recv_.Start(GetSocket(), EVT_READ | EVT_PERSIST);
  if (0 != ret) {
    LOG(L_ERROR) << "set recv event failed." << error_no();
    return false;
  }

  // 关联SOCKET的SEND事件
  evt_send_.Init(evt_loop_, EvtSend, this);
  /*ret = evt_send_.Start(GetSocket(), EVT_WRITE);
  if (0 != ret) {
    LOG(L_ERROR) << "set send event failed." << error_no();
    return ret;
  }*/
  return true;
}

int32 CEvtTcpClient::AsyncWrite(const void  *pdata, uint32 ndata, uint16 eflag) {
  if (isOpen() && cli_hdl_ptr_) {
    uint32 pkg_size = ndata + cli_hdl_ptr_->NetHeadSize();
    ByteBuffer::Ptr sptr = VzConnBufferPool()->TakeBuffer(pkg_size);

    // body
    sptr->WriteBytes((char*)pdata, ndata);
    // insert send list
    return AsyncWrite(sptr, eflag);
  }
  return -1;
}

int32 CEvtTcpClient::AsyncWrite(struct iovec iov[], uint32 iovs, uint16 eflag) {
  size_t iov_size = 0;
  for (size_t i = 0; i < iovs; i++) {
    iov_size += iov[i].iov_len;
  }

  uint32 pkg_size = iov_size + cli_hdl_ptr_->NetHeadSize();
  ByteBuffer::Ptr sptr = VzConnBufferPool()->TakeBuffer(pkg_size);

  // body
  for (size_t i = 0; i < iovs; i++) {
    sptr->WriteBytes((char*)iov[i].iov_base, iov[i].iov_len);
  }
  // insert send list
  return AsyncWrite(sptr, eflag);
}

int32 CEvtTcpClient::AsyncWrite(ByteBuffer::Ptr sptr, uint16 eflag) {
  if (!isOpen() || !cli_hdl_ptr_) {
    return -1;
  }

  // 包头
  sptr->HeadInit(cli_hdl_ptr_->NetHeadSize());
  cli_hdl_ptr_->NetHeadPacket((uint8*)sptr->HeadData(), sptr->HeadSize(),
                              sptr->Length(), eflag);
  sptr->send_size_ = sptr->HeadSize() + sptr->Length();

  // insert send list
  send_list_.push_back(sptr);

  // 打开事件
  if (send_list_.size() > 0) {
    int res = evt_send_.Start(GetSocket(), EVT_WRITE | EVT_PERSIST);
    if (res == 0) {
      evt_send_.ActiceEvent();
    }
  }
  return sptr->size();
}

int CEvtTcpClient::EvtRecv(SOCKET fd, short events, const void *p_usr_arg) {
  int32 n_ret = -1;
  if (p_usr_arg) {
    n_ret = ((CEvtTcpClient*)p_usr_arg)->OnRecv();
    if (n_ret < 0) {
      delete ((CEvtTcpClient*)p_usr_arg);
    }
  }
  return n_ret;
}

int CEvtTcpClient::OnRecv() {
  if (NULL == cli_hdl_ptr_) {
    LOG(L_ERROR) << "the cli hdl is null.";
    return -1;
  }

  head_size_ = cli_hdl_ptr_->NetHeadSize();
  if (NULL == head_data_) {
    wait_recv_size_ = head_size_;
    head_data_ = new char(head_size_);
  }
  if (NULL == head_data_) {
    LOG(L_ERROR) << "can't new head_data." << head_size_;
    return -2;
  }

  int res = 0;
  unsigned short eflag = 0;
  if (NULL == recv_data_.get()) {
    res = VSocket::Recv(head_data_ + (head_size_ - wait_recv_size_), wait_recv_size_);
    if (res > 0) {
      wait_recv_size_ -= res;
      if (wait_recv_size_ == 0) {
        unsigned int pkg_size = cli_hdl_ptr_->NetHeadParse((uint8*)head_data_,
                                head_size_,
                                &eflag);
        if (0 == pkg_size) {
          LOG(L_ERROR) << "packet size is zero.";
          return -4;
        }
        recv_data_ = VzConnBufferPool()->TakeBuffer(pkg_size);
        if (NULL == recv_data_.get()) {
          LOG(L_ERROR) << "can't get buffer." << pkg_size;
          return -3;
        }
        recv_data_->Head(head_data_, head_size_);
        wait_recv_size_ = pkg_size - head_size_;
      }
    }
  } else {
    res = VSocket::Recv(recv_data_->DataWrite(), wait_recv_size_);
    if (res > 0) {
      recv_data_->MoveDataWrite(res);

      wait_recv_size_ -= res;
      if (0 == wait_recv_size_) {
        cli_hdl_ptr_->HandleRecvPacket(this,
                                       (uint8*)recv_data_->DataRead(),
                                       recv_data_->Length(),
                                       eflag);
        wait_recv_size_ = head_size_;
        recv_data_.reset();
      }
    }
  }
  if (res < 0) {
    cli_hdl_ptr_->HandleClose(this);
  }
  return res;
}

int CEvtTcpClient::EvtSend(SOCKET fd, short events, const void *usr_arg) {
  int res = -1;
  if (usr_arg) {
    res = ((CEvtTcpClient*)usr_arg)->OnSend();
    if (res < 0) {
      delete ((CEvtTcpClient*)usr_arg);
    }
  }
  return res;
}

int CEvtTcpClient::OnSend() {
  if (NULL == send_data_.get()) {
    send_data_ = send_list_.front();
    send_list_.pop_front();
  }

  int nhdl = 0;
  int nsend = 0;
  nsend = VSocket::Send(send_data_->SendData(), send_data_->SendSize());
  if (nsend > 0) {
    send_data_->MoveDataSend(nsend);
  }

  /* is send all */
  if (send_data_->SendSize() == 0) {
    send_data_.reset();
    if (cli_hdl_ptr_) {
      nhdl = cli_hdl_ptr_->HandleSendPacket(this); // 发送完成回调
    }
    if (send_list_.size() <= 0) {
      evt_send_.Stop();
    }
  }
  if (nsend < 0 || nhdl < 0) {
    cli_hdl_ptr_->HandleClose(this);
    return -1;
  }
  return 0;
}

}  // namespace vzconn
