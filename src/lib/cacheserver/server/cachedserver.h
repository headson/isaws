#ifndef FILE_CACHED_SERVER_CONNSERVER_H_
#define FILE_CACHED_SERVER_CONNSERVER_H_

#include "cacheserver/base/basedefines.h"
#include "vzbase/base/basictypes.h"
#include "vzbase/base/noncoypable.h"
#include "cacheserver/server/cachedservice.h"
#include <string>

namespace cached {


class CachedServer : public vzbase::noncopyable,
  public vzconn::CTcpServerInterface,
  public vzconn::CClientInterface {
 public:
  CachedServer(vzconn::EventService &event_service);
  virtual ~CachedServer();

  bool StartCachedServer(
    const char *listen_addr,
    unsigned short listen_port);
  bool StopCachedServer();
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
  bool ProcessKvdbService(const CacheMessage *cache_msg,
                          const uint8 *data,
                          uint32 size);
  bool ProcessSelect(const CacheMessage *cache_msg,
                     const uint8 *data,
                     uint32 size,
                     vzconn::VSocket *p_cli);
  bool ResponseKvdb(vzconn::VSocket *p_cli, uint32 type);
 private:
  vzconn::EventService  &event_service_;
  vzconn::CEvtTcpServer *tcp_server_;
  CachedService          *cache_service_;
  CacheMessage           kvdb_message_;
};

}

#endif // FILE_CACHED_SERVER_CONNSERVER_H_