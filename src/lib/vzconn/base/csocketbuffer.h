/************************************************************************/
/* Author      : Sober.Peng 17-06-19
/* Description :
/************************************************************************/
#ifndef LIBVZCONN_CSOCKETBUFFER_H_
#define LIBVZCONN_CSOCKETBUFFER_H_

#include "cblockbuffer.h"
#include "vzconn/base/vsocket.h"

namespace vzconn {

class CSockRecvData : public CBlockBuffer {
 public:
  CSockRecvData();
  CSockRecvData(uint8 *p_data, uint32 n_data);
  virtual ~CSockRecvData();

  // �����������
  int32 RecvData(VSocket* p_sock);
  // �����ָ�����
  int32 ParseSplitData(VSocket* p_sock);

 private:
  int32  n_wait_len_;  // �ȴ����ݳ���
};

class CSockSendData : public CBlockBuffer {
 public:
  CSockSendData();
  virtual ~CSockSendData();
  // ���緢������
  int32 SendData(VSocket* p_sock);

  // ���ݻ��浽����buffer��
  int32 DataCacheToSendBuffer(VSocket    *p_sock,
                              const void *p_data,
                              uint32      n_data,
                              uint16      e_flag);

  int32 DataCacheToSendBuffer(VSocket      *p_sock,
                              struct iovec  iov[],
                              uint32        n_iov,
                              uint16        e_flag);
};

}  // namespace vzconn
#endif  // LIBVZCONN_CSOCKETBUFFER_H_
