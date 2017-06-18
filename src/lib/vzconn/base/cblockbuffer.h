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

//��󻺳��С:30M
#define DEF_BUFFER_SIZE 16 * 1024
#define MAX_BUFFER_SIZE 128 * 1024  // 128K

class CBlockBuffer {
 public:
  CBlockBuffer(uint32_t buff_size=DEF_BUFFER_SIZE);
  virtual ~CBlockBuffer();

  bool   ReallocBuffer(uint32 size);
  void   Reset();

  uint8* GetReadPtr();            // ��ȡ��λ��
  void   MoveReadPtr(uint32 n);   // �ƶ���λ��


  uint8* GetWritePtr();           // ��ȡдλ��
  void   MoveWritePtr(uint32 n);  // �ƶ�дλ��

  uint32 Length() const;          // BUFFER��������С

  uint32 UsedSize();              // ��д���ݳ���
  uint32 FreeSize() const;        // ʣ�໺��������

  bool   isFull() const;          // �������Ƿ�����
  void   Recycle();               // �����Ѷ�����;�ƶ���д���ݵ�pos=0

 protected:
  const uint32 DEF_BUFF_SIZE;     // Ĭ�ϳ���

  uint8*    buffer_;              // �洢buffer
  uint32    buffer_size_;         // buffer����

  uint32    read_pos_;            // ��ƫ��
  uint32    write_pos_;           // дƫ��
};

#endif  // LIBVZCONN_CBLOCKBUFFER_H_
