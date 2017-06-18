/************************************************************************/
/* Author      : Sober.Peng 17-06-16
/* Description : 实现SOCKET的接受和发送BUFFER控制,
                 主要针对TCP定义的收发缓冲区,缓冲区最大128K
/************************************************************************/
#include "cblockbuffer.h"

#include <stdio.h>

CBlockBuffer::CBlockBuffer(uint32_t buff_size) 
  : DEF_BUFF_SIZE(buff_size) {
  read_pos_ = 0;
  write_pos_ = 0;
  buffer_size_ = DEF_BUFF_SIZE;
  buffer_ = (uint8 *)malloc(buffer_size_ * sizeof(uint8));
}

CBlockBuffer::~CBlockBuffer() {
  read_pos_ = 0;
  write_pos_ = 0;

  if (buffer_) {
    free(buffer_);
    buffer_ = NULL;
  }
  buffer_size_ = 0;
}

bool CBlockBuffer::ReallocBuffer(uint32 size) {
  // 分配一个2倍的BUFFER
  // 剩余空间够用
  if (size < FreeSize() || buffer_size_ > MAX_BUFFER_SIZE)
    return false;

  uint32 buffer_size = buffer_size_ * 2;
  while (buffer_size - UsedSize() < size) { // 计算新的剩余空间是否够用
    buffer_size = buffer_size * 2;
  }

  buffer_ = (uint8 *)realloc(buffer_, buffer_size);
  buffer_size_ = buffer_size;
  return true;
}

void CBlockBuffer::Reset() {
  if (buffer_size_ > DEF_BUFF_SIZE) {
    free(buffer_);

    buffer_size_ = DEF_BUFF_SIZE;
    buffer_ = (uint8 *)malloc(buffer_size_ * sizeof(uint8));
  }
  memset(buffer_, 0x00, buffer_size_);

  read_pos_  = 0;
  write_pos_ = 0;
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
