#include "cacheserver/server/cachedserver.h"
#include "vzlogging/logging/vzloggingcpp.h"
#include <vector>


namespace cached {

CachedServer::CachedServer(vzconn::EventService &event_service)
  : event_service_(event_service),
    tcp_server_(NULL),
    cache_service_(NULL) {
}

CachedServer::~CachedServer() {
}


bool CachedServer::StartCachedServer(
  const char *listen_addr,
  unsigned short listen_port) {
  if (tcp_server_ != NULL || cache_service_ != NULL) {
    LOG(L_ERROR) << "Failure to restart kvdb server";
    return false;
  }
  vzconn::CEvtTcpServer *tcp_server_ = vzconn::CEvtTcpServer::Create(&event_service_,
                                       this,
                                       this);

  int32 n_ret = 0;
  vzconn::CInetAddr c_addr(listen_addr, listen_port);
  n_ret = tcp_server_->Open(&c_addr, false, true);

  cache_service_ = new CachedService(event_service_);

  return true;
}

bool CachedServer::StopCachedServer() {
  return true;
}

// 服务端回调函数
bool CachedServer::HandleNewConnection(vzconn::VSocket *p_srv,
                                      vzconn::VSocket *new_sock) {
  return true;
}

void CachedServer::HandleServerClose(vzconn::VSocket *p_srv) {

}

// 客户端回调函数
int32 CachedServer::HandleRecvPacket(vzconn::VSocket *p_cli,
                                    const uint8   *p_data,
                                    uint32         n_data,
                                    uint16         n_flag) {

  const CacheMessage *cache_msg = (const CacheMessage *)(p_data);
  memcpy(&kvdb_message_, cache_msg, sizeof(CacheMessage));

  if (cache_msg->type == CACHED_SELECT
      && ProcessSelect(cache_msg,
                       p_data + sizeof(CacheMessage),
                       n_data - sizeof(CacheMessage),
                       p_cli)) {
    LOG(L_INFO) << "Process with kvdb select operator";
  }
  if (cache_msg->type == CACHED_SELECT) {
    return 0;
  }

  if (ProcessKvdbService(cache_msg,
                         p_data + sizeof(CacheMessage),
                         n_data - sizeof(CacheMessage))) {
    ResponseKvdb(p_cli, CACHED_SUCCEED);

  } else {
    ResponseKvdb(p_cli, CACHED_FAILURE);
  }
  return 0;
}


bool CachedServer::ProcessKvdbService(const CacheMessage *cache_msg,
                                     const uint8 *data,
                                     uint32 size) {
  if (cache_msg->type == CACHED_REPLACE) {
    return cache_service_->AddFile(cache_msg->path, data, size);
  } else if (cache_msg->type == CACHED_DELETE) {
    return cache_service_->RemoveFile(cache_msg->path);
  }
  return false;
}

bool CachedServer::ResponseKvdb(vzconn::VSocket *p_cli, uint32 type) {
  kvdb_message_.type = type;
  p_cli->AsyncWrite(&kvdb_message_, sizeof(CacheMessage), 0);
  return true;
}

bool CachedServer::ProcessSelect(const CacheMessage *cache_msg,
                                const uint8 *data,
                                uint32 size,
                                vzconn::VSocket *p_cli) {
  static std::vector<char> buffer;
  if (cache_msg->type != CACHED_SELECT) {
    return false;
  }
  if (cache_service_->GetFile(cache_msg->path, buffer)) {
    kvdb_message_.type = CACHED_SUCCEED;

    iovec ivcec[2];

    ivcec[0].iov_base = (void *)&kvdb_message_;
    ivcec[0].iov_len  = sizeof(CacheMessage);

    ivcec[1].iov_base = (void *)&(buffer[0]);
    ivcec[1].iov_len  = buffer.size();

    p_cli->AsyncWrite(ivcec, 2, 0);

    return true;
  }
  ResponseKvdb(p_cli, CACHED_FAILURE);
  return false;
}
int32 CachedServer::HandleSendPacket(vzconn::VSocket *p_cli) {
  return 0;
}

void  CachedServer::HandleClose(vzconn::VSocket *p_cli) {
}
}