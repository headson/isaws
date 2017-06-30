#ifndef CACHESERVER_BASE_BASEDEFINES_H_
#define CACHESERVER_BASE_BASEDEFINES_H_

enum {
  CACHED_SELECT = 0,
  CACHED_REPLACE = 1,
  CACHED_DELETE = 2,

  CACHED_SUCCEED = 3,
  CACHED_FAILURE = 4
};

#define MAX_CACHED_PATH_SIZE 248

typedef struct _CacheMessage {
  unsigned int type;
  unsigned int id;
  char         path[MAX_CACHED_PATH_SIZE];
  char         data[0];
} CacheMessage;

#endif // CACHESERVER_BASE_BASEDEFINES_H_
