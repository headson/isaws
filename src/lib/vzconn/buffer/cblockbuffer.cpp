/************************************************************************/
/* Author      : Sober.Peng 17-06-16                                    */
/* Description : 实现SOCKET的接受和发送BUFFER控制,                       */
/*                 主要针对TCP定义的收发缓冲区,缓冲区最大128K             */
/************************************************************************/
#include "cblockbuffer.h"
#include "vzconn/base/basedefines.h"

#include <stdio.h>

#include "vzbase/helper/stdafx.h"

#include "vzconn/base/vsocket.h"
#include "vzconn/buffer/byteorder.h"

namespace vzconn {

CBlockBuffer::CBlockBuffer() {
  read_pos_ = 0;
  write_pos_ = 0;
  buffer_size_ = SOCK_DEF_BUFFER_SIZE;
  buffer_ = new uint8[buffer_size_ * sizeof(uint8)];
}

CBlockBuffer::~CBlockBuffer() {
  read_pos_ = 0;
  write_pos_ = 0;

  if (buffer_) {
    delete[] buffer_;
    buffer_ = NULL;
  }
  buffer_size_ = 0;
}

bool CBlockBuffer::ReallocBuffer(uint32 size) {
  // 剩余空间够用
  Recycle();
  if (size < FreeSize() ||
      size > SOCK_MAX_BUFFER_SIZE ||
      buffer_size_ > SOCK_MAX_BUFFER_SIZE) {
    return false;
  }

  // 分配一个1.5倍的BUFFER
  uint32 nused = UsedSize();
  uint32 new_size = 0;
  do {  // 计算新的剩余空间是否够用
    new_size = 3 * new_size / 2;
    if (new_size > SOCK_MAX_BUFFER_SIZE) {  // 超过了最大空间
      return false;
    }
    LOG_INFO("--------------- %d. %d. %d.", new_size, (new_size - nused), size);
  }while ((new_size - nused) < size);

  uint8 *new_buffer = new uint8[new_size];
  if (new_buffer) {
    //memcpy(new_buffer, GetWritePtr(), UsedSize());
    memcpy(new_buffer, buffer_, write_pos_); // 全拷贝
    delete[] buffer_;

    //read_pos_    = read_pos_;
    //write_pos_   = write_pos_;
    buffer_      = new_buffer;
    buffer_size_ = new_size;
    return true;
  }
  return false;
}

uint8* CBlockBuffer::GetReadPtr() {
  return buffer_ + read_pos_;
}

void CBlockBuffer::MoveReadPtr(uint32 n) {
  /*if ((read_pos_+n) > buffer_size_) {
    read_pos_ = buffer_size_;
    return;
    }*/
  read_pos_ += n;
  //printf("read pos %d, buffer size %d.\n", read_pos_, buffer_size_);
}

uint8* CBlockBuffer::GetWritePtr() {
  return buffer_ + write_pos_;
}

void CBlockBuffer::MoveWritePtr(uint32 n) {
  write_pos_ += n;
}

uint32 CBlockBuffer::Length() const {
  return buffer_size_;
}

uint32 CBlockBuffer::UsedSize() {
  if (write_pos_ > read_pos_) {
    return write_pos_ - read_pos_;
  } else {
    Recycle();
    return 0;
  }
}

uint32 CBlockBuffer::FreeSize() const {
  return buffer_size_ - write_pos_;
}

bool CBlockBuffer::isFull() const {
  return (FreeSize() == 0);
}

void CBlockBuffer::Recycle() {
  if (write_pos_ <= read_pos_) {
    write_pos_ = 0;
    read_pos_ = 0;
  } else if (read_pos_ > 0) {
    memmove(buffer_, buffer_ + read_pos_, write_pos_ - read_pos_);
    write_pos_ = write_pos_ - read_pos_;
    read_pos_ = 0;
  }
}

void CBlockBuffer::Clear() {
  write_pos_ = read_pos_ = 0;
}

bool CBlockBuffer::WriteBytes(const uint8 *val, uint32 len) {
  if (FreeSize() < len) {
    Recycle();
    if (FreeSize() < len) {
      ReallocBuffer(len);
    }
  }
  if (FreeSize() < len) {
    // 分配失败反馈
    return false;
  }

  memcpy(buffer_ + write_pos_, val, len);
  write_pos_ += len;
  return true;
}

bool CBlockBuffer::WriteBytes(const struct iovec iov[], uint32 n_iov) {
  uint32 n_data = 0;
  for (uint32 i = 0; i < n_iov; i++) {
    n_data += iov[i].iov_len;
  }

  if (FreeSize() < n_data) {
    Recycle();
    if (FreeSize() < n_data) {
      ReallocBuffer(n_data);
    }
  }
  if (FreeSize() < n_data) {
    return false;
  }

  for (uint32 i = 0; i < n_iov; i++) {
    if (!iov[i].iov_base || iov[i].iov_len <= 0) {
      continue;
    }

    n_data += iov[i].iov_len;
    memcpy(buffer_ + write_pos_,
           iov[i].iov_base, iov[i].iov_len);
    write_pos_ += iov[i].iov_len;
  }
  return true;
}

}  // namespace vzconn
