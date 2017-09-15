/************************************************************************
*Author      : Sober.Peng 17-07-10
*Description :
************************************************************************/
#ifndef LIBVZCONN_CMCASTSOCKET_H_
#define LIBVZCONN_CMCASTSOCKET_H_

#include "vzconn/base/vsocket.h"
#include "vzconn/base/clibevent.h"
namespace vzconn {

class CMCastSocket : public vzconn::VSocket {
 protected:
  CMCastSocket(EVT_LOOP* p_loop,
               CClientInterface *c_cli_proc);

 public:
  static CMCastSocket *Create(EVT_LOOP* p_loop,
                              CClientInterface *cli_hdl);
  virtual ~CMCastSocket();

  int Open(const char* center_ip, int center_port);
  void Stop();

  int SendUdpData(const char* center_ip, int center_port,
                  const char* pdata, unsigned int ndata,
                  const char *ip_addr=NULL);

 protected:
  // 接收事件
  static int32  EvtRecv(SOCKET        fd,
                        short         n_events,
                        const void   *p_usr_arg);
  virtual int32 OnRecv();

 private:
  vzconn::EVT_LOOP*     evt_loop_;
  vzconn::EVT_IO        evt_recv_;

  struct ip_mreq        mreq;
};

}  // namespace vzconn
#endif  // LIBVZCONN_CMCASTSOCKET_H_
