/************************************************************************/
/* Author      : Sober.Peng 17-06-16                                    */
/* Description : 实现SOCKET的接受和发送BUFFER控制,                       */
/*                 主要针对TCP定义的收发缓冲区,缓冲区最大128K             */
/************************************************************************/
#ifndef LIBVZCONN_CBLOCKBUFFER_H_
#define LIBVZCONN_CBLOCKBUFFER_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "vzbase/base/basictypes.h"
#include "byteorder.h"

namespace vzconn {

class CBlockBuffer {
 public:
  CBlockBuffer();
  CBlockBuffer(uint8 *p_data, uint32 n_data);
  virtual ~CBlockBuffer();

 public:
  bool   ReallocBuffer(uint32 size);

  uint8* GetReadPtr();            // 获取读位置
  void   MoveReadPtr(uint32 n);   // 移动读位置

  uint8* GetWritePtr();           // 获取写位置
  void   MoveWritePtr(uint32 n);  // 移动写位置

  uint32 Length() const;          // BUFFER缓冲区大小

  uint32 UsedSize();              // 已写数据长度
  uint32 FreeSize() const;        // 剩余缓冲区长度

  bool   isFull() const;          // 数据区是否已满
  void   Recycle();               // 回收已读数据;移动已写数据到pos=0

  void   Clear();                 // 清空数据

 public:
  bool ReadUInt8(uint8* val);
  bool ReadUInt16(uint16* val);
  bool ReadUInt24(uint32* val);
  bool ReadUInt32(uint32* val);
  bool ReadUInt64(uint64* val);
  bool ReadBytes(char* val, size_t len);

  bool WriteUInt8(uint8 val);
  bool WriteUInt16(uint16 val);
  bool WriteUInt24(uint32 val);
  bool WriteUInt32(uint32 val);
  bool WriteUInt64(uint64 val);
  bool WriteBytes(const uint8* val, uint32 len);

 protected:
  void Construct(uint32 size);
  void Construct(uint8 *p_data, uint32 size);

 protected:
  uint8*    buffer_;              // 存储buffer
  uint32    buffer_size_;         // buffer长度

  uint32    read_pos_;            // 读偏移
  uint32    write_pos_;           // 写偏移

  uint32    is_out_buffer_;       // 外部buffer不能delete
};

}  // namespace vzconn
#endif  // LIBVZCONN_CBLOCKBUFFER_H_
