/************************************************************************
* Author      : SoberPeng 2017-11-04
* Description :
************************************************************************/
#ifndef LIBCLIENT_HS_CCLIENTDISPATCH_H
#define LIBCLIENT_HS_CCLIENTDISPATCH_H

#include "vzbase/thread/thread.h"
#include "vzbase/base/boost_settings.hpp"
#include "client_hs/client/cclientaccess.h"
#include "client_hs/client/ctcpasyncclient.h"

#define DIS_TIMER_TIMEOUT   (5 * 1000)

class CClientDispatch : public vzbase::noncopyable,
  public vzconn::CClientInterface,
  public vzbase::MessageHandler,
  public boost::enable_shared_from_this<CClientDispatch> {
 public:
  typedef boost::shared_ptr<CClientDispatch> Ptr;

 public:
  CClientDispatch(CClientAccess::Ptr cli, vzbase::Thread *pthread);
  virtual ~CClientDispatch();

  bool Start(int nMain, int nMinor, const std::string &sXmlReq);

  std::string GetChannel() {
    return video_channel_;
  }

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

 public:
  bool OnRespCreateChannel(int nMinor);

 private:
  vzbase::Thread           *thread_;
  CClientAccess::Ptr        access_ptr_;

 private:
  int                       cmd_main_;
  int                       cmd_minor_;

 private:
  std::string               term_id_;
  std::string               chn_ticket_;
  std::string               server_host_;
  unsigned int              server_port_;
  std::string               video_channel_;
  cli::CTcpAsyncClient     *server_connect_;
};

#endif  // LIBCLIENT_HS_CCLIENTDISPATCH_H
