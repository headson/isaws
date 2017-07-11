/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : 
************************************************************************/
#ifndef LIBVZCONN_CEVTIPCCLIENT_H
#define LIBVZCONN_CEVTIPCCLIENT_H

#include "vzbase/base/basictypes.h"

#include "vzconn/base/vsocket.h"
#include "vzconn/base/clibevent.h"
#include "vzconn/async/cevttcpclient.h"

namespace vzconn {

class CEvtIpcServer;
class CEvtIpcClient : public CEvtTcpClient {
 protected:
  CEvtIpcClient(const EVT_LOOP *p_loop, CClientInterface *cli_hdl);
  virtual ~CEvtIpcClient();

 public:
  static CEvtIpcClient* Create(const EVT_LOOP   *p_loop,
                               CClientInterface *cli_hdl);
 public:
  // 链接到服务端;无需调用Open
  virtual bool Connect(const CInetAddr *p_remote_addr,
                       bool             b_block,
                       bool             b_reuse,
                       uint32           n_timeout=5000);

};

}  // namespace vzconn
#endif  // LIBVZCONN_CEVTIPCCLIENT_H
