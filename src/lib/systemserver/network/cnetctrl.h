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

class CNetCtrl : public vzbase::MessageHandler,
  public vzconn::CClientInterface {
 protected:
  CNetCtrl(vzbase::Thread *p_thread);

 public:
  static CNetCtrl* Create(vzbase::Thread *p_thread);

  virtual ~CNetCtrl();

  bool Start();
  void Stop();

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
  vzbase::Thread        *p_thread_;
  vzconn::CMCastSocket  *p_mcast_sock_;
};

#endif  // LIBSYSTEMSERVER_CNETCTRL_H
