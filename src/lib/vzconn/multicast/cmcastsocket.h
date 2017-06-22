/************************************************************************/
/* Author      : Sober.Peng 17-06-22
/* Description :
/************************************************************************/
#ifndef LIBVZCONN_CMCASTSOCKET_H
#define LIBVZCONN_CMCASTSOCKET_H

#include "vzconn/base/vsocket.h"
#include "vzconn/async/clibevent.h"
#include "vzconn/base/cblockbuffer.h"

class CMCastSocket : public vzconn::VSocket {
 protected:
  CMCastSocket(vzconn::EVT_LOOP* p_loop,
               vzconn::CClientInterface *c_cli_proc);

 public:
  static CMCastSocket *Create(vzconn::EVT_LOOP* p_loop,
                              vzconn::CClientInterface *c_cli_proc);
  virtual ~CMCastSocket();

  int32 Open(vzconn::CInetAddr *p_loca_addr,
             bool b_block=false,
             bool b_reuse=false);

  /************************************************************************/
  /* Description : ����һ������;���浽����cache��
  /* Parameters  : p_data[IN] ����(body��)
                   n_data[IN] ���ݳ���
                   e_flag[IN] VZΪ��ͷ��flag[uint16]
  /* Return      : >0 �������ݳ���,<=0 ����ʧ��
  /************************************************************************/
  int32 AsyncWrite(const void  *p_data,
                   uint32       n_data,
                   uint16       e_flag);

  /************************************************************************/
  /* Description : ����һ������;���浽����cache��
  /* Parameters  : iov[IN]    ����(body��)
                   n_iov[IN]  iov����
                   e_flag[IN] VZΪ��ͷ��flag[uint16]
  /* Return      : >0 �������ݳ���,<=0 ����ʧ��
  /************************************************************************/
  int32 AsyncWrite(struct iovec iov[],
                   uint32       n_iov,
                   uint16       e_flag);

 protected:
  // �����¼�
  static int32  EvtRecv(SOCKET        fd,
                        short         n_events,
                        const void   *p_usr_arg);
  virtual int32 OnRecv();

 private:
  vzconn::EVT_LOOP*     p_evt_loop_;

  vzconn::EVT_IO        c_evt_recv_;
  vzconn::CBlockBuffer  c_recv_data_;

  vzconn::CBlockBuffer  c_send_data_;
};

#endif  // LIBVZCONN_CMCASTSOCKET_H
