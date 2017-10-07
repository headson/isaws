#ifndef KVDB_KBDBSERVER_H_
#define KVDB_KBDBSERVER_H_

#include "vzbase/thread/thread.h"
#include "vzbase/base/boost_settings.hpp"
#include "vzconn/async/cevtipcserver.h"
#include "dispatcher/kvdbserver/kvdbsqlite.h"
#include "dispatcher/base/pkghead.h"

namespace kvdb {


class KvdbServer : public boost::noncopyable,
  public vzconn::CTcpServerInterface,
  public vzconn::CClientInterface,
  public vzbase::MessageHandler {

 public:
  KvdbServer(vzbase::Thread *main_thread);
  virtual ~KvdbServer();

  bool StartKvdbServer(const char *listen_addr,
                       unsigned short listen_port,
                       const char *kvdb_path,
                       const char *backup_path);
  bool StopKvdbServer();
 private:
  // 服务端回调函数
  virtual bool HandleNewConnection(vzconn::VSocket *p_srv,
                                   vzconn::VSocket *new_sock);
  virtual void HandleServerClose(vzconn::VSocket *p_srv);

  // 客户端回调函数
  virtual int32 HandleRecvPacket(vzconn::VSocket *p_cli,
                                 const uint8   *p_data,
                                 uint32         n_data,
                                 uint16         n_flag);
  virtual int32 HandleSendPacket(vzconn::VSocket *p_cli);
  virtual void  HandleClose(vzconn::VSocket *p_cli);

 private:
  bool ProcessKvdbService(const KvdbMessage *kvdb_msg,
                          const uint8 *data,
                          uint32 size);
  bool ProcessSelect(const KvdbMessage *kvdb_msg,
                     const uint8 *data,
                     uint32 size,
                     vzconn::VSocket *p_cli);
  bool ResponseKvdb(vzconn::VSocket *p_cli, uint32 type);

 protected:
  void OnMessage(vzbase::Message* msg);

 private:
  vzbase::Thread        *main_thread_;
  vzconn::CEvtTcpServer *tcp_server_;

  KvdbSqlite            *kvdb_sqlite_;
  KvdbMessage            kvdb_message_;
};

}

#endif // KVDB_KBDBSERVER_H_
