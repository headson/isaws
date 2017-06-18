/************************************************************************/
/* Author      : SoberPeng 2017-06-18
/* Description : 消息分发器中的服务器
/************************************************************************/
#ifndef LIBDISPATCHER_CDPSERVER_H
#define LIBDISPATCHER_CDPSERVER_H

#include "basictypes.h"

#include "vzconn/async/cevttcpserver.h"
#include "vzconn/async/cevtipcserver.h"

class CDpServer {
 public:
  CDpServer();
  virtual ~CDpServer();

  int32 Start(const char *s_ip, uint16 n_port);

  int32 RunLoop();

 protected:
  EVT_LOOP         c_event_;

 protected:
  CEvtIpcServer   *p_ipc_srv_;
  CEvtTcpServer   *p_tcp_srv_;
};

#endif  // LIBDISPATCHER_CDPSERVER_H
