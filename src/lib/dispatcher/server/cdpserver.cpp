/************************************************************************/
/* Author      : SoberPeng 2017-06-18
/* Description :
/************************************************************************/
#include "cdpserver.h"

#include "stdafx.h"

CDpServer::CDpServer() 
  : c_event_()
  , p_ipc_srv_(NULL) 
  , p_tcp_srv_(NULL) {
}

CDpServer::~CDpServer() {

}

int32 CDpServer::Start(const char *s_ip, uint16 n_port) {
  return 0;
}

int32 CDpServer::RunLoop() {
  return c_event_.RunLoop();
}
