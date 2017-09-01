#include "cacheserver/server/cachedservice.h"
#include "vzlogging/logging/vzloggingcpp.h"
#include "json/json.h"

#ifndef WIN32

#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#else
#endif

namespace cached {


// -----------------------------------------------------------------------------
#ifdef WIN32
#define FILE_LIMIT_CONFIG "E:/work/vscp/code/branches/ftp_dir/file_limit_config.json"
#else
#define FILE_LIMIT_CONFIG "/tmp/app/exec/file_limit_config.json"
#endif

#define MAX_PATH_SIZE         256

#define JSON_FLC_LIMIT_CHECKS "limit_checks"
#define JSON_FLC_PATH         "path"
#define JSON_FLC_MAX_SIZE     "max_size"

#define SD_CAP_PATH    "/media/mmcblk0p1/VzIPCCap"

CachedService::CachedService(vzbase::Thread *cached_thread,
                             std::size_t cache_size)
  : cache_size_(cache_size),
    cached_thread_(cached_thread),
    current_cache_size_(0),
    current_stanzas_index_(0) {
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
  if (InitFileLimitCheck()) {
    WattingNextTransactionAsyncEvent();
  }
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
  switch (msg->message_id) {
  case CACHED_ADD: {
    StanzaMessageData *stanza_msg =
      static_cast<StanzaMessageData *>(msg->pdata.get());
    if (stanza_msg) {
      OnAsyncSaveFile(stanza_msg->stanza);
    }
  }
  break;
  case CACHED_CHECK: {
    CheckFileLimit();
    WattingNextTransactionAsyncEvent();
  }
  break;
  default:
    break;
  }
}

void CachedService::CheckFileLimit() {
  // Check to reset stanza index
  if (current_stanzas_index_ >= flc_stanzas_.size()) {
    current_stanzas_index_ = 0;
  }
  FlcStanza &stanza = flc_stanzas_[current_stanzas_index_];
  current_stanzas_index_++;
  uint32 file_size = 0;

#ifndef WIN32
  struct stat statbuf;
  if (!stat(stanza.path.c_str(), &statbuf)) {
    file_size = statbuf.st_size;
  }
#endif
  LOG(L_INFO) << "The file info "
            << stanza.max_size << "\t"
            << file_size << "\t"
            << stanza.path;
  if (stanza.max_size < file_size) {
    // std::remove(stanza.path.c_str());
	::remove(stanza.path.c_str());
  }
}

void CachedService::WattingNextTransactionAsyncEvent() {
  cached_thread_->PostDelayed(ASYNC_TIMEOUT_TIMES * 1000,
                              this,
                              CACHED_CHECK);
}


bool CachedService::InitFileLimitCheck() {
  // Open this file
  FILE * fp = fopen(FILE_LIMIT_CONFIG, "rb");
  if (fp == NULL) {
    LOG(L_ERROR) << "Failure to open " << FILE_LIMIT_CONFIG;
    return false;
  }

  do {
    // Read the file data
    char temp[128];
    std::string data;
    while(true) {
      std::size_t read_size = fread(temp, sizeof(char), 128, fp);
      if (read_size) {
        data.append(temp, read_size);
      } else {
        break;
      }
    }
    // Check the file data is empty
    if (data.empty()) {
      LOG(L_ERROR) << "The file is empty " << FILE_LIMIT_CONFIG;
      break;
    }

    // Parse the file data to flc_stanzas_
    Json::Reader reader;
    Json::Value  value;
    if (!reader.parse(data, value)) {
      LOG(L_ERROR) << "Parse the file data error "
                 << reader.getFormattedErrorMessages();
      break;
    }

    Json::Value limit_checks = value[JSON_FLC_LIMIT_CHECKS];
    for (std::size_t i = 0; i < limit_checks.size(); i++) {
      if (limit_checks[i][JSON_FLC_PATH].isNull()
          || limit_checks[i][JSON_FLC_MAX_SIZE].isNull()) {
        LOG(L_ERROR) << "Failure Item";
        continue;
      }
      FlcStanza stanza;
      stanza.max_size   = limit_checks[i][JSON_FLC_MAX_SIZE].asUInt();
      stanza.path       = limit_checks[i][JSON_FLC_PATH].asString();
      LOG(L_INFO) << stanza.max_size << "\t" << stanza.path;
      flc_stanzas_.push_back(stanza);
    }

  } while(0);

  fclose(fp);

  if (flc_stanzas_.size()) {
    return true;
  }
  return false;
}

bool CachedService::AddFile(CachedStanza::Ptr stanza, bool is_cached) {
  LOG(L_INFO) << "Cached file = " << stanza->path()
            <<" is cached "<< is_cached
            <<" stanza use count "<<stanza.use_count();
  ReplaceCachedFile(stanza);
  // Try to remove out of data stanza
  RemoveOutOfDataStanza();
  if(cached_stanzas_.size() > MAX_CACHED_STANZA_SIZE) {
    stanza->SaveConfimation();
    LOG(L_WARNING) << "cached_stanzas size big than 64 " << cached_stanzas_.size();
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
  LOG(L_INFO) << "Remove file " << path;
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
  LOG(L_INFO) << "Get File " << path;
  for(std::deque<CachedStanza::Ptr>::iterator iter = cached_stanzas_.begin();
      iter != cached_stanzas_.end(); ++iter) {
    if((*iter)->path() == path) {
      LOG(L_INFO) << "Find stanza";
      return (*iter);
    }
  }
  BOOST_ASSERT(cachedstanza_pool_ != NULL);
  LOG(L_INFO) << "Can't to find the file " << path;
  CachedStanza::Ptr stanza = cachedstanza_pool_->TakeStanza(READ_MAX_BUFFER);
  if(!stanza) {
    return CachedStanza::Ptr();
  }
  stanza->SetPath(path);

  if(ReadFile(stanza->path(), stanza->data())) {
    // AddFile(stanza, false);
    // stanza->SaveConfimation();
    LOG(L_INFO) << "Read Stanza";
    return stanza;
  } else {
  }
  LOG(L_INFO) << "Not find";
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
    LOG(L_ERROR) << "Failure to open file " << stanza->path();
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
      LOG(L_ERROR) << "Write file error: size != stanza->data.size() "
                 << ferror(fp);
      perror("Failure to write file");
      LOG(L_ERROR) << "Try times " << MAX_TRY_TIMES - try_write_times;
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
    LOG(L_ERROR) << "Write file error: size != stanza->data.size() "
               << ferror(fp);
    perror("Failure to write file");
  } else {
    LOG(L_INFO) << "save cached file " << stanza->path()
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
    LOG(L_ERROR) << "Failure to open the file " << path;
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
  LOG(L_INFO) << "cached stanza = " << cached_stanzas_.size()
            << " cache_size_ " << cache_size_;
  /*##rjx## 如果缓存队首数据未被存储，但队列中数据存在已经存储的则不能及时回收*/
  while(cached_stanzas_.size() > cache_size_) {
    CachedStanza::Ptr stanza = cached_stanzas_.front();
    if(stanza && stanza->IsSaved()) {
      cached_stanzas_.pop_front();
    } else {
      //break;
      vzsleep(1);
    }
  }
}


}
