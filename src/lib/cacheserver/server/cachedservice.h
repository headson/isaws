#ifndef FILE_CACHED_CONN_CACHED_SERVICES_H_
#define FILE_CACHED_CONN_CACHED_SERVICES_H_

#include <set>
#include <deque>
#include <vector>
#include "vzbase/base/noncoypable.h"
#include "vzconn/async/cevtipcserver.h"
#include "vzbase/thread/thread.h"
#include "cacheserver/server/cachedstanzapool.h"

namespace cached {

#define MAX_ROOT_PREFIX_SIZE 64

struct FlashCachedSettings {
  uint32_t max_image_size;
  char     root_prefix_[MAX_ROOT_PREFIX_SIZE];
};

struct FlcStanza {
  uint32 max_size;
  std::string path;
  FlcStanza &operator=(const FlcStanza &stanza) {
    max_size = stanza.max_size;
    path     = stanza.path;
    return *this;
  }
};

enum CachedMessage {
  CACHED_ADD,
  CACHED_CHECK
};

class CachedService : public vzbase::noncopyable,
  public vzbase::MessageHandler {
 public:

  CachedService(vzbase::Thread *cached_thread,
                std::size_t cache_size);
  virtual ~CachedService();
  bool Start();
  bool AddFile(CachedStanza::Ptr stanza, bool is_cached = true);
  bool RemoveFile(const std::string path);
  CachedStanza::Ptr GetFile(const char *path);
  void RemoveOutOfDataStanza();

  virtual void OnMessage(vzbase::Message *msg);
 private:
  void CheckFileLimit();
  bool InitFileLimitCheck();
  void WattingNextTransactionAsyncEvent();
 private:
  // void OnCachedThread();
  int  OnAsyncSaveFile(CachedStanza::Ptr stanza);
  void OnAsyncRemoveFile(std::string path);
  void ReplaceCachedFile(CachedStanza::Ptr stanza);

#ifndef WIN32
  void iMakeDirRecursive(const char *pPath);
#else
#endif
  bool ReadFile(const std::string path, std::vector<char> &data_buffer);
 private:
  struct StanzaMessageData : public vzbase::MessageData {
    CachedStanza::Ptr stanza;
  };
 private:
  static const int READ_FILE_BUFFER_SIZE  = 4096;
  static const int READ_MAX_BUFFER        = 1024 * 128;
  static const int MAX_CACHED_STANZA_SIZE = 32;
  static const int ASYNC_TIMEOUT_TIMES    = 10;
  std::size_t cache_size_;
  std::size_t current_cache_size_;
  vzbase::Thread *cached_thread_;
  std::deque<CachedStanza::Ptr> cached_stanzas_;
  CachedStanzaPool *cachedstanza_pool_;
  FlashCachedSettings *flash_cached_settings_;


  std::vector<FlcStanza> flc_stanzas_;
  uint32                 current_stanzas_index_;
};
}

#endif // FILE_CACHED_CONN_CACHED_SERVICES_H_