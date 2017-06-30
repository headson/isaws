#include "filecached/server/cachedsession.h"

namespace cached {

const char FILE_CACHED_RESULT_SUCCEED_BUFFER[1] = {FILE_CACHED_RESULT_SUCCEED};
const char FILE_CACHED_RESULT_FAILURE_BUFFER[1] = {FILE_CACHED_RESULT_FAILURE};

CachedSession::CachedSession(vzconn::VzConnect::Ptr tcp_connect,
                             CachedService::Ptr cached_service)
  : tcp_connect_(tcp_connect),
    io_service_(tcp_connect->io_service()),
    cached_service_(cached_service) {
  cachedstanza_pool_ = CachedStanzaPool::Instance();
  BOOST_ASSERT(cachedstanza_pool_ != NULL);
}

CachedSession::~CachedSession() {
  LOG(WARNING) << "Delete CachedSession";
}

bool CachedSession::StartSession() {
  BOOST_ASSERT(tcp_connect_);
  tcp_connect_->SignalConnectError.connect(
    boost::bind(&CachedSession::OnConnectError, shared_from_this(), _1, _2));
  tcp_connect_->SignalConnectWrite.connect(
    boost::bind(&CachedSession::OnConnectWrite, shared_from_this(), _1));
  tcp_connect_->SignalConnectRead.connect(
    boost::bind(&CachedSession::OnConnectRead, shared_from_this(), _1, _2, _3, _4));
  return true;
}

void CachedSession::StopSession() {
}

void CachedSession::OnConnectError(vzconn::VzConnect::Ptr connect,
                                   const boost::system::error_code& err) {
  tcp_connect_.reset();
  SignalSessionError(shared_from_this(), err);
}

void CachedSession::OnConnectWrite(vzconn::VzConnect::Ptr connect) {
}

void CachedSession::OnConnectRead(vzconn::VzConnect::Ptr connect,
                                  const char* data, int size, int flag) {
  unsigned char type = 0;
  //CachedStanza::Ptr stanza = cachedstanza_pool_->TakeStanza();
  //stanza->is_saved = false;
  cached_service_->RemoveOutOfDataStanza();
  CachedStanza::Ptr stanza = Parse(data, size, &type);
  if(!stanza) {
    ResponseFailure();
    return;
  }
  if(!ProcessStanza(type, stanza)) {
    LOG(ERROR) << "Failure to Process Stanza";
  }
}

CachedStanza::Ptr CachedSession::Parse(const char* buffer,
                                       int size,
                                       unsigned char *type) {
  int offset = 0;
  // Read the type
  if(offset + 1 >= size) {
    LOG(ERROR) << "The size of recv error";
    return CachedStanza::Ptr();
  }
  *type = buffer[offset];
  offset++;

  // Read the path size
  int path_size = 0;
  if(*type == FILE_CACHED_NONE || offset + 1 >= size) {
    LOG(ERROR) << "The size of recv error";
    return CachedStanza::Ptr();
  }
  path_size = buffer[offset];
  if(path_size <= 0) {
    LOG(ERROR) << "Read the size of path error";
    return CachedStanza::Ptr();
  }
  offset++;

  // Read the data size
  uint32 data_size = 0;
  if(offset + 4 >= size) {
    LOG(ERROR) << "The size of recv error";
    return CachedStanza::Ptr();
  }
  memcpy(&data_size, buffer + offset, 4);
  data_size = ntohl(data_size);
  offset += 4;

  //////////////////////////////////////////////////////////////////////////////
  BOOST_ASSERT(cachedstanza_pool_ != NULL);
  uint32 stanza_size = DEFUALT_STANZA_SIZE;
  if(data_size > stanza_size) {
    stanza_size = data_size;
  }
  CachedStanza::Ptr stanza = cachedstanza_pool_->TakeStanza(stanza_size);
  if(!stanza) {
    return stanza;
  }
  BOOST_ASSERT(!stanza->IsSaved());
  //////////////////////////////////////////////////////////////////////////////

  // Read the path
  if(offset + path_size > size) {
    LOG(ERROR) << "The size of recv error";
    return CachedStanza::Ptr();
  }
  stanza->path().append(buffer + offset, path_size);
  offset += path_size;

  // Read the data
  if(offset + data_size != size) {
    LOG(ERROR) << "The size of recv error";
    return CachedStanza::Ptr();
  }
  if(data_size != 0) {
    std::vector<char> &data_buffer = stanza->data();
    data_buffer.resize(data_size);
    memcpy(&data_buffer[0], buffer + offset, data_size);
    offset += data_size;
  }
  if(offset != size) {
    LOG(ERROR) << "The size is big than data";
    return CachedStanza::Ptr();
  }
  return stanza;
}

bool CachedSession::ProcessStanza(unsigned char type,
                                  CachedStanza::Ptr stanza) {
  switch (type) {
  case cached::FILE_CACHED_REPLACE:
    // LOG(INFO) << "cached::FILE_CACHED_REPLACE";
    return ReplaceCachedFile(stanza);
    break;
  case cached::FILE_CACHED_DELETE:
    // LOG(INFO) << "cached::FILE_CACHED_DELETE";
    return DeleteCachedFile(stanza);
    break;
  case cached::FILE_CACHED_SEARCH:
    // LOG(INFO) << "cached::FILE_CACHED_SEARCH";
    return SelectCachedFile(stanza);
    break;
  }
  // LOG(INFO) << "Ending";
  return false;
}


bool CachedSession::ReplaceCachedFile(CachedStanza::Ptr stanza) {
  bool res = cached_service_->AddFile(stanza);
  if(res) {
    ResponseSucceed();
  } else {
    ResponseFailure();
  }
  return res;
}

bool CachedSession::DeleteCachedFile(CachedStanza::Ptr stanza) {
  bool res = cached_service_->RemoveFile(stanza->path());
  if(res) {
    ResponseSucceed();
  } else {
    ResponseFailure();
  }
  return res;
}

bool CachedSession::SelectCachedFile(CachedStanza::Ptr stanza) {
  CachedStanza::Ptr res_stanza = cached_service_->GetFile(stanza);
  if(res_stanza) {
    buffer_.resize(res_stanza->size() + 5);
    buffer_[0] = FILE_CACHED_RESULT_SUCCEED;
    uint32 size = res_stanza->size();
    size = htonl(size);

    memcpy((void *)&buffer_[1], &size, 4);
    memcpy((void *)&buffer_[5],
           (const void *)&(res_stanza->data()[0]),
           res_stanza->size());
    WriteResponse((const char *)&buffer_[0], buffer_.size());
  } else {
    ResponseFailure();
  }
  return true;
}

void CachedSession::ResponseSucceed() {
  WriteResponse(FILE_CACHED_RESULT_SUCCEED_BUFFER, 1);
}

void CachedSession::ResponseFailure() {
  WriteResponse(FILE_CACHED_RESULT_FAILURE_BUFFER, 1);
}

void CachedSession::WriteResponse(const char *data, std::size_t data_size) {
  if(tcp_connect_->IsConnected()) {
    tcp_connect_->AsyncWrite(data, data_size);
  } else {
    LOG(ERROR) << "Tcp disconnected";
  }
}

}