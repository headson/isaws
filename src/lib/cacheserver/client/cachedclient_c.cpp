#include "cacheserver/client/cachedclient_c.h"

#include "vzbase/helper/stdafx.h"
#include "cacheserver/client/cachedclient.h"

static cached::CachedClient *g_cache_client = NULL;

int Cached_Start(const char *server, unsigned short port) {
  if (g_cache_client == NULL) {
    g_cache_client = cached::CachedClient::Create(server, port);
    if (g_cache_client == NULL) {
      return CACHED_RET_FAILURE;
    }
  }
  return CACHED_RET_SUCCEED;
}

int Cached_Stop() {
  if(g_cache_client != NULL) {
    delete g_cache_client;
    g_cache_client = NULL;
    return CACHED_RET_SUCCEED;
  }
  return CACHED_RET_FAILURE;
}

// SetKey, if the path not exist, then create the path
// Return CACHED_RET_SUCCEED, CACHED_RET_FAILURE
int Cached_SaveFile(const char *path, int path_size,
                    const char *data, int data_size) {
  if (g_cache_client == NULL) {
    return CACHED_RET_FAILURE;
  }

  if(g_cache_client->SaveCachedFile(path, path_size, data, data_size)) {
    return CACHED_RET_SUCCEED;
  }
  return CACHED_RET_FAILURE;
}

// Get the path
// Return CACHED_RET_SUCCEED, CACHED_RET_FAILURE
int Cached_GetFile(const char *path, int path_size,
                   Cached_GetFileCallback call_back,
                   void *user_data) {
  if (g_cache_client == NULL) {
    return CACHED_RET_FAILURE;
  }

  if(g_cache_client->GetCachedFile(path, path_size, call_back, user_data)) {
    return CACHED_RET_SUCCEED;
  }
  return CACHED_RET_FAILURE;
}

// Delete the path
// Return CACHED_RET_SUCCEED, CACHED_RET_FAILURE
int Cached_DeleteFile(const char *path, int path_size) {
  if (g_cache_client == NULL) {
    return CACHED_RET_FAILURE;
  }

  if(g_cache_client->DeleteCachedFile(path, path_size)) {
    return CACHED_RET_SUCCEED;
  }
  return CACHED_RET_FAILURE;
}
