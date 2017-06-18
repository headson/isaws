/************************************************************************/
/* Author      : SoberPeng 2017-06-17
/* Description :
/************************************************************************/
#ifndef LIBVZCONN_CEVTIPCSERVER_H
#define LIBVZCONN_CEVTIPCSERVER_H

#include "clibevent.h"
#include "basictypes.h"

#include "vzconn/base/vsocket.h"
#include "vzconn/async/cevttcpserver.h"

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

  /************************************************************************/
  /* Description : ���������,�ȴ������ӽ���
  /* Parameters  : p_addr[IN] ������ַ
                   b_block[IN] true=����,false=������
                   b_reuse[IN]
  /* Return      : 0=�ɹ� <0ʧ��
  /************************************************************************/
  int32 Open(const CInetAddr* p_addr, bool b_block=false, bool b_reuse=true);

 private:
  virtual int32  OnAccept();
};

#endif  // LIBVZCONN_CEVTIPCSERVER_H
