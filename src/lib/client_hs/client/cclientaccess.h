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
  DISCONNECT   = 0,     /*���ӶϿ�*/
  CONNECT,              /*���ӽ�����*/
  CONNECTED,            /*���ӳɹ�*/
  HEARTBEAT,            /*��������*/
  HEARTBEATED           /*�����ɹ�*/
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

  virtual bool OnDisconnect();                          // ��������
  virtual bool OnConnected(const std::string &sXml);    // �������Ӵ�����ֵ;��������
  virtual bool OnHeartbeat();                           // ��������
  virtual bool OnHeartbeated(const std::string &sXml);  // ��������

  virtual bool OnCreateChannel(const std::string &sXml);    // ����ͨ��
  virtual bool OnTransStream(const std::string &sXml);      // ������

 private:
  vzbase::Thread           *thread_;

 private:
  std::string               server_host_;
  unsigned int              server_port_;

  CLI_CONN_STATE            connect_state_;     /*����״̬*/
  unsigned int              last_state_time_;   /*��һ��״̬�仯ʱ��*/
  cli::CTcpAsyncClient     *server_connect_;
};

#endif  // LIBCLIENT_HS_CCLIENTACCESS_H
