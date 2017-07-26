#include "cacheserver/server/cachedserver.h"
#include "vzlogging/logging/vzloggingcpp.h"
#include <vector>


namespace cached {

CachedServer::CachedServer(vzconn::EventService &event_service)
  : vzconn::CTcpServerInterface()
  , vzconn::CClientInterface()
  , event_service_(event_service)
  , tcp_server_(NULL)
  , cache_service_(NULL)
  , cached_thread_(NULL)
  , cached_message_() {
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

  cached_thread_ = new vzbase::Thread();
  cached_thread_->Start();

  vzconn::CEvtTcpServer *tcp_server_ = vzconn::CEvtTcpServer::Create(
                                         &event_service_,
                                         this,
                                         this);

  int32 n_ret = 0;
  vzconn::CInetAddr c_addr(listen_addr, listen_port);
  n_ret = tcp_server_->Open(&c_addr, false, true);


  cache_service_ = new CachedService(cached_thread_, 12);
  cache_service_->Start();

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
  memcpy(&cached_message_, cache_msg, sizeof(CacheMessage));

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

  if (ProcessCacheService(cache_msg,
                          p_data + sizeof(CacheMessage),
                          n_data - sizeof(CacheMessage))) {
    ResponseKvdb(p_cli, CACHED_SUCCEED);

  } else {
    ResponseKvdb(p_cli, CACHED_FAILURE);
  }
  return 0;
}


bool CachedServer::ProcessCacheService(const CacheMessage *cache_msg,
                                       const uint8 *data,
                                       uint32 size) {
  if (cache_msg->type == CACHED_REPLACE) {
    CachedStanza::Ptr stanza = CachedStanzaPool::Instance()->TakeStanza(size);
    if (stanza) {
      stanza->SetPath(cache_msg->path);
      stanza->SetData(data, size);
      return cache_service_->AddFile(stanza, true);
    } else {
      return false;
    }
  } else if (cache_msg->type == CACHED_DELETE) {
    return cache_service_->RemoveFile(cache_msg->path);
  }
  return false;
}

bool CachedServer::ResponseKvdb(vzconn::VSocket *p_cli, uint32 type) {
  cached_message_.type = type;
  p_cli->AsyncWrite(&cached_message_, sizeof(CacheMessage), 0);
  return true;
}

bool CachedServer::ProcessSelect(const CacheMessage *cache_msg,
                                 const uint8 *data,
                                 uint32 size,
                                 vzconn::VSocket *p_cli) {
  if (cache_msg->type != CACHED_SELECT) {
    return false;
  }
  CachedStanza::Ptr stanza = cache_service_->GetFile(cache_msg->path);

  if (stanza) {
    cached_message_.type = CACHED_SUCCEED;

    iovec ivcec[2];

    ivcec[0].iov_base = (void *)&cached_message_;
    ivcec[0].iov_len  = sizeof(CacheMessage);

    ivcec[1].iov_base = (void *)&(stanza->data()[0]);
    ivcec[1].iov_len  = stanza->data().size();
    
    //std::size_t n_pos = stanza->path().find_last_of("/");
    //if (n_pos != stanza->path().npos) {
    //  std::string s_path = "/media/" + stanza->path().substr(n_pos + 1, stanza->path().npos);

    //  LOG(L_WARNING) << "save tmp path " << s_path.c_str();
    //  FILE *file = fopen(s_path.c_str(), "wb+");
    //  if (file) {
    //    fwrite(&stanza->data()[0], 1, stanza->data().size(), file);
    //    fclose(file);
    //  }
    //}
    LOG(L_WARNING) << "save tmp path " << stanza->path();
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