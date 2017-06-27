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

CBlockBuffer::CBlockBuffer(uint8 *p_data,
                           uint32 n_data) {
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

void CBlockBuffer::Construct(uint32 size) {
  is_out_buffer_ = false;

  read_pos_    = 0;
  write_pos_   = 0;
  buffer_size_ = size;
  buffer_      = new uint8[buffer_size_ * sizeof(uint8)];
}

void CBlockBuffer::Construct(uint8      *p_data,
                             uint32      n_data) {
  is_out_buffer_ = true;
  read_pos_     = 0;
  write_pos_    = 0;
  buffer_size_  = n_data;
  buffer_       = p_data;
}

bool CBlockBuffer::ReallocBuffer(uint32 size) {
  // 剩余空间够用
  Recycle();
  if (size < FreeSize() ||
      buffer_size_ > MAX_BUFFER_SIZE) {
    return false;
  }

  // 分配一个1.5倍的BUFFER
  uint32 buffer_size = 3 * buffer_size_ / 2;
  while ((buffer_size - UsedSize()) < size) { // 计算新的剩余空间是否够用
    buffer_size = 3 * buffer_size / 2;
    if (buffer_size > MAX_BUFFER_SIZE) {      // 超过了最大空间
      return false;
    }
  }

  uint8 *new_buffer = new uint8[buffer_size];
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

bool CBlockBuffer::ReadUInt8(uint8* val) {
  if (!val)
    return false;

  return ReadBytes(reinterpret_cast<char*>(val), 1);
}

bool CBlockBuffer::ReadUInt16(uint16* val) {
  if (!val)
    return false;

  uint16 v;
  if (!ReadBytes(reinterpret_cast<char*>(&v), 2)) {
    return false;
  }
  *val = (VZ_ORDER_BYTE == ORDER_NETWORK) ? NetworkToHost16(v) : v;
  return true;
}

bool CBlockBuffer::ReadUInt24(uint32* val) {
  if (!val)
    return false;

  uint32 v = 0;
  char* read_into = reinterpret_cast<char*>(&v);
  if (VZ_ORDER_BYTE == ORDER_NETWORK || IsHostBigEndian()) {
    ++read_into;
  }

  if (!ReadBytes(read_into, 3)) {
    return false;
  }

  *val = (VZ_ORDER_BYTE == ORDER_NETWORK) ? NetworkToHost32(v) : v;
  return true;
}

bool CBlockBuffer::ReadUInt32(uint32* val) {
  if (!val)
    return false;

  uint32 v;
  if (!ReadBytes(reinterpret_cast<char*>(&v), 4)) {
    return false;
  }

  *val = (VZ_ORDER_BYTE == ORDER_NETWORK) ? NetworkToHost32(v) : v;
  return true;
}

bool CBlockBuffer::ReadUInt64(uint64* val) {
  if (!val) return false;

  uint64 v;
  if (!ReadBytes(reinterpret_cast<char*>(&v), 8)) {
    return false;
  }
  *val = (VZ_ORDER_BYTE == ORDER_NETWORK) ? NetworkToHost64(v) : v;
  return true;
}

bool CBlockBuffer::ReadBytes(char* val, size_t len) {
  if (len > UsedSize()) {
    return false;
  }

  memcpy(val, buffer_ + read_pos_, len);
  read_pos_ += len;
  return true;
}

bool CBlockBuffer::WriteUInt8(uint8 val) {
  return WriteBytes(reinterpret_cast<const uint8*>(&val), 1);
}

bool CBlockBuffer::WriteUInt16(uint16 val) {
  uint16 v = (VZ_ORDER_BYTE == ORDER_NETWORK) ? HostToNetwork16(val) : val;
  return WriteBytes(reinterpret_cast<const uint8*>(&v), 2);
}

bool CBlockBuffer::WriteUInt24(uint32 val) {
  uint32 v = (VZ_ORDER_BYTE == ORDER_NETWORK) ? HostToNetwork32(val) : val;
  uint8* start = reinterpret_cast<uint8*>(&v);
  if (VZ_ORDER_BYTE == ORDER_NETWORK || IsHostBigEndian()) {
    ++start;
  }
  return WriteBytes(start, 3);
}

bool CBlockBuffer::WriteUInt32(uint32 val) {
  uint32 v = (VZ_ORDER_BYTE == ORDER_NETWORK) ? HostToNetwork32(val) : val;
  return WriteBytes(reinterpret_cast<const uint8*>(&v), 4);
}

bool CBlockBuffer::WriteUInt64(uint64 val) {
  uint64 v = (VZ_ORDER_BYTE == ORDER_NETWORK) ? HostToNetwork64(val) : val;
  return WriteBytes(reinterpret_cast<const uint8*>(&v), 8);
}

bool CBlockBuffer::WriteBytes(const uint8* val, uint32 len) {
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

}  // namespace vzconn
