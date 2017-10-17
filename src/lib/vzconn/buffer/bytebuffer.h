
#ifndef VZCONN_BASE_BYTEBUFFER_H_
#define VZCONN_BASE_BYTEBUFFER_H_

#include "vzbase/base/basictypes.h"
#include "vzconn/base/basedefines.h"

#include <string>

#include "vzconn/base/vsocket.h"
#include "vzbase/base/boost_settings.hpp"

namespace vzconn {

class ByteBuffer : public vzbase::noncopyable,
  public boost::enable_shared_from_this<ByteBuffer>  {
 public:
  typedef boost::shared_ptr<ByteBuffer> Ptr;

  enum ByteOrder {
    ORDER_NETWORK = 0,  // Default, use network byte order (big endian).
    ORDER_HOST,         // Use the native order of the host.
  };

 public:
  // |byte_order| defines order of bytes in the buffer.
  ByteBuffer(size_t nhead);
  explicit ByteBuffer(size_t nhead, ByteOrder byte_order);
  ByteBuffer(size_t nhead, const char* bytes, size_t len);
  ByteBuffer(size_t nhead, const char* bytes, size_t len, ByteOrder byte_order);
  // Initializes buffer from a zero-terminated string.
  explicit ByteBuffer(size_t nhead, const char* bytes);
  ~ByteBuffer();

 public:
  ByteOrder Order() const {
    return byte_order_;
  }
  void SetByteOrder(ByteOrder byte_order) {
    byte_order_ = byte_order;
  }

 public:
  size_t Capacity() const {
    return size_;
  }

  size_t Length() const {
    return end_ - start_;
  }
  size_t size() {
    return end_ - start_;
  }

  size_t Free() const {
    return size_ - end_;
  }

  // read
  const char *DataRead() const {
    return body_ + start_;
  }

  bool MoveDataRead(size_t size) {
    if (size > Length())
      return false;
    start_ += size;
    return true;
  }

  // Read a next value from the buffer. Return false if there isn't
  // enough data left for the specified type.
  bool ReadUInt8(uint8* val);
  bool ReadUInt16(uint16* val);
  bool ReadUInt24(uint32* val);
  bool ReadUInt32(uint32* val);
  bool ReadUInt64(uint64* val);
  bool ReadBytes(char* val, size_t len);

  // Appends next |len| bytes from the buffer to |val|. Returns false
  // if there is less than |len| bytes left.
  bool ReadString(std::string* val, size_t len);

  // Write value to the buffer. Resizes the buffer when it is
  // neccessary.
  void WriteUInt8(uint8 val);
  void WriteUInt16(uint16 val);
  void WriteUInt24(uint32 val);
  void WriteUInt32(uint32 val);
  void WriteUInt64(uint64 val);
  void WriteString(const std::string& val);
  void WriteBytes(const char* val, size_t len);
  void WriteBytes(const struct iovec iov[], uint32 n_iov);

  char *DataWrite() {
    return body_ + end_;
  }
  bool MoveDataWrite(size_t size) {
    if (size > Free())
      return false;
    end_ += size;
    return true;
  }

  // Reserves the given number of bytes and returns a char* that can be written
  // into. Useful for functions that require a char* buffer and not a
  // ByteBuffer.
  char *ReserveWriteBuffer(size_t len);

  // Resize the buffer to the specified |size|. This invalidates any remembered
  // seek positions.
  void Resize(size_t size);

  void ResetSize(size_t size);

  // Clears the contents of the buffer. After this, Length() will be 0.
  void Clear();

  static void ReleaseByteBuffer(void *bytebuffer);

  // Reclear the start of the buffer
  void ReClearStart();

 private:
  void Construct(size_t nhead, const char* bytes, size_t size, ByteOrder byte_order);

 private:
  char       *body_;
  size_t      size_;
  size_t      start_;
  size_t      end_;
  int         version_;

 private:
  ByteOrder   byte_order_;

 public:
  void HeadInit(size_t nhead) {
    send_size_ = 0;
    head_size_ = nhead;
  }
  char *SendData() {
    return head_data_;
  }
  size_t SendSize() {
    return send_size_;
  }
  bool MoveDataSend(size_t size) {
    if (size > send_size_)
      return false;

    send_size_ -= size;
    return true;
  }

  char *HeadData() {
    return head_data_;
  }
  size_t HeadSize() {
    return head_size_;
  }
  bool Head(const char *phead, size_t nhead) {
    if (nhead != head_size_) {
      return false;
    }
    memcpy(head_data_, phead, head_size_);
    return true;
  }

 private:
  char       *head_data_;
  size_t      head_size_;
 public:
  size_t      send_size_;
};
}  // namespace vzconn

#endif  // VZCONN_BASE_BYTEBUFFER_H_
