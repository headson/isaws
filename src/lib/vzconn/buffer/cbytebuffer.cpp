#include "vzconn/base/cbytebuffer.h"

#include <algorithm>
#include <cassert>
#include <cstring>

#include "stdafx.h"
#include "basictypes.h"
#include "vzconn/base/byteorder.h"

CByteBuffer::CByteBuffer() {
  Construct(NULL, DEF_BUFFER_SIZE, ORDER_NETWORK);
}

CByteBuffer::CByteBuffer(ByteOrder byte_order) {
  Construct(NULL, DEF_BUFFER_SIZE, byte_order);
}

CByteBuffer::CByteBuffer(const char* bytes, size_t len) {
  Construct(bytes, len, ORDER_NETWORK);
}

CByteBuffer::CByteBuffer(const char* bytes, size_t len, ByteOrder byte_order) {
  Construct(bytes, len, byte_order);
}

CByteBuffer::CByteBuffer(const char* bytes) {
  Construct(bytes, strlen(bytes), ORDER_NETWORK);
}

void CByteBuffer::Construct(const char* bytes,
                            size_t len,
                            ByteOrder byte_order) {
  version_    = 0;
  start_      = 0;
  size_       = len;
  byte_order_ = byte_order;
  bytes_      = new char[size_];

  if (bytes) {
    end_ = len;
    memcpy(bytes_, bytes, end_);
  } else {
    end_ = 0;
  }
}

CByteBuffer::~CByteBuffer() {
  LOG(L_INFO) << "Delete Buffer";
  delete[] bytes_;
}

bool CByteBuffer::ReadUInt8(uint8* val) {
  if (!val) return false;

  return ReadBytes(reinterpret_cast<char*>(val), 1);
}

bool CByteBuffer::ReadUInt16(uint16* val) {
  if (!val) return false;

  uint16 v;
  if (!ReadBytes(reinterpret_cast<char*>(&v), 2)) {
    return false;
  } else {
    *val = (byte_order_ == ORDER_NETWORK) ? NetworkToHost16(v) : v;
    return true;
  }
}

