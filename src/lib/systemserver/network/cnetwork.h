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

class CNetwork : public vzbase::MessageHandler,
  public vzconn::CClientInterface {
 protected:
  CNetwork(std::string suuid, vzbase::Thread *thread_fast);
 public:
  virtual ~CNetwork();

 public:
  static CNetwork* Create(std::string suuid, vzbase::Thread *thread_fast);

  bool Start();
  void Stop();

  bool GetNet(Json::Value &jret);
  bool SetNet(const Json::Value &jreq);

  void Boardcast(const char *pData, int nData);

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

 private:
  int SetIp(in_addr_t ip);
  int SetNetmask(in_addr_t ip);
  int SetGateway(in_addr_t ipaddr);
  int SetDNS(in_addr_t ip);

  void SetNetToSys();
  void GetNetFromSys();

 private:
  std::string           suuid_;

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
