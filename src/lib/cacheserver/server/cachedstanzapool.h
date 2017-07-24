#ifndef FILECACHED_SERVER_CACHEDSTANZAPOOL_H_
#define FILECACHED_SERVER_CACHEDSTANZAPOOL_H_

#include "vzbase/base/boost_settings.hpp"
#include "vzbase/base/basictypes.h"
#include "vzbase/base/criticalsection.h"
#include <list>
#include <vector>
#include <string>

namespace cached {

class CachedStanza : public vzbase::noncopyable,
  public boost::enable_shared_from_this<CachedStanza> {
 public:
  typedef boost::shared_ptr<CachedStanza> Ptr;

  CachedStanza();
  virtual ~CachedStanza();

  std::string &path() {
    return path_;
  }

  void SetPath(const char *path) {
    path_ = path;
  }

  std::vector<char> &data() {
    return data_;
  }

  void SetData(const uint8 *data, uint32 size);

  bool IsSaved();
  void SaveConfimation();
  void ResetDefualtState();

  std::size_t Capacity() {
    return data_.capacity();
  }
  std::size_t size() {
    return data_.size();
  }
  char *DataString() {
    return (char *)&data_[0];
  }
  static uint32 stanza_count;
 private:
  std::string path_;
  std::vector<char> data_;
  bool is_saved_;

  vzbase::CriticalSection stanza_mutex_;
};

class CachedStanzaPool : public vzbase::noncopyable,
  public boost::enable_shared_from_this<CachedStanzaPool> {
 public:
  typedef boost::shared_ptr<CachedStanzaPool> Ptr;
  CachedStanzaPool();
  virtual ~CachedStanzaPool();
  static CachedStanzaPool *Instance();
  // Thread safed
  CachedStanza::Ptr TakeStanza(std::size_t mini_size);
  // Thread safed
  void SetDefaultCachedSize(std::size_t mini_size, std::size_t stanza_size);
  std::size_t CachedStanzaSize() {
    return stanzas_.size();
  }
 private:
  void InsertStanza(CachedStanza *stanza);
  CachedStanza::Ptr TaskPerfectStanza(std::size_t mini_size);
  static void RecyleBuffer(void *stanza);
  void RecyleStanza(CachedStanza *stanza);
 private:
  static const int MAX_CACHED_STANZA_SIZE = 128;
  typedef std::list<CachedStanza *> Stanzas;
  Stanzas stanzas_;
  vzbase::CriticalSection pool_mutex_;
 private:
  static CachedStanzaPool *pool_instance_;
};
}

#endif // FILECACHED_SERVER_CACHEDSTANZAPOOL_H_
