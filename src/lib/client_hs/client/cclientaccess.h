/************************************************************************
* Author      : SoberPeng 2017-10-28
* Description :
************************************************************************/
#ifndef LIBCLIENT_HS_CCLIENTACCESS_H
#define LIBCLIENT_HS_CCLIENTACCESS_H

#include "vzbase/thread/thread.h"
#include "vzconn/sync/ctcpasyncclient.h"
#include "vzbase/base/boost_settings.hpp"

#define AES_TIMER_TIMEOUT   (5 * 1000)

class CClientAccess : vzbase::noncopyable,
  public vzconn::CClientInterface,
  public vzbase::MessageHandler,
  public boost::enable_shared_from_this < CClientAccess > {

 public:
  typedef boost::shared_ptr<CClientAccess> Ptr;

 public:
  CClientAccess(vzbase::Thread *pthread);
  virtual ~CClientAccess();

  bool Start(const std::string &sAesHost, unsigned int nAesPort);

 public:
  virtual uint32 NetHeadSize();
  virtual int32 NetHeadParse(const uint8 *pdata, uint32 ndata, uint16 *nflag);
  virtual int32 NetHeadPacket(uint8 *pdata, uint32 ndata, uint32 nbody,
                              uint16  nflag);

 protected:
  virtual int32 HandleConnected(vzconn::VSocket *cli);
  virtual int32 HandleRecvPacket(vzconn::VSocket *cli,
                                 const uint8 *pdata, uint32 ndata,
                                 uint16 nflag);
  virtual int32 HandleSendPacket(vzconn::VSocket *cli);
  virtual void HandleClose(vzconn::VSocket *cli);

  virtual void OnMessage(vzbase::Message *msg);

 private:
  vzbase::Thread           *thread_;

 private:
  std::string               server_host_;
  unsigned int              server_port_;
  vzconn::CTcpAsyncClient  *server_connect_;
};

#endif  // LIBCLIENT_HS_CCLIENTACCESS_H
