/************************************************************************/
/* Author      : Sober.Peng 17-06-19
/* Description :
/************************************************************************/
#ifndef LIBVZCONN_CBYTEBUFFER_H_
#define LIBVZCONN_CBYTEBUFFER_H_

#include <string>

#include "basictypes.h"

#define NET_MARK_0  'V'
#define NET_MARK_1  'Z'
typedef struct _NetHead {
  uint8   mark[2];    // 'V', 'Z'
  uint16  type_flag;  // 类型标签; 0=分发消息,1=注册监听,2=取消监听,3=客户端注册
  uint32  data_size;  //
} NetHead;
static const int32 DEF_BUFFER_SIZE = 4096;

class CByteBuffer {
 public:
  enum ByteOrder {
    ORDER_NETWORK = 0,  // Default, use network byte order (big endian).
    ORDER_HOST,         // Use the native order of the host.
  };

  // |byte_order| defines order of bytes in the buffer.
  CByteBuffer();
  explicit CByteBuffer(ByteOrder byte_order);
  CByteBuffer(const char* bytes, size_t len);
  CByteBuffer(const char* bytes, size_t len, ByteOrder byte_order);

  // Initializes buffer from a zero-terminated string.
  explicit CByteBuffer(const char* bytes);

  ~CByteBuffer();

  const char* ReadData() const {
    return bytes_ + start_;
  }  
  // Moves current position |size| bytes forward. Returns false if
  // there is less than |size| bytes left in the buffer. Consume doesn't
  // permanently remove data, so remembered read positions are still valid
  // after this call.
  bool MoveReadPtr(size_t size);

  char* WriteData() {
    return bytes_ + end_;
  }
  bool MoveWritePtr(size_t size);

  size_t Length() const {
    return end_ - start_;
  }
  size_t Capacity() const {
    return size_ - start_;
  }

  ByteOrder Order() const {
    return byte_order_;
  }
  void SetByteOrder(ByteOrder byte_order) {
    byte_order_ = byte_order;
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

  // Reserves the given number of bytes and returns a char* that can be written
  // into. Useful for functions that require a char* buffer and not a
  // ByteBuffer.
  char *ReserveWriteBuffer(size_t len);

  // Resize the buffer to the specified |size|. This invalidates any remembered
  // seek positions.
  void Resize(size_t size);

  // Clears the contents of the buffer. After this, Length() will be 0.
  void Clear();

  static void ReleaseByteBuffer(void *bytebuffer);

 protected:
  void Construct(const char* bytes, size_t size, ByteOrder byte_order);

 private:
  char      *bytes_;
  size_t     size_;
  size_t     start_;
  size_t     end_;
  int        version_;
  ByteOrder  byte_order_;

 public:
  int32 NetHeadSize();
  int32 NetHeadParse();
  int32 NetHeadPacket(uint32 n_body, uint16 n_flag);

 private:
  NetHead c_head_;
};

#endif  // LIBVZCONN_CBYTEBUFFER_H_
