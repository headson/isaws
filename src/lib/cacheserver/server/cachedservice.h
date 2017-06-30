#ifndef FILE_CACHED_CONN_CACHED_SERVICES_H_
#define FILE_CACHED_CONN_CACHED_SERVICES_H_

#include <set>
#include <deque>
#include <vector>
#include "vzbase/base/noncoypable.h"
#include "vzconn/async/cevtipcserver.h"

namespace cached {

#define MAX_ROOT_PREFIX_SIZE 64

struct FlashCachedSettings {
  uint32_t max_image_size;
  char     root_prefix_[MAX_ROOT_PREFIX_SIZE];
};

class CachedService : public vzbase::noncopyable {
 public:
  CachedService(vzconn::EventService &event_service);
  virtual ~CachedService();
  bool Start();
  bool AddFile(const char *path,
               const uint8 *data, uint32 size,
               bool is_cached = true);
  bool RemoveFile(const char *path);
  bool GetFile(const char *path, std::vector<char> &buffer);
//  void RemoveOutOfDataStanza();
 private:
//  void OnCachedThread();
  void AsyncSaveFile(const char *path,
                     const uint8 *data,
                     uint32 size);
  void AsyncRemoveFile(const char *file_path);
  bool ReadFile(const char *path, std::vector<char> &data_buffer);
//  void ReplaceCachedFile(CachedStanza::Ptr stanza);
//
#ifndef WIN32
  void iMakeDirRecursive(const char *pPath);
#else
#endif
//  bool ReadFile(const std::string path, std::vector<char> &data_buffer);
 private:
  static const int READ_FILE_BUFFER_SIZE  = 4096;
  static const int READ_MAX_BUFFER        = 1024 * 128;
  static const int MAX_CACHED_STANZA_SIZE = 32;
  std::size_t cache_size_;
  std::size_t current_cache_size_;
  //boost::asio::io_service& io_service_;
  //boost::asio::io_service cached_service_;
  //boost::scoped_ptr<boost::asio::io_service::work> cached_service_work_;
  //boost::scoped_ptr<boost::thread> cache_thread_;
  //std::deque<CachedStanza::Ptr> cached_stanzas_;
  //CachedStanzaPool *cachedstanza_pool_;
  FlashCachedSettings *flash_cached_settings_;
  vzconn::EventService  &event_service_;
};

}

#endif // FILE_CACHED_CONN_CACHED_SERVICES_H_