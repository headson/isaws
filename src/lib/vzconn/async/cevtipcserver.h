/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : 
************************************************************************/
#ifndef LIBVZCONN_CEVTIPCSERVER_H
#define LIBVZCONN_CEVTIPCSERVER_H

#include "vzbase/base/basictypes.h"

#include "vzconn/base/vsocket.h"
#include "vzconn/base/clibevent.h"
#include "vzconn/async/cevttcpserver.h"

namespace vzconn {

class CEvtIpcServer : public CEvtTcpServer {
 private:
  CEvtIpcServer(const EVT_LOOP      *p_loop,
                CClientInterface    *cli_hdl,
                CTcpServerInterface *srv_hdl);

 public:
  static CEvtIpcServer *Create(const EVT_LOOP      *p_loop,
                               CClientInterface    *cli_hdl,
                               CTcpServerInterface *srv_hdl);

  virtual ~CEvtIpcServer();

  /***********************************************************************
  *Description : 打开网络监听,等待新链接进入
  *Parameters  : p_addr[IN]  监听地址
  *              b_block[IN] true=阻塞,false=非阻塞
  *              b_reuse[IN]
  *Return      : true成功 false失败
  ***********************************************************************/
  bool Open(const CInetAddr* p_addr, bool b_block=false, bool b_reuse=true);

 private:
  virtual int32  OnAccept();
};

}  // namespace vzconn

#endif  // LIBVZCONN_CEVTIPCSERVER_H
