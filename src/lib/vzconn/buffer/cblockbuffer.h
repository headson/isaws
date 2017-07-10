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

#include "vzconn/base/vsocket.h"
#include "vzconn/base/byteorder.h"

namespace vzconn {

class CBlockBuffer {
 public:
  CBlockBuffer();
  CBlockBuffer(char *p_data, unsigned int n_data);
  virtual ~CBlockBuffer();

 public:
  bool   ReallocBuffer(unsigned int size);

  char* GetReadPtr();            // 获取读位置
  void   MoveReadPtr(unsigned int n);   // 移动读位置

  char* GetWritePtr();           // 获取写位置
  void   MoveWritePtr(unsigned int n);  // 移动写位置

  unsigned int Length() const;          // BUFFER缓冲区大小

  unsigned int UsedSize();              // 已写数据长度
  unsigned int FreeSize() const;        // 剩余缓冲区长度

  bool   isFull() const;          // 数据区是否已满
  void   Recycle();               // 回收已读数据;移动已写数据到pos=0

  void   Clear();                 // 清空数据

 public:
  bool WriteBytes(const unsigned char *val, unsigned int len);
  bool WriteBytes(const struct iovec iov[], unsigned int n_iov);

 protected:
  void Construct(unsigned int size);
  void Construct(char *p_data, unsigned int size);

 protected:
  char*    buffer_;              // 存储buffer
  unsigned int    buffer_size_;         // buffer长度

  unsigned int    read_pos_;            // 读偏移
  unsigned int    write_pos_;           // 写偏移

 protected:
  unsigned int    is_out_buffer_;       // 外部buffer不能delete
};

}  // namespace vzconn
#endif  // LIBVZCONN_CBLOCKBUFFER_H_
