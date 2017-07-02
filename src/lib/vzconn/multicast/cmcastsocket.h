/************************************************************************/
/* Author      : Sober.Peng 17-06-22
/* Description :
/************************************************************************/
#ifndef LIBVZCONN_CMCASTSOCKET_H
#define LIBVZCONN_CMCASTSOCKET_H

#include "vzconn/base/vsocket.h"
#include "vzconn/base/clibevent.h"
namespace vzconn {

class CMCastSocket : public vzconn::VSocket {
 protected:
  CMCastSocket(EVT_LOOP* p_loop,
               CClientInterface *c_cli_proc);

 public:
  static CMCastSocket *Create(EVT_LOOP* p_loop,
                              CClientInterface *c_cli_proc);
  virtual ~CMCastSocket();

  int32 Open(const uint8* s_center_ip, uint16 n_center_port);

  static int SendUdpData(const uint8* s_center_ip, uint16 n_center_port,
                         char* p_data, int n_data);

 protected:
  // 接收事件
  static int32  EvtRecv(SOCKET        fd,
                        short         n_events,
                        const void   *p_usr_arg);
  virtual int32 OnRecv();

 private:
  vzconn::EVT_LOOP*     p_evt_loop_;
  vzconn::EVT_IO        c_evt_recv_;

  static SOCKET         send_socket_;
};

}  // namespace vzconn
#endif  // LIBVZCONN_CMCASTSOCKET_H
