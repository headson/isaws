#include "cacheserver/server/cachedservice.h"
#include "vzlogging/logging/vzloggingcpp.h"

#ifndef WIN32

#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#else
#endif

namespace cached {

#define SD_CAP_PATH    "/media/mmcblk0p1/VzIPCCap"

CachedService::CachedService(vzbase::Thread *cached_thread,
                             std::size_t cache_size)
  : cache_size_(cache_size),
    cached_thread_(cached_thread),
    current_cache_size_(0) {
  cachedstanza_pool_ = CachedStanzaPool::Instance();
  cachedstanza_pool_->SetDefaultCachedSize(32, cache_size + 6);
  BOOST_ASSERT(cachedstanza_pool_ != NULL);
}

CachedService::~CachedService() {
}

bool CachedService::Start() {
  //cached_service_work_.reset(
  //  new boost::asio::io_service::work(cached_service_));
  //cache_thread_.reset(
  //  new boost::thread(boost::bind(&CachedService::OnCachedThread, this)));
  return true;
}

//void CachedService::OnCachedThread() {
//  //while(true) {
//  //  try {
//  //    cached_service_.run();
//  //  } catch(std::exception &e) {
//  //    LOG(ERROR) << e.what();
//  //  }
//  //}
//}

void CachedService::OnMessage(vzbase::Message *msg) {
  StanzaMessageData *stanza_msg = static_cast<StanzaMessageData *>(msg->pdata.get());
  if (stanza_msg) {
    OnAsyncSaveFile(stanza_msg->stanza);
  }
}

bool CachedService::AddFile(CachedStanza::Ptr stanza, bool is_cached) {
  LOG(INFO) << "Cached file = " << stanza->path()
            <<" is cached "<< is_cached
            <<" stanza use count "<<stanza.use_count();
  ReplaceCachedFile(stanza);
  // Try to remove out of data stanza
  RemoveOutOfDataStanza();
  if(cached_stanzas_.size() > MAX_CACHED_STANZA_SIZE) {
    stanza->SaveConfimation();
    LOG(WARNING) << "cached_stanzas size big than 64 " << cached_stanzas_.size();
    return true;
  }
  if(is_cached) {
    StanzaMessageData *stanza_msg = new StanzaMessageData();
    stanza_msg->stanza = stanza;
    vzbase::MessageData::Ptr msg_data(stanza_msg);

    cached_thread_->Post(this, 0, msg_data);
  }
  return true;
}

void CachedService::ReplaceCachedFile(CachedStanza::Ptr stanza) {
  // 去掉重复的元素
  for(std::size_t i = 0; i < cached_stanzas_.size(); i++) {
    if(cached_stanzas_[i]->path() == stanza->path()) {
      cached_stanzas_.erase(cached_stanzas_.begin() + i);
    }
  }
  cached_stanzas_.push_back(stanza);
}

bool CachedService::RemoveFile(const std::string path) {
  LOG(INFO) << "Remove file " << path;
  BOOST_ASSERT(cachedstanza_pool_ != NULL);
  for(std::deque<CachedStanza::Ptr>::iterator iter = cached_stanzas_.begin();
      iter != cached_stanzas_.end(); ++iter) {
    if((*iter)->path() == path) {
      //cached_service_.post(
      //  boost::bind(&CachedService::OnAsyncRemoveFile, this, path));
      OnAsyncRemoveFile(path);
      cached_stanzas_.erase(iter);
      break;
    }
  }
  return true;
}

void CachedService::OnAsyncRemoveFile(std::string path) {
  remove(path.c_str());
}

CachedStanza::Ptr CachedService::GetFile(const char *path) {
  LOG(INFO) << "Get File " << path;
  for(std::deque<CachedStanza::Ptr>::iterator iter = cached_stanzas_.begin();
      iter != cached_stanzas_.end(); ++iter) {
    if((*iter)->path() == path) {
      LOG(INFO) << "Find stanza";
      return (*iter);
    }
  }
  BOOST_ASSERT(cachedstanza_pool_ != NULL);
  LOG(INFO) << "Can't to find the file " << path;
  CachedStanza::Ptr stanza = cachedstanza_pool_->TakeStanza(READ_MAX_BUFFER);
  if(!stanza) {
    return CachedStanza::Ptr();
  }

  if(ReadFile(stanza->path(), stanza->data())) {
    // AddFile(stanza, false);
    // stanza->SaveConfimation();
    LOG(INFO) << "Read Stanza";
    return stanza;
  } else {
  }
  LOG(INFO) << "Not find";
  return CachedStanza::Ptr();
}

#ifndef WIN32
void CachedService::iMakeDirRecursive(const char *pPath) {

  if(pPath == NULL)
    return;

  int i,j;
  const char *p = pPath;
  char buf[512] = {0};
  int len = strlen(pPath);

  //去掉连续的 '/'
  for(i = 0,j = 0; i < len && j < 512; i++,j++) {
    buf[j] = p[i];
    if(p[i] == '/') {
      while(p[i] == '/') {
        i++;
      }
      j++;
      buf[j] = p[i];
    }
  }

  char *q = buf;
  char *pQ;
  //递归创建文件夹
  while((pQ = strchr(q, '/')) != NULL) {
    int rt;
    struct stat st;

    *pQ = 0;                //每次创建一级目录，结束符
    rt = stat(buf, &st);
    if(!(rt >= 0 && S_ISDIR(st.st_mode))) { //如果不是目录则创建，是则往下搜索
      mkdir(buf, 0777);
    }
    q = pQ + 1;
    *pQ = '/';              //恢复
  }
}
#else
#endif

#ifndef WIN32
#define vzsleep(x) usleep(x * 1000);
#else
#define vzsleep(x) Sleep(x)
#endif

void CachedService::OnAsyncSaveFile(CachedStanza::Ptr stanza) {
  BOOST_ASSERT(stanza && (!stanza->IsSaved()));
  static const std::size_t CHAUNK_SIZE = 32 * 1024;
  static const uint32 RECURSION_TIME = 128;
  static const uint32 MAX_TRY_TIMES  = 4;
  //// For Debug
  //stanza->SaveConfimation();
  //LOG(INFO) << "save cached file " << stanza->path();
  //return;
#ifndef WIN32
  iMakeDirRecursive(stanza->path().c_str());
#endif
  FILE *fp = fopen(stanza->path().c_str(), "wb");
  if(fp == NULL) {
    LOG(ERROR) << "Failure to open file " << stanza->path();
    stanza->SaveConfimation();
    return;
  }
  uint32 recursion_time = RECURSION_TIME;
  const char *pdata = stanza->DataString();
  std::size_t data_size = stanza->size();
  std::size_t write_size = 0;
  std::size_t try_write_times = MAX_TRY_TIMES;

  for(std::size_t i = 0; i < data_size; i += CHAUNK_SIZE) {
    std::size_t remain_size = data_size - i;
    std::size_t res = 0;
    if(remain_size > CHAUNK_SIZE) {
      res = fwrite(pdata + i, 1, CHAUNK_SIZE, fp);
    } else {
      res = fwrite(pdata + i, 1, remain_size, fp);
    }
    //LOG(INFO) << "res = " << res
    //          << "\t write size = "
    //          << write_size
    //          << "\t" << stanza->size();
    if (ferror(fp) && try_write_times) {
      LOG(ERROR) << "Write file error: size != stanza->data.size() "
                 << ferror(fp);
      perror("Failure to write file");
      LOG(ERROR) << "Try times " << MAX_TRY_TIMES - try_write_times;
      clearerr(fp);
      break;
    }
    if(res == 0) {
      break;
    } else {
#ifndef WIN32
      // Mandatory file synchronization
      // fsync(fileno(fp));
#endif
    }
    write_size += res;
    if(recursion_time) {
      vzsleep(recursion_time);
      recursion_time = recursion_time >> 1;
    }
  }
  if(write_size != stanza->size()) {
    LOG(ERROR) << "Write file error: size != stanza->data.size() "
               << ferror(fp);
    perror("Failure to write file");
  } else {
    LOG(INFO) << "save cached file " << stanza->path()
      << " stanze use count " << stanza.use_count();
  }
  stanza->SaveConfimation();
  fclose(fp);
}

bool CachedService::ReadFile(const std::string path,
                             std::vector<char> &data_buffer) {
  static char read_buffer[READ_FILE_BUFFER_SIZE];
  FILE *fp = fopen(path.c_str(), "rb");
  if(fp == NULL) {
    LOG(ERROR) << "Failure to open the file " << path;
    return false;
  }
  data_buffer.resize(0);
  while(!feof(fp)) {
    int read_size = fread((void *)read_buffer, 1, READ_FILE_BUFFER_SIZE, fp);
    if(read_size > 0) {
      data_buffer.insert(data_buffer.end(),
                         read_buffer,
                         read_buffer + read_size);
    } else {
      break;
    }
  }
  fclose(fp);
  return true;
}

void CachedService::RemoveOutOfDataStanza() {
  BOOST_ASSERT(cachedstanza_pool_ != NULL);
  LOG(INFO) << "cached stanza = " << cached_stanzas_.size()
    << " cache_size_ " << cache_size_;
  /*##rjx## 如果缓存队首数据未被存储，但队列中数据存在已经存储的则不能及时回收*/
  while(cached_stanzas_.size() > cache_size_) {
    CachedStanza::Ptr stanza = cached_stanzas_.front();
    if(stanza && stanza->IsSaved()) {
      cached_stanzas_.pop_front();
    }/* else {
      break;
    }*/
  }
}


}
