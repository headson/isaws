/************************************************************************/
/* Author      : Sober.Peng 17-06-16
/* Description : ʵ��SOCKET�Ľ��ܺͷ���BUFFER����,
                 ��Ҫ���TCP������շ�������,���������128K
/************************************************************************/
#ifndef LIBVZCONN_CBLOCKBUFFER_H_
#define LIBVZCONN_CBLOCKBUFFER_H_

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "basictypes.h"
#include "byteorder.h"

namespace vzconn {

class CBlockBuffer {
 public:
  CBlockBuffer();
  CBlockBuffer(uint8 *p_data, uint32 n_data);
  virtual ~CBlockBuffer();

 public:
  bool   ReallocBuffer(uint32 size);

  uint8* GetReadPtr();            // ��ȡ��λ��
  void   MoveReadPtr(uint32 n);   // �ƶ���λ��

  uint8* GetWritePtr();           // ��ȡдλ��
  void   MoveWritePtr(uint32 n);  // �ƶ�дλ��

  uint32 Length() const;          // BUFFER��������С

  uint32 UsedSize();              // ��д���ݳ���
  uint32 FreeSize() const;        // ʣ�໺��������

  bool   isFull() const;          // �������Ƿ�����
  void   Recycle();               // �����Ѷ�����;�ƶ���д���ݵ�pos=0

  void   Clear();                 // �������

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
  uint8*    buffer_;              // �洢buffer
  uint32    buffer_size_;         // buffer����

  uint32    read_pos_;            // ��ƫ��
  uint32    write_pos_;           // дƫ��

  uint32    is_out_buffer_;       // �ⲿbuffer����delete
};

}  // namespace vzconn
#endif  // LIBVZCONN_CBLOCKBUFFER_H_
