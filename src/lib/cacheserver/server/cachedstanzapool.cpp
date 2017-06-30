#include "filecached/server/cachedstanzapool.h"

namespace cached {

CachedStanzaPool *CachedStanzaPool::pool_instance_ = NULL;

CachedStanzaPool *CachedStanzaPool::Instance() {
  if(!pool_instance_) {
    pool_instance_ = new CachedStanzaPool();
  }
  return pool_instance_;
}

CachedStanzaPool::CachedStanzaPool() {
}

CachedStanzaPool::~CachedStanzaPool() {
}

CachedStanza::Ptr CachedStanzaPool::TakeStanza(std::size_t mini_size) {
  boost::mutex::scoped_lock stanza_mutex(pool_mutex_);
  if(stanzas_.size()) {
    return TaskPerfectStanza(mini_size);
  }
  //if(CachedStanza::stanza_count <= MAX_CACHED_STANZA_SIZE) {
  //  LOG(INFO) << "stanzas_.size() == 0, Create cached stanza";
  /*return CachedStanza::Ptr(new CachedStanza());*/
  //}
  //LOG(WARNING) << "The stanza size reach limit "
  //             << MAX_CACHED_STANZA_SIZE
  //             << "\t" << stanzas_.size();
  return CachedStanza::Ptr();
}

void CachedStanzaPool::RecyleStanza(CachedStanza *stanza) {
  boost::mutex::scoped_lock stanza_mutex(pool_mutex_);
  LOG(INFO) << "Recyle Stanza " << stanza->size();
  stanza->ResetDefualtState();
  InsertStanza(stanza);
}

void CachedStanzaPool::SetDefaultCachedSize(std::size_t mini_size,
    std::size_t stanza_size) {
  LOG(INFO) << "Set default cached size = " << stanza_size;
  for(std::size_t i = 0; i < stanza_size; i++) {
    LOG(INFO) << "Create cached stanza";
    stanzas_.push_back(new CachedStanza());
  }
}

void CachedStanzaPool::RecyleBuffer(void *stanza) {
  CachedStanzaPool::Instance()->RecyleStanza((CachedStanza *)stanza);
}

CachedStanza::Ptr CachedStanzaPool::TaskPerfectStanza(std::size_t mini_size) {
  BOOST_ASSERT(stanzas_.size() > 0);
  // Small -> Big
  CachedStanza::Ptr perfect_stanza;
  for(Stanzas::iterator iter = stanzas_.begin();
      iter != stanzas_.end(); ++iter) {
    if((*iter)->Capacity() >= mini_size) {
      perfect_stanza.reset(*iter,
                           boost::bind(&CachedStanzaPool::RecyleBuffer,
                                       this, _1));
      stanzas_.erase(iter);
      LOG(INFO) << "Take stanza by mini size = " << mini_size
                << "\t" << stanzas_.size();
      return perfect_stanza;
    }
  }
  LOG(INFO) << "Task stanza by back size = " << stanzas_.size();
  perfect_stanza.reset(stanzas_.front(),
                       boost::bind(&CachedStanzaPool::RecyleBuffer,
                                   this, _1));
  stanzas_.pop_front();
  LOG(INFO) << "Task stanza by back size = " << stanzas_.size();
  return perfect_stanza;
}

void CachedStanzaPool::InsertStanza(CachedStanza *stanza) {
  for(Stanzas::iterator iter = stanzas_.begin();
      iter != stanzas_.end(); ++iter) {
    if(stanza->Capacity() > (*iter)->Capacity()) {
      LOG(INFO) << stanzas_.size();
      stanzas_.insert(iter, stanza);
      return;
    }
  }
  stanzas_.push_back(stanza);
  LOG(INFO) << stanzas_.size();
}


////////////////////////////////////////////////////////////////////////////////

uint32 CachedStanza::stanza_count = 0;

CachedStanza::CachedStanza()
  : is_saved_(false) {
  stanza_count++;
  LOG(WARNING) << "Create stanza " << stanza_count;
}

CachedStanza::~CachedStanza() {
  stanza_count--;
  LOG(WARNING) << "Delete Cache Stanza " << stanza_count;
}

bool CachedStanza::IsSaved() {
  boost::mutex::scoped_lock stanza_mutex(stanza_mutex_);
  return is_saved_;
}

void CachedStanza::SaveConfimation() {
  boost::mutex::scoped_lock stanza_mutex(stanza_mutex_);
  BOOST_ASSERT(!is_saved_);
  is_saved_ = true;
}

void CachedStanza::ResetDefualtState() {
  path_.clear();
  data_.resize(0);
  is_saved_ = false;
}

}