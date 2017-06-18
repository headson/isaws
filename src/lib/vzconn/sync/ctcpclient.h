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
  // 设置已打开的SCOKET
  virtual int32 Open(SOCKET s, bool b_block=false);

 public:
  // 链接到服务端;无需调用Open
  virtual int32 Connect(const CInetAddr *p_remote_addr,
                        bool             b_block=false,
                        bool             b_reuse=true,
                        uint32           n_timeout=5000);

 public:
  /************************************************************************/
  /* Description : 发送一包数据
  /* Parameters  : p_req[IN] 请求数据(body区)
                   n_req[IN] 请求数据长度
                   p_req_arg[IN] 请求数据参数.(VZ为包头的flag[uint16])
                   p_resp[OUT] 回执数据
                   n_resp[IN] 回执数据长度
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
