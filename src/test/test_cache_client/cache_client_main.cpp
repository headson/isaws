#include "vzbase/helper/stdafx.h"
#include <string.h>

#include "vzbase/base/basictypes.h"
#include "cacheserver/client/cachedclient_c.h"

void Cache_GetFileCallBack(const char *p_path,
                           int         n_path,
                           const char *p_data,
                           int         n_data,
                           void       *p_usr_arg) {
  ((char*)p_path)[n_path] = '\0';
  ((char*)p_data)[n_data] = '\0';
  printf("callback get %s \t %s \t %d.\n", p_path, p_data, n_data);
}

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);
#ifdef WIN32
  ShowVzLoggingAlways();
#endif

  Cached_Start("192.168.6.8", 5499);

  uint32 n_path = 0;
  char   s_path[64]   = "d:\\log\\hello";

  uint32 n_data = 0;
  char   s_data[128];

  while(1) {

    snprintf(s_path, 63, "d:/log/hello_%d", time(NULL));
    n_path = strlen(s_path);

    snprintf(s_data, 127, "key_work-%d", time(NULL));
    n_data = strlen(s_data);
    Cached_SaveFile(s_path, n_path, s_data, n_data);

    Cached_GetFile(s_path, n_path, Cache_GetFileCallBack, NULL);

    Cached_DeleteFile(s_path, n_path);
  }

  getchar();
  Cached_Stop();
  return 0;
}
