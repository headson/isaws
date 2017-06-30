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

CachedService::CachedService(vzconn::EventService &event_service)
  : event_service_(event_service) {
}

CachedService::~CachedService() {
}

bool CachedService::Start() {
  return true;
}

bool CachedService::AddFile(const char *path,
                            const uint8 *data, uint32 size,
                            bool is_cached) {
  AsyncSaveFile(path, data, size);
  return true;
}

//void CachedService::ReplaceCachedFile(CachedStanza::Ptr stanza) {
//  // 去掉重复的元素
//  for(std::size_t i = 0; i < cached_stanzas_.size(); i++) {
//    if(cached_stanzas_[i]->path() == stanza->path()) {
//      cached_stanzas_.erase(cached_stanzas_.begin() + i);
//    }
//  }
//  cached_stanzas_.push_back(stanza);
//}

bool CachedService::RemoveFile(const char *path) {
  AsyncRemoveFile(path);
  return true;
}

void CachedService::AsyncRemoveFile(const char *file_path) {
  remove(file_path);
}

bool CachedService::GetFile(const char *path, std::vector<char> &buffer) {

  LOG(L_INFO) << "Get File " << path;

  if(ReadFile(path, buffer)) {
    LOG(L_INFO) << "Read Stanza";
    return true;
  } else {
  }
  LOG(L_INFO) << "Not find";
  return false;
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

void CachedService::AsyncSaveFile(const char *path,
                                  const uint8 *data,
                                  uint32 size) {

  static const std::size_t CHAUNK_SIZE = 32 * 1024;
  static const uint32 RECURSION_TIME = 128;
  static const uint32 MAX_TRY_TIMES  = 4;
  //// For Debug
  //stanza->SaveConfimation();
  //LOG(INFO) << "save cached file " << stanza->path();
  //return;
#ifndef WIN32
  iMakeDirRecursive(path);
#endif
  FILE *fp = fopen(path, "wb");
  if(fp == NULL) {
    LOG(L_ERROR) << "Failure to open file " << path;
    return;
  }
  uint32 recursion_time = RECURSION_TIME;
  const char *pdata = (const char *)data;
  std::size_t data_size = size;
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
  if(write_size != size) {
    LOG(L_ERROR) << "Write file error: size != stanza->data.size() "
                 << ferror(fp);
    perror("Failure to write file");
  } else {
    LOG(L_INFO) << "save cached file " << path;
  }
  fclose(fp);
}

bool CachedService::ReadFile(const char *path, std::vector<char> &data_buffer) {
  static char read_buffer[READ_FILE_BUFFER_SIZE];
  FILE *fp = fopen(path, "rb");
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


}