bool CByteBuffer::ReadUInt24(uint32* val) {
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

bool CByteBuffer::ReadUInt32(uint32* val) {
  if (!val) return false;

  uint32 v;
  if (!ReadBytes(reinterpret_cast<char*>(&v), 4)) {
    return false;
  } else {
    *val = (byte_order_ == ORDER_NETWORK) ? NetworkToHost32(v) : v;
    return true;
  }
}

bool CByteBuffer::ReadUInt64(uint64* val) {
  if (!val) return false;

  uint64 v;
  if (!ReadBytes(reinterpret_cast<char*>(&v), 8)) {
    return false;
  } else {
    *val = (byte_order_ == ORDER_NETWORK) ? NetworkToHost64(v) : v;
    return true;
  }
}

bool CByteBuffer::ReadString(std::string* val, size_t len) {
  if (!val) return false;

  if (len > Length()) {
    return false;
  } else {
    val->append(bytes_ + start_, len);
    start_ += len;
    return true;
  }
}

bool CByteBuffer::ReadBytes(char* val, size_t len) {
  if (len > Length()) {
    return false;
  } else {
    memcpy(val, bytes_ + start_, len);
    start_ += len;
    return true;
  }
}

void CByteBuffer::WriteUInt8(uint8 val) {
  WriteBytes(reinterpret_cast<const char*>(&val), 1);
}

void CByteBuffer::WriteUInt16(uint16 val) {
  uint16 v = (byte_order_ == ORDER_NETWORK) ? HostToNetwork16(val) : val;
  WriteBytes(reinterpret_cast<const char*>(&v), 2);
}

void CByteBuffer::WriteUInt24(uint32 val) {
  uint32 v = (byte_order_ == ORDER_NETWORK) ? HostToNetwork32(val) : val;
  char* start = reinterpret_cast<char*>(&v);
  if (byte_order_ == ORDER_NETWORK || IsHostBigEndian()) {
    ++start;
  }
  WriteBytes(start, 3);
}

void CByteBuffer::WriteUInt32(uint32 val) {
  uint32 v = (byte_order_ == ORDER_NETWORK) ? HostToNetwork32(val) : val;
  WriteBytes(reinterpret_cast<const char*>(&v), 4);
}

void CByteBuffer::WriteUInt64(uint64 val) {
  uint64 v = (byte_order_ == ORDER_NETWORK) ? HostToNetwork64(val) : val;
  WriteBytes(reinterpret_cast<const char*>(&v), 8);
}

void CByteBuffer::WriteString(const std::string& val) {
  WriteBytes(val.c_str(), val.size());
}

void CByteBuffer::WriteBytes(const char* val, size_t len) {
  memcpy(ReserveWriteBuffer(len), val, len);
}

char* CByteBuffer::ReserveWriteBuffer(size_t len) {
  if (Length() + len > Capacity())
    Resize(Length() + len);

  char* start = bytes_ + end_;
  end_ += len;
  return start;
}

void CByteBuffer::Resize(size_t size) {
  size_t len = std::min(end_ - start_, size);
  if (size <= size_) {
    // Don't reallocate, just move data backwards
    // memmove(bytes_, bytes_ + start_, len);
  } else {
    // Reallocate a larger buffer.
    size_ = std::max(size, 3 * size_ / 2);
    char* new_bytes = new char[size_];
    memcpy(new_bytes, bytes_ + start_, len);
    delete[] bytes_;
    bytes_ = new_bytes;
  }
  start_ = 0;
  end_ = len;
  ++version_;
}

bool CByteBuffer::MoveReadPtr(size_t size) {
  if ((start_ + size) > Length())
    return false;

  start_ += size;
  return true;
}

bool CByteBuffer::MoveWritePtr(size_t size) {
  if ((end_+size) > Length())
    return false;

  end_ += size;
  return true;
}

void CByteBuffer::Clear() {
  //memset(bytes_, 0, size_);
  start_ = end_ = 0;
  ++version_;
}

void CByteBuffer::ReleaseByteBuffer(void *bytebuffer) {
  if (bytebuffer) {
    ((CByteBuffer *)(bytebuffer))->Clear();
  }
}

//////////////////////////////////////////////////////////////////////////
int32 CByteBuffer::NetHeadSize() {
  return sizeof(NetHead);
}

int32 CByteBuffer::NetHeadParse() {
  if (Length() < NetHeadSize()) {
    return 0;
  }

  int32 n_len = 0;
  if ((((uint32)ReadData()) % sizeof(uint32)) == 0) {
    // 对齐解析
    NetHead* p_head = (NetHead*)ReadData();
    if (p_head->mark[0] == NET_MARK_0
        && p_head->mark[1] == NET_MARK_1) {
      n_len = NetHeadSize();
      n_len += (byte_order_ == ORDER_NETWORK) ?
               HostToNetwork32(p_head->data_size) : p_head->data_size;
    } else {
      return -1;
    }
  } else {
    // 未对齐解析
    memcpy(&c_head_, ReadData(), NetHeadSize());
    if (c_head_.mark[0] == NET_MARK_0
        && c_head_.mark[1] == NET_MARK_1) {
      n_len = NetHeadSize();
      n_len += (byte_order_ == ORDER_NETWORK) ?
               HostToNetwork32(c_head_.data_size) : c_head_.data_size;
    } else {
      return -1;
    }
  }
  return n_len;
}

int32 CByteBuffer::NetHeadPacket(uint32 n_body, uint16 n_flag) {
  c_head_.mark[0] = NET_MARK_0;
  c_head_.mark[1] = NET_MARK_1;
  c_head_.type_flag = (byte_order_ == ORDER_NETWORK) ?
                      HostToNetwork16(n_flag) : n_flag;
  c_head_.data_size = (byte_order_ == ORDER_NETWORK) ?
                      HostToNetwork16(n_body) : n_body;
  WriteBytes((char*)&c_head_, NetHeadSize());
  return NetHeadSize();
}

