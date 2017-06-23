/************************************************************************/
/* Author      : Sober.Peng 17-06-19
/* Description :
/************************************************************************/
#include "csocketbuffer.h"

#include "stdafx.h"
#include "connhead.h"

namespace vzconn {

CSockRecvData::CSockRecvData()
  : CBlockBuffer() {
  n_wait_len_ = 0;
}

CSockRecvData::CSockRecvData(uint8 *p_data, uint32 n_data)
  : CBlockBuffer(p_data, n_data) {
}

CSockRecvData::~CSockRecvData() {
}

int32 CSockRecvData::RecvData(VSocket* p_sock) {
  int32 n_ret = -1;
  if (!p_sock && !p_sock->cli_hdl_ptr_) {
    return -1;
  }
  if (n_wait_len_ <= 0) {
    n_wait_len_ = p_sock->cli_hdl_ptr_->NetHeadSize();
  }

  // 数据不够,分配大一点的空间
  if (n_wait_len_ > Length()) {
    ReallocBuffer(n_wait_len_);
  }

  // 接收数据
  n_ret = p_sock->Recv(GetWritePtr(), n_wait_len_ - UsedSize());
  if (n_ret > 0) {
    MoveWritePtr(n_ret);
    LOG(L_INFO) << "recv buffer size "<<n_ret;
  } else {
    n_ret = -1;
  }

  return n_ret;
}

int32 CSockRecvData::ParseSplitData(VSocket* p_sock) {
  int32 n_ret = 0;
  if (!p_sock && !p_sock->cli_hdl_ptr_) {
    return -1;
  }

  if (n_wait_len_ <= UsedSize()) {
    // 解析包头,获取整包数据长度
    uint16 n_flag = 0;
    uint32 n_offset = 0;  // 解析时,发现起始数据无包头,矫正包头的偏移
    int32 n_pkg_size = 0;  // 一整包数据长度;head+body
    n_pkg_size = p_sock->cli_hdl_ptr_->NetHeadParse(GetReadPtr(),
                 UsedSize(),
                 &n_flag);
    if (n_offset > 0) {
      MoveReadPtr(n_offset);
    }
    // 剩余需要读的数据
    if (n_pkg_size > 0) {
      n_wait_len_ = n_pkg_size;

      // 回调
      if ((n_pkg_size <= UsedSize()) && p_sock->cli_hdl_ptr_) {
        n_ret = p_sock->cli_hdl_ptr_->HandleRecvPacket(p_sock,
                GetReadPtr() + p_sock->cli_hdl_ptr_->NetHeadSize(),
                n_pkg_size - p_sock->cli_hdl_ptr_->NetHeadSize(),
                n_flag);
        if (n_ret >= 0) {
          n_ret = n_pkg_size;
        }
        MoveReadPtr(n_pkg_size);
        Recycle(); // 每次移动趋近于移动单位为0

        n_wait_len_ = p_sock->cli_hdl_ptr_->NetHeadSize();
      }
    } else if (n_pkg_size < 0) {
      LOG(L_ERROR) << "error packet." << (uint32)buffer_;
      n_ret = -1;
    }
  }
  return n_ret;
}

//////////////////////////////////////////////////////////////////////////
CSockSendData::CSockSendData()
  : CBlockBuffer() {
}

CSockSendData::~CSockSendData() {

}

int32 CSockSendData::SendData(VSocket* p_sock) {
  int32 n_ret = 0;
  if (UsedSize() > 0) {
    int32 n_send = p_sock->Send(GetReadPtr(), UsedSize());
    if (n_send > 0) {
      MoveReadPtr(n_send);
      //Recycle();
    }
    n_ret = n_send;
  }
  return n_ret;
}

int32 CSockSendData::DataCacheToSendBuffer(VSocket     *p_sock,
    const void  *p_data,
    uint32       n_data,
    uint16       e_flag) {
  if (!p_sock && !p_sock->cli_hdl_ptr_) {
    return -1;
  }

  int32 n_head = p_sock->cli_hdl_ptr_->NetHeadSize();
  // 数据不够,分配大一点的空间
  if ((n_head + n_data) > FreeSize()) {
    Recycle();
    if ((n_head + n_data) > FreeSize()) {
      ReallocBuffer((n_head + n_data));
    }
  }
  if ((n_head + n_data) > FreeSize()) {
    return -1;
  }

  // 包头
  int32 n_head_size = p_sock->cli_hdl_ptr_->NetHeadPacket(GetWritePtr(),
                      FreeSize(),
                      n_data,
                      e_flag);
  MoveWritePtr(n_head_size);

  // body
  memcpy(GetWritePtr(), p_data, n_data);
  MoveWritePtr(n_data);
  return n_head_size + n_data;
}

int32 CSockSendData::DataCacheToSendBuffer(VSocket      *p_sock,
    struct iovec  iov[],
    uint32        n_iov,
    uint16        e_flag) {
  if (!p_sock && !p_sock->cli_hdl_ptr_) {
    return -1;
  }

  int32 n_head = p_sock->cli_hdl_ptr_->NetHeadSize();
  uint32 n_data = 0;
  for (uint32 i = 0; i < n_iov; i++) {
    n_data += iov[i].iov_len;
  }

  // 数据不够,分配大一点的空间
  if ((n_head + n_data) > FreeSize()) {
    Recycle();
    if ((n_head + n_data) > FreeSize()) {
      ReallocBuffer((n_head + n_data));
    }
  }
  if ((n_head + n_data) > FreeSize()) {
    return -1;
  }

  // 包头
  int32 n_head_size = p_sock->cli_hdl_ptr_->NetHeadPacket(GetWritePtr(),
                      FreeSize(),
                      n_data,
                      e_flag);
  MoveWritePtr(n_head_size);

  // body
  n_data = 0;
  for (uint32 i = 0; i < n_iov; i++) {
    memcpy(GetWritePtr(), iov[i].iov_base, iov[i].iov_len);
    MoveWritePtr(iov[i].iov_len);
    n_data += iov[i].iov_len;
  }

  return n_head_size + n_data;
}

}  // namespace vzconn
