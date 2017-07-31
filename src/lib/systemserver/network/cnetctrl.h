/************************************************************************/
/* Author      : SoberPeng 2017-07-08
/* Description :
/************************************************************************/
#ifndef LIBSYSTEMSERVER_CNETCTRL_H
#define LIBSYSTEMSERVER_CNETCTRL_H

#include "vzbase/base/basictypes.h"

#include "json/json.h"
#include "vzbase/thread/thread.h"
#include "vzconn/multicast/cmcastsocket.h"

namespace sys {

class CNetCtrl : public vzbase::MessageHandler,
  public vzconn::CClientInterface {
 protected:
  CNetCtrl(vzbase::Thread *thread_fast);

 public:
  static CNetCtrl* Create(vzbase::Thread *thread_fast);

  virtual ~CNetCtrl();

  bool Start();
  void Stop();

  vzbase::Thread *GetThread() {
    return thread_fast_;
  }

 protected:
  virtual void  OnMessage(vzbase::Message* msg);

  virtual int32 HandleRecvPacket(vzconn::VSocket  *p_cli,
                                 const uint8      *p_data,
                                 uint32            n_data,
                                 uint16            n_flag);
  virtual int32 HandleSendPacket(vzconn::VSocket *p_cli) {
    return 0;
  }
  virtual void  HandleClose(vzconn::VSocket *p_cli) {
  }

 public:
  bool IpAddrCompare(const char *ip_addr);
  bool NetmaskCompare(const char *netmask);
  bool GatewayCompare(const char *gateway);
  bool DnsAddrCompare(const char *dns_addr);

  bool ModityNetwork(const Json::Value &jnet);

 private:
  vzconn::CMCastSocket *mcast_sock_;
  vzbase::Thread       *thread_fast_;
};

}  // namespace sys

#endif  // LIBSYSTEMSERVER_CNETCTRL_H
