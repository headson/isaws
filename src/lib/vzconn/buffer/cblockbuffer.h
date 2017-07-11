/************************************************************************/
/* Author      : Sober.Peng 17-06-16                                    */
/* Description : ʵ��SOCKET�Ľ��ܺͷ���BUFFER����,                       */
/*                 ��Ҫ���TCP������շ�������,���������128K             */
/************************************************************************/
#ifndef LIBVZCONN_CBLOCKBUFFER_H_
#define LIBVZCONN_CBLOCKBUFFER_H_

#include "vzbase/base/basictypes.h"

#include "vzconn/base/vsocket.h"

namespace vzconn {

class CBlockBuffer {
 public:
  CBlockBuffer();
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
  bool WriteBytes(const uint8 *val, uint32 len);
  bool WriteBytes(const struct iovec iov[], uint32 n_iov);

 protected:
  uint8*    buffer_;              // �洢buffer
  uint32    buffer_size_;         // buffer����

  uint32    read_pos_;            // ��ƫ��
  uint32    write_pos_;           // дƫ��
};

}  // namespace vzconn
#endif  // LIBVZCONN_CBLOCKBUFFER_H_
