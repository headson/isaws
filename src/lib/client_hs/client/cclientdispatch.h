/************************************************************************
* Author      : SoberPeng 2017-11-04
* Description :
************************************************************************/
#ifndef LIBCLIENT_HS_CCLIENTDISPATCH_H
#define LIBCLIENT_HS_CCLIENTDISPATCH_H

#include "vzbase/thread/thread.h"
#include "vzbase/base/boost_settings.hpp"
#include "client_hs/client/ctcpasyncclient.h"

#define DIS_TIMER_TIMEOUT   (5 * 1000)

class CClientDispatch : public vzconn::CClientInterface,
  public vzbase::MessageHandler {
public:
  CClientDispatch(vzbase::Thread *pthread);
  virtual ~CClientDispatch();

  bool Start(const std::string &sDisHost, unsigned int nDisPort, 
             unsigned int video_channel);
protected:
  virtual uint32 NetHeadSize();
  virtual int32 NetHeadParse(const uint8 *pdata, uint32 ndata, uint16 *head_size);
  virtual int32 NetHeadPacket(uint8 *pdata, uint32 ndata, uint32 nbody,
                              uint16 nflag);
protected:
  virtual int32 HandleConnected(vzconn::VSocket *cli);
  virtual int32 HandleRecvPacket(vzconn::VSocket *cli,
                                 const uint8 *pdata, uint32 ndata,
                                 uint16 head_size);
  virtual int32 HandleSendPacket(vzconn::VSocket *cli);
  virtual void HandleClose(vzconn::VSocket *cli);

  virtual void OnMessage(vzbase::Message *msg);

private:
  vzbase::Thread           *thread_;

private:
  std::string               server_host_;
  unsigned int              server_port_;
  unsigned int              video_channel_;
  cli::CTcpAsyncClient     *server_connect_;
};

#endif  // LIBCLIENT_HS_CCLIENTDISPATCH_H
