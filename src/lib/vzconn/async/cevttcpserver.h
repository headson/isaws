/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : 
************************************************************************/
#ifndef LIBVZCONN_CEVTTCPSERVER_H_
#define LIBVZCONN_CEVTTCPSERVER_H_

#include "vzbase/base/basictypes.h"

#include "vzconn/base/vsocket.h"
#include "vzconn/base/clibevent.h"

namespace vzconn {

class CEvtTcpServer : public VSocket {
 protected:
  CEvtTcpServer(const EVT_LOOP      *p_loop,
                CClientInterface    *cli_hdl,
                CTcpServerInterface *srv_hdl);

 public:
  static CEvtTcpServer *Create(const EVT_LOOP      *p_loop,
                               CClientInterface    *cli_hdl,
                               CTcpServerInterface *srv_hdl);

  virtual ~CEvtTcpServer();

  /************************************************************************
  *Description : 打开网络监听,等待新链接进入
  *Parameters  : p_addr[IN] 监听地址
  *              b_block[IN] true=阻塞,false=非阻塞
  *              b_reuse[IN]
  *Return      : true成功 false失败
  ***********************************************************************/
  virtual bool  Open(const CInetAddr *p_addr,
                     bool             b_block=false,
                     bool             b_reuse=true);

 protected:
  static  int32  EvtAccept(SOCKET          fd,
                           short           events,
                           const void      *p_usr_arg);
  virtual int32  OnAccept();

 protected:
  const EVT_LOOP        *p_evt_loop_;   // 随进程退出而销毁,不必关心生命周期

 protected:
  EVT_IO                c_evt_accept_;  // 接收事件

 protected:
  CClientInterface      *cli_hdl_ptr_;
  CTcpServerInterface   *srv_handle_ptr_;
};

typedef CEvtTcpServer VzTcpServer;

}  // namespace vzconn
#endif  // LIBVZCONN_CEVTTCPSERVER_H_
