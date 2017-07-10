/************************************************************************/
/* Author      : Sober.Peng 17-06-16                                    */
/* Description : 实现SOCKET的接受和发送BUFFER控制,                       */
/*                 主要针对TCP定义的收发缓冲区,缓冲区最大128K             */
/************************************************************************/
#include "cblockbuffer.h"

#include <stdio.h>

#include "vzconn/base/connhead.h"
#include "vzconn/base/byteorder.h"

namespace vzconn {
CBlockBuffer::CBlockBuffer() {
  Construct(DEF_BUFFER_SIZE);
}

CBlockBuffer::CBlockBuffer(char *p_data,
                           unsigned int n_data) {
  Construct(p_data, n_data);
}

CBlockBuffer::~CBlockBuffer() {
  read_pos_ = 0;
  write_pos_ = 0;

  if (is_out_buffer_ == false) {
    if (buffer_) {
      delete[] buffer_;
      buffer_ = NULL;
    }
  }
  buffer_size_ = 0;
}

void CBlockBuffer::Construct(unsigned int size) {
  is_out_buffer_ = false;

  read_pos_    = 0;
  write_pos_   = 0;
  buffer_size_ = size;
  buffer_      = new char[buffer_size_ * sizeof(unsigned char)];
}

void CBlockBuffer::Construct(char      *p_data,
                             unsigned int      n_data) {
  is_out_buffer_ = true;
  read_pos_     = 0;
  write_pos_    = 0;
  buffer_size_  = n_data;
  buffer_       = p_data;
}

bool CBlockBuffer::ReallocBuffer(unsigned int size) {
  // 剩余空间够用
  Recycle();
  if (size < FreeSize() ||
      buffer_size_ > MAX_BUFFER_SIZE) {
    return false;
  }

  // 分配一个1.5倍的BUFFER
  unsigned int buffer_size = 3 * buffer_size_ / 2;
  while ((buffer_size - UsedSize()) < size) { // 计算新的剩余空间是否够用
    buffer_size = 3 * buffer_size / 2;
    if (buffer_size > MAX_BUFFER_SIZE) {      // 超过了最大空间
      return false;
    }
  }

  char *new_buffer = new char[buffer_size];
  if (new_buffer) {
    //memcpy(new_buffer, GetWritePtr(), UsedSize());
    memcpy(new_buffer, buffer_, write_pos_); // 全拷贝
    delete[] buffer_;

    //read_pos_    = read_pos_;
    //write_pos_   = write_pos_;
    buffer_      = new_buffer;
    buffer_size_ = buffer_size;
    return true;
  }
  return false;
}

char* CBlockBuffer::GetReadPtr() {
  return buffer_ + read_pos_;
}

void CBlockBuffer::MoveReadPtr(unsigned int n) {
  /*if ((read_pos_+n) > buffer_size_) {
    read_pos_ = buffer_size_;
    return;
    }*/
  read_pos_ += n;
  //printf("read pos %d, buffer size %d.\n", read_pos_, buffer_size_);
}

char* CBlockBuffer::GetWritePtr() {
  return buffer_ + write_pos_;
}

void CBlockBuffer::MoveWritePtr(unsigned int n) {
  write_pos_ += n;
}

unsigned int CBlockBuffer::Length() const {
  return buffer_size_;
}

unsigned int CBlockBuffer::UsedSize() {
  if (write_pos_ > read_pos_) {
    return write_pos_ - read_pos_;
  } else {
    Recycle();
    return 0;
  }
}

unsigned int CBlockBuffer::FreeSize() const {
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

bool CBlockBuffer::WriteBytes(const unsigned char *val, unsigned int len) {
  if (FreeSize() < len) {
    Recycle();
    if (FreeSize() < len) {
      ReallocBuffer(len);
    }
  }
  if (FreeSize() < len) {
    return false;
  }

  memcpy(buffer_ + write_pos_, val, len);
  write_pos_ += len;
  return true;
}

bool CBlockBuffer::WriteBytes(const struct iovec iov[], unsigned int n_iov) {
  unsigned int n_data = 0;
  for (unsigned int i = 0; i < n_iov; i++) {
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

  for (unsigned int i = 0; i < n_iov; i++) {
    n_data += iov[i].iov_len;
    memcpy(buffer_ + write_pos_, 
           iov[i].iov_base, iov[i].iov_len);
    write_pos_ += iov[i].iov_len;
  }
  return true;
}

}  // namespace vzconn
