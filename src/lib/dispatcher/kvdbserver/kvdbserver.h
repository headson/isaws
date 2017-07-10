#ifndef KVDB_KBDBSERVER_H_
#define KVDB_KBDBSERVER_H_

#include "vzbase/base/noncoypable.h"
#include "vzconn/async/cevtipcserver.h"
#include "dispatcher/kvdbserver/kvdbsqlite.h"
#include "dispatcher/base/pkghead.h"

namespace kvdb {


class KvdbServer : public vzbase::noncopyable,
  public vzconn::CTcpServerInterface,
  public vzconn::CClientInterface {
 public:
  KvdbServer(vzconn::EventService &event_service);
  virtual ~KvdbServer();

  bool StartKvdbServer(
    const char *listen_addr,
    unsigned short listen_port,
    const char *kvdb_path,
    const char *backup_path);
  bool StopKvdbServer();
 private:
  // ����˻ص�����
  virtual bool HandleNewConnection(vzconn::VSocket *p_srv,
                                   vzconn::VSocket *new_sock);
  virtual void HandleServerClose(vzconn::VSocket *p_srv);
  // �ͻ��˻ص�����
  virtual int32 HandleRecvPacket(vzconn::VSocket *p_cli,
                                 const char   *p_data,
                                 unsigned int         n_data,
                                 unsigned short         n_flag);
  virtual int32 HandleSendPacket(vzconn::VSocket *p_cli);
  virtual void  HandleClose(vzconn::VSocket *p_cli);
 private:
  bool ProcessKvdbService(const KvdbMessage *kvdb_msg,
                          const char *data,
                          unsigned int size);
  bool ProcessSelect(const KvdbMessage *kvdb_msg,
                     const char *data,
                     unsigned int size,
                     vzconn::VSocket *p_cli);
  bool ResponseKvdb(vzconn::VSocket *p_cli, unsigned int type);
 private:
  vzconn::EventService  &event_service_;
  vzconn::CEvtTcpServer *tcp_server_;
  KvdbSqlite            *kvdb_sqlite_;
  KvdbMessage           kvdb_message_;
};

}

#endif // KVDB_KBDBSERVER_H_
