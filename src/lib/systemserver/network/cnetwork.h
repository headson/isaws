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

<<<<<<< HEAD:src/lib/systemserver/network/cnetwork.h
class CNetwork : vzbase::MessageHandler,
  public vzconn::CClientInterface {
 protected:
  CNetwork(vzbase::Thread *thread_fast);

 public:
  static CNetwork* Create(vzbase::Thread *thread_fast);

  virtual ~CNetwork();
=======
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
>>>>>>> 7bf3daa37cb02fa856739e7bd691a4a2bfd6f2cf:src/lib/systemserver/network/cnetwork.h

  bool Start();
  void Stop();

<<<<<<< HEAD:src/lib/systemserver/network/cnetwork.h
  void Boardcast(const char *pData, int nData);

  bool GetNet(Json::Value &jnet);
  bool SetNet(const Json::Value &jnet);
=======
  bool OnDpMessage(const DpMessage *dmp,
                   const Json::Value &jreq, Json::Value &jret);
>>>>>>> 7bf3daa37cb02fa856739e7bd691a4a2bfd6f2cf:src/lib/systemserver/network/cnetwork.h

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
<<<<<<< HEAD:src/lib/systemserver/network/cnetwork.h
=======
  bool SysBngSetNet();
  bool OnCheckNetAddr();

  bool OnGetNet(Json::Value &jbody);
  bool OnSetNet(const Json::Value &jbody);

 protected:
>>>>>>> 7bf3daa37cb02fa856739e7bd691a4a2bfd6f2cf:src/lib/systemserver/network/cnetwork.h
  int SetIp(in_addr_t ip);
  int SetNetmask(in_addr_t ip);
  int SetGateway(in_addr_t ipaddr);
  int SetDNS(in_addr_t ip);
<<<<<<< HEAD:src/lib/systemserver/network/cnetwork.h
=======

 public:
  void BcastDevInfo();
>>>>>>> 7bf3daa37cb02fa856739e7bd691a4a2bfd6f2cf:src/lib/systemserver/network/cnetwork.h

 private:
  vzbase::Thread       *thread_fast_;

 private:
<<<<<<< HEAD:src/lib/systemserver/network/cnetwork.h
  TAG_DEV_NET           network_;
  std::string           nickname_;
  unsigned int          ip_change_;
=======
  vzconn::CMCastSocket *mcast_sock_;

 private:
  TAG_DEV_NET           dev_net_;
  std::string           nickname_;      // eth0\wlan0
>>>>>>> 7bf3daa37cb02fa856739e7bd691a4a2bfd6f2cf:src/lib/systemserver/network/cnetwork.h
};

}  // namespace sys

#endif  // LIBSYSTEMSERVER_CNETCTRL_H
