/************************************************************************/
/* Author      : SoberPeng 2017-06-17
/* Description :
/************************************************************************/
#ifndef LIBVZCONN_CTCPCLIENT_H
#define LIBVZCONN_CTCPCLIENT_H

#include "basictypes.h"

#include "vzconn/base/vsocket.h"
#include "vzconn/base/cblockbuffer.h"

class CTcpClient : public VSocket {
 protected:
  CTcpClient();

 public:
  static CTcpClient* Create();

  virtual ~CTcpClient();

 public:
  // �����Ѵ򿪵�SCOKET
  virtual int32 Open(SOCKET s, bool b_block=false);

 public:
  // ���ӵ������;�������Open
  virtual int32 Connect(const CInetAddr *p_remote_addr,
                        bool             b_block=false,
                        bool             b_reuse=true,
                        uint32           n_timeout=5000);

 public:
  /************************************************************************/
  /* Description : ����һ������
  /* Parameters  : p_req[IN] ��������(body��)
                   n_req[IN] �������ݳ���
                   p_req_arg[IN] �������ݲ���.(VZΪ��ͷ��flag[uint16])
                   p_resp[OUT] ��ִ����
                   n_resp[IN] ��ִ���ݳ���
  /* Return      :
  /************************************************************************/
  virtual int32 SendReqWithResp(const void  *p_req,
                                uint32       n_req,
                                const void  *p_req_arg,
                                void        *p_resp,
                                uint32       n_resp);

 protected:
   CBlockBuffer c_sock_data_;
};

#endif  // LIBVZCONN_CTCPCLIENT_H
