/************************************************************************
* Author      : SoberPeng 2017-10-28
* Description :
************************************************************************/
#ifndef LIBCLIENT_HS_CCLIENTACCESS_H
#define LIBCLIENT_HS_CCLIENTACCESS_H

#include "vzbase/thread/thread.h"
#include "vzbase/base/boost_settings.hpp"
#include "client_hs/client/ctcpasyncclient.h"

#define AES_TIMER_TIMEOUT   (5 * 1000)
typedef enum CLI_CONN_STATE {
  DISCONNECT   = 0,     /*连接断开*/
  CONNECT,              /*连接进行中*/
  CONNECTED,            /*链接成功*/
  HEARTBEAT,            /*发送心跳*/
  HEARTBEATED           /*心跳成功*/
} CLI_CONN_STATE;

class CClientAccess : public vzbase::noncopyable,
  public vzconn::CClientInterface,
  public vzbase::MessageHandler,
  public boost::enable_shared_from_this<CClientAccess> {

 public:
  typedef boost::shared_ptr<CClientAccess> Ptr;

 public:
  CClientAccess(vzbase::Thread *pthread);
  virtual ~CClientAccess();

  bool Start(const std::string &sAesHost, unsigned int nAesPort);

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

 protected:
  void UpdateState(CLI_CONN_STATE state);

  virtual bool OnDisconnect();                          // 建立连接
  virtual bool OnConnected(const std::string &sXml);    // 建立连接处理返回值;发送心跳
  virtual bool OnHeartbeat();                           // 发送心跳
  virtual bool OnHeartbeated(const std::string &sXml);  // 心跳处理

  virtual bool OnCreateChannel(const std::string &sXml);    // 建立通道
  virtual bool OnTransStream(const std::string &sXml);      // 传输流

 private:
  vzbase::Thread           *thread_;

 private:
  std::string               server_host_;
  unsigned int              server_port_;

  CLI_CONN_STATE            connect_state_;     /*链接状态*/
  unsigned int              last_state_time_;   /*上一次状态变化时间*/
  cli::CTcpAsyncClient     *server_connect_;
};

#endif  // LIBCLIENT_HS_CCLIENTACCESS_H
