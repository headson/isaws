/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : TCP客户端
************************************************************************/
#ifndef LIBVZCONN_CTCPCLIENT_H_
#define LIBVZCONN_CTCPCLIENT_H_

#include "vzbase/base/basictypes.h"

#include "vzconn/base/vsocket.h"
#include "vzconn/base/clibevent.h"
#include "vzconn/buffer/cblockbuffer.h"
#include "vzconn/async/cevttcpclient.h"

namespace vzconn {

class CTcpClient : public CEvtTcpClient {
 protected:
  CTcpClient(const EVT_LOOP *p_loop, CClientInterface *cli_hdl);
  void Remove();

 public:
  static CTcpClient* Create(const EVT_LOOP   *p_loop,
                            CClientInterface *cli_hdl);
  virtual ~CTcpClient();

 public:
  // 此函数内指定使用SubEvtRecv\SubEvtSend
  bool Open(SOCKET s, bool b_block);
  // 链接到服务端;无需调用Open
  virtual bool  Connect(const CInetAddr *p_remote_addr,
                        bool             b_block,
                        bool             b_reuse,
                        uint32           n_timeout=5000);

 public:
  virtual int32 SyncWrite(const void  *p_data,
                          uint32       n_data,
                          uint16       e_flag);

  virtual int32 SyncWrite(struct iovec iov[],
                          uint32       n_iov,
                          uint16       e_flag);
 protected:
  // 父类会delete自己
  static int32 SubEvtRecv(SOCKET      fd,
                          short       events,
                          const void *usr_arg);
  // 父类会delete自己
  static int32 SubEvtSend(SOCKET      fd,
                          short       events,
                          const void *p_usr_arg);

 protected:
  int32 SendN(const uint8 *p_data, uint32 n_data);
};

}  // namespace vzconn
#endif  // LIBVZCONN_CTCPCLIENT_H_
