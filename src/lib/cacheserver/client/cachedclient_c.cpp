#include "cacheserver/client/cachedclient_c.h"

#include "vzbase/helper/stdafx.h"
#include "cacheserver/client/cachedclient.h"

static char                  g_cache_addr[64] = { 0 };
static unsigned short        g_cache_port = 0;
static cached::CachedClient *g_cache_client = NULL;

cached::CachedClient* GetCacheCli(bool b_can_reconn = true) {
  if (g_cache_client) {
    if (g_cache_client->isClose()) {        // 断开链接
      delete g_cache_client;
      g_cache_client = NULL;
    }
  }

  if (b_can_reconn && g_cache_client == NULL) {
    // 新建
    g_cache_client = cached::CachedClient::Create();
    if (g_cache_client == NULL) {
      LOG(L_ERROR) << "create client failed.";
      return NULL;
    }

    // 链接
    vzconn::CInetAddr c_addr(g_cache_addr, g_cache_port);
    bool b_ret = g_cache_client->Connect(&c_addr,
                                        false,
                                        true,
                                        DEF_TIMEOUT_MSEC);
    if (false == b_ret) {
      LOG(L_ERROR) << "connect to server failed " << c_addr.ToString();
      delete g_cache_client;
      g_cache_client = NULL;
      return NULL;
    }
  }
  return g_cache_client;
}

int Cached_Start(const char *server, unsigned short port) {
  g_cache_port = port;

  memset(g_cache_addr, 0, 63);
  strncpy(g_cache_addr, server, 63);

  return CACHED_SUCCEED;
}

int Cached_Stop() {
  if(g_cache_client != NULL) {
    delete g_cache_client;
    g_cache_client = NULL;
    return CACHED_SUCCEED;
  }
  return CACHED_FAILURE;
}

// SetKey, if the path not exist, then create the path
// Return CACHED_SUCCEED, CACHED_FAILURE
int Cached_SaveFile(const char *path, int path_size,
                    const char *data, int data_size) {
  cached::CachedClient* p_cli = GetCacheCli();
  if (p_cli == NULL) {
    return CACHED_FAILURE;
  }
  if(p_cli != NULL) {
    if(p_cli->SaveCachedFile(path, path_size, data, data_size)) {
      return CACHED_SUCCEED;
    }
  }
  return CACHED_FAILURE;
}

// Get the path
// Return CACHED_SUCCEED, CACHED_FAILURE
int Cached_GetFile(const char *path, int path_size,
                   Cached_GetFileCallback call_back,
                   void *user_data) {
  cached::CachedClient* p_cli = GetCacheCli();
  if (p_cli == NULL) {
    return CACHED_FAILURE;
  }

  if(p_cli->GetCachedFile(path, path_size, call_back, user_data)) {
    return CACHED_SUCCEED;
  }
  return CACHED_FAILURE;
}

// Delete the path
// Return CACHED_SUCCEED, CACHED_FAILURE
int Cached_DeleteFile(const char *path, int path_size) {
  cached::CachedClient* p_cli = GetCacheCli();
  if (p_cli == NULL) {
    return CACHED_FAILURE;
  }

  if(p_cli != NULL) {
    if(p_cli->DeleteCachedFile(path, path_size)) {
      return CACHED_SUCCEED;
    }
  }
  return CACHED_FAILURE;
}
