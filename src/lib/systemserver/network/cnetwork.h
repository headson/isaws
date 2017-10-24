/************************************************************************/
/* Author      : SoberPeng 2017-07-08
/* Description :
/************************************************************************/
#ifndef LIBSYSTEMSERVER_CNETCTRL_H
#define LIBSYSTEMSERVER_CNETCTRL_H

#include "vzbase/base/basictypes.h"

#include "systemserver/base/pkghead.h"

#include "json/json.h"
#include "vzbase/thread/thread.h"
#include "vzconn/multicast/cmcastsocket.h"

namespace sys {

class CNetwork : vzbase::MessageHandler,
  public vzconn::CClientInterface {
 protected:
  CNetwork(vzbase::Thread *thread_fast);

 public:
  static CNetwork* Create(vzbase::Thread *thread_fast);

  virtual ~CNetwork();

  bool Start();
  void Stop();

  void Boardcast(const char *pData, int nData);

  bool GetNet(Json::Value &jnet);
  bool SetNet(const Json::Value &jnet);

 protected:
  virtual void OnMessage(vzbase::Message* msg);

  virtual int32 HandleRecvPacket(vzconn::VSocket  *p_cli,
                                 const uint8      *p_data,
                                 uint32            n_data,
                                 uint16            n_flag);
  virtual int32 HandleSendPacket(vzconn::VSocket *p_cli) {
    return 0;
  }
  virtual void  HandleClose(vzconn::VSocket *p_cli) {
  }

 protected:
  int SetIp(in_addr_t ip);
  int SetNetmask(in_addr_t ip);
  int SetGateway(in_addr_t ipaddr);
  int SetDNS(in_addr_t ip);

 private:
  vzconn::CMCastSocket *mcast_sock_;
  vzbase::Thread       *thread_fast_;

 private:
  TAG_DEV_NET           network_;
  std::string           nickname_;
  unsigned int          ip_change_;
};

}  // namespace sys

#endif  // LIBSYSTEMSERVER_CNETCTRL_H
