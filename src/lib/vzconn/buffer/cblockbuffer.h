/************************************************************************/
/* Author      : Sober.Peng 17-06-16                                    */
/* Description : ʵ��SOCKET�Ľ��ܺͷ���BUFFER����,                       */
/*                 ��Ҫ���TCP������շ�������,���������128K             */
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

  char* GetReadPtr();            // ��ȡ��λ��
  void   MoveReadPtr(unsigned int n);   // �ƶ���λ��

  char* GetWritePtr();           // ��ȡдλ��
  void   MoveWritePtr(unsigned int n);  // �ƶ�дλ��

  unsigned int Length() const;          // BUFFER��������С

  unsigned int UsedSize();              // ��д���ݳ���
  unsigned int FreeSize() const;        // ʣ�໺��������

  bool   isFull() const;          // �������Ƿ�����
  void   Recycle();               // �����Ѷ�����;�ƶ���д���ݵ�pos=0

  void   Clear();                 // �������

 public:
  bool WriteBytes(const unsigned char *val, unsigned int len);
  bool WriteBytes(const struct iovec iov[], unsigned int n_iov);

 protected:
  void Construct(unsigned int size);
  void Construct(char *p_data, unsigned int size);

 protected:
  char*    buffer_;              // �洢buffer
  unsigned int    buffer_size_;         // buffer����

  unsigned int    read_pos_;            // ��ƫ��
  unsigned int    write_pos_;           // дƫ��

 protected:
  unsigned int    is_out_buffer_;       // �ⲿbuffer����delete
};

}  // namespace vzconn
#endif  // LIBVZCONN_CBLOCKBUFFER_H_
