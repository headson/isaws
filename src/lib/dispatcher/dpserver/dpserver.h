#ifndef LIBDISPATCHER_SERVER_SERVER_H_
#define LIBDISPATCHER_SERVER_SERVER_H_

#include "vzconn/async/cevttcpserver.h"
#include "dispatcher/dpserver/dpsession.h"
#include "vzbase/base/noncoypable.h"

namespace dp {

#define MAX_SESSION_SIZE 128

class DpServer : public vzbase::noncopyable,
  public vzconn::CTcpServerInterface,
  public vzconn::CClientInterface,
  public SessionInterface {
 public:
  DpServer(vzconn::EventService &event_service);
  virtual ~DpServer();
  bool StartDpServer(
    const char *listen_addr,
    unsigned short listen_port);
  bool StopDpServer();
 private:
  // 服务端回调函数
  virtual bool  HandleNewConnection(vzconn::VSocket *p_srv,
                                   vzconn::VSocket *new_sock);
  virtual void  HandleServerClose(vzconn::VSocket *p_srv);
  // 客户端回调函数
  virtual int32 HandleRecvPacket(vzconn::VSocket *p_cli,
                                 const char      *p_data,
                                 unsigned int     n_data,
                                 unsigned short   n_flag);
  virtual int32 HandleSendPacket(vzconn::VSocket *p_cli);
  virtual void  HandleClose(vzconn::VSocket *p_cli);
  // Session回调
  virtual bool AsyncWrite(Session *session,
                          vzconn::VSocket *vz_socket,
                          const DpMessage *dmp,
                          const char *data, 
                          int size);
  virtual void OnSessionError(Session *session, vzconn::VSocket *vz_socket);

 private:
  unsigned char GetNewSessionId();
  bool AddSession(unsigned char n_idx, Session *session, vzconn::VSocket *socket);
  bool RemoveBySession(Session *session);
  bool RemoveBySocket(vzconn::VSocket *socket);
  Session *FindSessionBySocket(vzconn::VSocket *socket);
  vzconn::VSocket *FindSocketBySession(Session *session);

 private:
  unsigned int DispatcherSession(Session *session,
                           const DpMessage *dmsg,
                           const char *data,
                           unsigned int data_size);
 private:
  struct SessionSocketPair {
    Session *session;
    vzconn::VSocket *socket;
  };
 private:
  vzconn::CEvtTcpServer *tcp_server_;
  SessionSocketPair      session_socket_map_[MAX_SESSION_SIZE];
  vzconn::EventService  &event_service_;
};

};

#endif // LIBDISPATCHER_SERVER_SERVER_H_
