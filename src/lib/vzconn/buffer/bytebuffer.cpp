/************************************************************************
*Author      : Sober.Peng 17-08-22
*Description :
************************************************************************/
#include "vzconn/buffer/bytebuffer.h"

#include <algorithm>
#include <cassert>
#include <cstring>

#include "vzbase/helper/stdafx.h"
#include "vzconn/buffer/byteorder.h"

namespace vzconn {

static const int DEF_BYTE_BUFFER_SIZE = 1024;

ByteBuffer::ByteBuffer(size_t nhead) {
  Construct(nhead, NULL, DEF_BYTE_BUFFER_SIZE, ORDER_NETWORK);
}

ByteBuffer::ByteBuffer(size_t nhead, ByteOrder byte_order) {
  Construct(nhead, NULL, DEF_BYTE_BUFFER_SIZE, byte_order);
}

ByteBuffer::ByteBuffer(size_t nhead, const char *bytes, size_t len) {
  Construct(nhead, bytes, len, ORDER_NETWORK);
}

ByteBuffer::ByteBuffer(size_t nhead, const char* bytes, size_t len, ByteOrder byte_order) {
  Construct(nhead, bytes, len, byte_order);
}

ByteBuffer::ByteBuffer(size_t nhead, const char* bytes) {
  Construct(nhead, bytes, strlen(bytes), ORDER_NETWORK);
}

void ByteBuffer::Construct(size_t nhead,
                           const char *bytes, size_t len,
                           ByteOrder byte_order) {
  LOG(L_INFO) << "Construct Buffer";
  head_size_  = nhead;

  version_    = 0;
  start_      = 0;
  size_       = len;
  byte_order_ = byte_order;
  head_data_  = new char[head_size_ + size_];
  body_       = head_data_ + head_size_;
  // body_       = new char[size_];

  if (bytes) {
    end_ = len;
    memcpy(body_, bytes, end_);
  } else {
    end_ = 0;
  }
}

ByteBuffer::~ByteBuffer() {
  LOG(L_INFO) << "Release Buffer";
  if (head_data_) {
    delete[] head_data_;
    head_data_ = NULL;
  }
}

bool ByteBuffer::ReadUInt8(uint8* val) {
  if (!val) return false;

  return ReadBytes(reinterpret_cast<char*>(val), 1);
}

bool ByteBuffer::ReadUInt16(uint16* val) {
  if (!val) return false;

  uint16 v;
  if (!ReadBytes(reinterpret_cast<char*>(&v), 2)) {
    return false;
  } else {
    *val = (byte_order_ == ORDER_NETWORK) ? NetworkToHost16(v) : v;
    return true;
  }
}

bool ByteBuffer::ReadUInt24(uint32* val) {
  if (!val) return false;

  uint32 v = 0;
  char* read_into = reinterpret_cast<char*>(&v);
  if (byte_order_ == ORDER_NETWORK || IsHostBigEndian()) {
    ++read_into;
  }

  if (!ReadBytes(read_into, 3)) {
    return false;
  } else {
    *val = (byte_order_ == ORDER_NETWORK) ? NetworkToHost32(v) : v;
    return true;
  }
}

bool ByteBuffer::ReadUInt32(uint32* val) {
  if (!val) return false;

  uint32 v;
  if (!ReadBytes(reinterpret_cast<char*>(&v), 4)) {
    return false;
  } else {
    *val = (byte_order_ == ORDER_NETWORK) ? NetworkToHost32(v) : v;
    return true;
  }
}

bool ByteBuffer::ReadUInt64(uint64* val) {
  if (!val) return false;

  uint64 v;
  if (!ReadBytes(reinterpret_cast<char*>(&v), 8)) {
    return false;
  } else {
    *val = (byte_order_ == ORDER_NETWORK) ? NetworkToHost64(v) : v;
    return true;
  }
}

bool ByteBuffer::ReadString(std::string* val, size_t len) {
  if (!val) return false;

  if (len > Length()) {
    return false;
  } else {
    val->append(body_ + start_, len);
    start_ += len;
    return true;
  }
}

bool ByteBuffer::ReadBytes(char* val, size_t len) {
  if (len > Length()) {
    return false;
  } else {
    memcpy(val, body_ + start_, len);
    start_ += len;
    return true;
  }
}

void ByteBuffer::WriteUInt8(uint8 val) {
  WriteBytes(reinterpret_cast<const char*>(&val), 1);
}

void ByteBuffer::WriteUInt16(uint16 val) {
  uint16 v = (byte_order_ == ORDER_NETWORK) ? HostToNetwork16(val) : val;
  WriteBytes(reinterpret_cast<const char*>(&v), 2);
}

void ByteBuffer::WriteUInt24(uint32 val) {
  uint32 v = (byte_order_ == ORDER_NETWORK) ? HostToNetwork32(val) : val;
  char* start = reinterpret_cast<char*>(&v);
  if (byte_order_ == ORDER_NETWORK || IsHostBigEndian()) {
    ++start;
  }
  WriteBytes(start, 3);
}

void ByteBuffer::WriteUInt32(uint32 val) {
  uint32 v = (byte_order_ == ORDER_NETWORK) ? HostToNetwork32(val) : val;
  WriteBytes(reinterpret_cast<const char*>(&v), 4);
}

void ByteBuffer::WriteUInt64(uint64 val) {
  uint64 v = (byte_order_ == ORDER_NETWORK) ? HostToNetwork64(val) : val;
  WriteBytes(reinterpret_cast<const char*>(&v), 8);
}

void ByteBuffer::WriteString(const std::string& val) {
  WriteBytes(val.c_str(), val.size());
}

void ByteBuffer::WriteBytes(const char* val, size_t len) {
  if (val == NULL) {
    return;
  }
  memcpy(ReserveWriteBuffer(len), val, len);
}

void ByteBuffer::WriteBytes(const struct iovec iov[], uint32 n_iov) {
  for (uint32 i = 0; i < n_iov; i++) {
    if (!iov[i].iov_base) {
      continue;
    }

    memcpy(ReserveWriteBuffer(iov[i].iov_len),
           iov[i].iov_base, iov[i].iov_len);
  }
}

char* ByteBuffer::ReserveWriteBuffer(size_t len) {
  if (Length() + len > Capacity()) {
    Resize(Length() + len);
  }

  char* start = body_ + end_;
  end_ += len;
  return start;
}

void ByteBuffer::Resize(size_t size) {
  size_t len = min(end_ - start_, size);
  if (size <= size_) {
    // Don't reallocate, just move data backwards
    // memmove(bytes_, bytes_ + start_, len);
  } else {
    // Reallocate a larger buffer.
    size_ = max(size, 3 * size_ / 2);
#if 0
    char* new_bytes = new char[size_];
    memcpy(new_bytes, body_ + start_, len);
    delete[] body_;
    body_ = new_bytes;
#else
    char* new_bytes = new char[head_size_+size_];
    memcpy(new_bytes + head_size_, 
           body_ + head_size_ + start_, len);
    delete[] head_data_;
    head_data_ = new_bytes;
    body_ = head_data_ + head_size_;
#endif
  }
  start_ = 0;
  end_ = len;
  ++version_;
}

void ByteBuffer::ResetSize(size_t size) {
  size_t len = min(end_ - start_, size);
  if (size <= size_) {
    // Don't reallocate, just move data backwards
    // memmove(bytes_, bytes_ + start_, len);
  } else {
    // Reallocate a larger buffer.
    size_ = max(size, 3 * size_ / 2);
#if 0
    char* new_bytes = new char[size_];
    delete[] body_;
    body_ = new_bytes;
#else
    char* new_bytes = new char[head_size_ + size_];
    delete[] head_data_;
    head_data_ = new_bytes;
    body_ = head_data_ + head_size_;
#endif
  }
  start_ = 0;
  end_ = size;
  ++version_;
}

void ByteBuffer::Clear() {
  //memset(bytes_, 0, size_);
  start_ = end_ = 0;
  ++version_;
}

void ByteBuffer::ReClearStart() {
  start_ = end_ = 0;
  ++version_;
}

void ByteBuffer::ReleaseByteBuffer(void *bytebuffer) {
  if (bytebuffer) {
    ((ByteBuffer *)(bytebuffer))->Clear();
  }
}

}  // namespace vzconn
