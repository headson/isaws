/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : TCP客户端
************************************************************************/
#ifndef LIBVZCONN_CTCPASYNCCLIENT_H_
#define LIBVZCONN_CTCPASYNCCLIENT_H_

#include "vzbase/base/basictypes.h"

#include "vzconn/base/vsocket.h"
#include "vzconn/base/clibevent.h"
#include "vzconn/buffer/cblockbuffer.h"
#include "vzconn/async/cevttcpclient.h"

namespace vzconn {

class CTcpAsyncClient : public CEvtTcpClient {
 protected:
  CTcpAsyncClient(const EVT_LOOP *p_loop, CClientInterface *cli_hdl);
  void Remove();

 public:
  static CTcpAsyncClient *Create(const EVT_LOOP   *p_loop,
                                 CClientInterface *cli_hdl);
  virtual ~CTcpAsyncClient();

 public:
  // 链接到服务端;无需调用Open
  virtual bool  Connect(const CInetAddr *p_remote_addr,
                        bool             b_block,
                        bool             b_reuse,
                        uint32           n_timeout = 5000);

 protected:
  static int32 EvtConnect(SOCKET       fd,
                          short        events,
                          const void  *p_usr_arg);

  int32 OnConnect(SOCKET fd);
};

}  // namespace vzconn
#endif  // LIBVZCONN_CTCPASYNCCLIENT_H_
