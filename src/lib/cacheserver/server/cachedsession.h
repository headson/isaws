#ifndef FILE_CACHED_CONN_SESSION_H_
#define FILE_CACHED_CONN_SESSION_H_

#include "vzconn/conn/tcpconnect.h"
#include <set>
#include "vzbase/base/loggingsettings.h"
#include "sqlite3/sqlite3.h"
#include "filecached/base/basedefines.h"
#include "filecached/server/cachedservice.h"
#include "filecached/server/cachedstanzapool.h"

namespace cached {

typedef boost::shared_ptr<boost::asio::io_service> io_service_ptr;

class CachedSession : public boost::noncopyable,
  public boost::enable_shared_from_this < CachedSession > {
 public:
  typedef boost::shared_ptr<CachedSession> Ptr;

  // Signal a new tcp connected
  boost::signals2::signal < void(CachedSession::Ptr cached_session,
                                 const boost::system::error_code& err
                                )> SignalSessionError;

  CachedSession(vzconn::VzConnect::Ptr tcp_connect,
                CachedService::Ptr cached_service);
  virtual ~CachedSession();

  bool StartSession();
  void StopSession();
 private:
  // With VzConnect callback function
  void OnConnectError(vzconn::VzConnect::Ptr connect,
                      const boost::system::error_code& err);
  void OnConnectWrite(vzconn::VzConnect::Ptr connect);
  void OnConnectRead(vzconn::VzConnect::Ptr connect,
                     const char* data, int size, int flag);

  //
  bool ReplaceCachedFile(CachedStanza::Ptr 
    );
  bool DeleteCachedFile(CachedStanza::Ptr stanza);
  bool SelectCachedFile(CachedStanza::Ptr stanza);

  void WriteResponse(const char *data, std::size_t data_size);
  void ResponseSucceed();
  void ResponseFailure();
  //
 private:
  bool ProcessStanza(unsigned char type, CachedStanza::Ptr stanza);
  CachedStanza::Ptr Parse(const char* buffer,
                          int size,
                          unsigned char *type);
 private:
  static const int DEFUALT_STANZA_SIZE = 128 * 1024;
  boost::asio::io_service& io_service_;
  vzconn::VzConnect::Ptr tcp_connect_;
  CachedService::Ptr cached_service_;
  std::vector<char> buffer_;
  CachedStanzaPool *cachedstanza_pool_;
};

}

#endif // FILE_CACHED_CONN_SESSION_H_