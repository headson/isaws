/************************************************************************/
/* Author      : Sober.Peng 17-06-15
/* Description :
/************************************************************************/
#ifndef LIBVZCONN_CEVTTCPSERVER_H_
#define LIBVZCONN_CEVTTCPSERVER_H_

#include "clibevent.h"
#include "basictypes.h"

#include "vzconn/base/vsocket.h"

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

  /************************************************************************/
  /* Description : ���������,�ȴ������ӽ���
  /* Parameters  : p_addr[IN] ������ַ
                   b_block[IN] true=����,false=������
                   b_reuse[IN]
  /* Return      : 0=�ɹ� <0ʧ��
  /************************************************************************/
  virtual int32 Open(const CInetAddr* p_addr, bool b_block=false, bool b_reuse=true);

 protected:
  static  int32  EvtAccept(SOCKET          fd,
                           short           events,
                           const void      *p_usr_arg);
  virtual int32  OnAccept();

 protected:
  const EVT_LOOP        *p_evt_loop_;   // ������˳�������,���ع�����������

 protected:
  EVT_IO                c_evt_accept_;  // �����¼�

 protected:
  CClientInterface      *cli_handle_ptr_;
  CTcpServerInterface   *srv_handle_ptr_;
};

#endif  // LIBVZCONN_CEVTTCPSERVER_H_
