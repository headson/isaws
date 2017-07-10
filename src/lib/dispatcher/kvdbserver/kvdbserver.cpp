#include "dispatcher/kvdbserver/kvdbserver.h"
#include "vzlogging/logging/vzloggingcpp.h"

namespace kvdb {


KvdbServer::KvdbServer(vzconn::EventService &event_service)
  : event_service_(event_service),
    tcp_server_(NULL),
    kvdb_sqlite_(NULL) {
}

KvdbServer::~KvdbServer() {
}


bool KvdbServer::StartKvdbServer(
  const char *listen_addr,
  unsigned short listen_port,
  const char *kvdb_path,
  const char *backup_path) {
  if (tcp_server_ != NULL || kvdb_sqlite_ != NULL) {
    LOG(L_ERROR) << "Failure to restart kvdb server";
    return false;
  }
  vzconn::CEvtTcpServer *tcp_server_ = vzconn::CEvtTcpServer::Create(&event_service_,
                                       this,
                                       this);

  int32 n_ret = 0;
  vzconn::CInetAddr c_addr(listen_addr, listen_port);
  n_ret = tcp_server_->Open(&c_addr, false, true);

  kvdb_sqlite_ = new KvdbSqlite();
  if(!kvdb_sqlite_->InitKvdb(kvdb_path, backup_path)) {
    LOG(L_ERROR) << "open kvdb error";
    kvdb_sqlite_->RemoveDatabase(kvdb_path);
    if(!kvdb_sqlite_->InitKvdb(kvdb_path, backup_path)) {
      LOG(L_ERROR) << "reopen the database error";
      return false;
    }
  }
  return true;
}

bool KvdbServer::StopKvdbServer() {
  return true;
}

// 服务端回调函数
bool KvdbServer::HandleNewConnection(vzconn::VSocket *p_srv,
                                     vzconn::VSocket *new_sock) {
  return true;
}

void KvdbServer::HandleServerClose(vzconn::VSocket *p_srv) {

}

// 客户端回调函数
int32 KvdbServer::HandleRecvPacket(vzconn::VSocket *p_cli,
                                   const char   *p_data,
                                   unsigned int         n_data,
                                   unsigned short         n_flag) {

  const KvdbMessage *kvdb_msg = (const KvdbMessage *)(p_data);
  memcpy(&kvdb_message_, kvdb_msg, sizeof(KvdbMessage));

  if (kvdb_msg->type == KVDB_SELECT
      && ProcessSelect(kvdb_msg,
                       p_data + sizeof(KvdbMessage),
                       n_data - sizeof(KvdbMessage),
                       p_cli)) {
    LOG(L_INFO) << "Process with kvdb select operator";
  }
  if (kvdb_msg->type == KVDB_SELECT) {
    return 0;
  }

  if (ProcessKvdbService(kvdb_msg,
                         p_data + sizeof(KvdbMessage),
                         n_data - sizeof(KvdbMessage))) {
    ResponseKvdb(p_cli, KVDB_SUCCEED);

  } else {
    ResponseKvdb(p_cli, KVDB_FAILURE);
  }
  return 0;
}


bool KvdbServer::ProcessKvdbService(const KvdbMessage *kvdb_msg,
                                    const char *data,
                                    unsigned int size) {
  if (kvdb_msg->type == KVDB_REPLACE) {
    return kvdb_sqlite_->ReplaceKeyValue(kvdb_msg->key,
                                         strnlen(kvdb_msg->key, MAX_KVDB_KEY_SIZE),
                                         (const char *)data, size);
  } else if (kvdb_msg->type == KVDB_DELETE) {
    return kvdb_sqlite_->DeleteKeyValue(kvdb_msg->key,
                                        strnlen(kvdb_msg->key, MAX_KVDB_KEY_SIZE));
  } else if (kvdb_msg->type == KVDB_RESTORE) {
    return kvdb_sqlite_->RestoreDatabase();
  } else if (kvdb_msg->type == KVDB_BACKUP) {
    return kvdb_sqlite_->BackupDatabase();
  }
  return false;
}

bool KvdbServer::ResponseKvdb(vzconn::VSocket *p_cli, unsigned int type) {
  kvdb_message_.type = type;
  p_cli->AsyncWrite(&kvdb_message_, sizeof(KvdbMessage), 0);
  return true;
}

bool KvdbServer::ProcessSelect(const KvdbMessage *kvdb_msg,
                               const char *data,
                               unsigned int size,
                               vzconn::VSocket *p_cli) {
  static std::vector<char> buffer;
  if (kvdb_msg->type != KVDB_SELECT) {
    return false;
  }
  if (kvdb_sqlite_->SelectKeyValue(kvdb_msg->key,
                                   strnlen(kvdb_msg->key, MAX_KVDB_KEY_SIZE),
                                   buffer)) {
    kvdb_message_.type = KVDB_SUCCEED;

    iovec ivcec[2];

    ivcec[0].iov_base = (void *)&kvdb_message_;
    ivcec[0].iov_len  = sizeof(KvdbMessage);

    ivcec[1].iov_base = (void *)&(buffer[0]);
    ivcec[1].iov_len  = buffer.size();

    p_cli->AsyncWrite(ivcec, 2, 0);

    return true;
  }
  ResponseKvdb(p_cli, KVDB_FAILURE);
  return false;
}
int32 KvdbServer::HandleSendPacket(vzconn::VSocket *p_cli) {
  return 0;
}

void  KvdbServer::HandleClose(vzconn::VSocket *p_cli) {
}

}
