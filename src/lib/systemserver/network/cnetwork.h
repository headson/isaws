/************************************************************************/
/* Author      : SoberPeng 2017-07-08
/* Description :
/************************************************************************/
#ifndef LIBSYSTEMSERVER_CNETCTRL_H
#define LIBSYSTEMSERVER_CNETCTRL_H

#include "vzbase/base/basictypes.h"
#include "dispatcher/base/pkghead.h"

#include "systemserver/system/csystem.h"
#include "systemserver/base/basedefines.h"

#include "json/json.h"
#include "vzbase/thread/thread.h"
#include "vzconn/multicast/cmcastsocket.h"

namespace sys {

class CNetwork : public vzbase::MessageHandler,
  public vzconn::CClientInterface {
 protected:
  CNetwork(vzbase::Thread *thread_fast);
 public:
  virtual ~CNetwork();

 public:
  static CNetwork* Create(vzbase::Thread *thread_fast);
  vzbase::Thread *GetThread() {
    return thread_fast_;
  }

  bool Start();
  void Stop();

  bool OnDpMessage(const DpMessage *dmp,
                   const Json::Value &jreq, Json::Value &jret);

 protected:
  virtual void OnMessage(vzbase::Message* msg);

 protected:
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
  bool SysBngSetNet();
  bool OnCheckNetAddr();

  bool OnGetNet(Json::Value &jbody);
  bool OnSetNet(const Json::Value &jbody);

 protected:
  int SetIp(in_addr_t ip);
  int SetNetmask(in_addr_t ip);
  int SetGateway(in_addr_t ipaddr);
  int SetDNS(in_addr_t ip);

 public:
  void BcastDevInfo();

 private:
  vzbase::Thread       *thread_fast_;

 private:
  vzconn::CMCastSocket *mcast_sock_;

 private:
  TAG_DEV_NET           dev_net_;
  std::string           nickname_;      // eth0\wlan0
};

}  // namespace sys

#endif  // LIBSYSTEMSERVER_CNETCTRL_H
