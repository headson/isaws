/************************************************************************/
/* Author      : Sober.Peng 17-06-16
/* Description : 实现SOCKET的接受和发送BUFFER控制,
                 主要针对TCP定义的收发缓冲区,缓冲区最大128K
/************************************************************************/
#ifndef LIBVZCONN_CBLOCKBUFFER_H_
#define LIBVZCONN_CBLOCKBUFFER_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "basictypes.h"

//最大缓冲大小:30M
#define DEF_BUFFER_SIZE 16 * 1024
#define MAX_BUFFER_SIZE 128 * 1024  // 128K

class CBlockBuffer {
 public:
  CBlockBuffer(uint32_t buff_size=DEF_BUFFER_SIZE);
  virtual ~CBlockBuffer();

  bool   ReallocBuffer(uint32 size);
  void   Reset();

  uint8* GetReadPtr();            // 获取读位置
  void   MoveReadPtr(uint32 n);   // 移动读位置


  uint8* GetWritePtr();           // 获取写位置
  void   MoveWritePtr(uint32 n);  // 移动写位置

  uint32 Length() const;          // BUFFER缓冲区大小

  uint32 UsedSize();              // 已写数据长度
  uint32 FreeSize() const;        // 剩余缓冲区长度

  bool   isFull() const;          // 数据区是否已满
  void   Recycle();               // 回收已读数据;移动已写数据到pos=0

 protected:
  const uint32 DEF_BUFF_SIZE;     // 默认长度

  uint8*    buffer_;              // 存储buffer
  uint32    buffer_size_;         // buffer长度

  uint32    read_pos_;            // 读偏移
  uint32    write_pos_;           // 写偏移
};

#endif  // LIBVZCONN_CBLOCKBUFFER_H_
