#include "vzbase/helper/stdafx.h"

#include "dispatcher/base/pkghead.h"
#include "dispatcher/sync/dpclient_c.h"

void kvdb_GetKeyCallBack(const char *p_key,
                         int         n_key,
                         const char *p_value,
                         int         n_value,
                         void       *p_usr_arg) {
  ((char*)p_key)[n_key] = '\0';
  ((char*)p_value)[n_value] = '\0';
  printf("callback get %s \t %s \ t %d.\n", p_key, p_value, n_value);
}

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);
#ifdef WIN32
  ShowVzLoggingAlways();
#endif

  Kvdb_Start("192.168.6.8", 5299);

  uint32 n_key = 0;
  char   s_key[64]   = "hello";

  uint32 n_value = 0;
  char   s_value[128];

  while(1) {    
    snprintf(s_key, 63, "key_work-%d", time(NULL));
    n_key = strlen(s_key);

    snprintf(s_value, 127, "key_work-%d", time(NULL));
    n_value = strlen(s_value);
    Kvdb_SetKey(s_key, n_key, s_value, n_value);

    memset(s_value, 0, 128);
    Kvdb_GetKeyToBuffer(s_key, n_key, s_value, 127);
    printf("get %s \t %s.\n", s_key, s_value);

    Kvdb_BackupDatabase();

    memset(s_value, 0, 128);
    Kvdb_GetKey(s_key, n_key, kvdb_GetKeyCallBack, NULL);

    Kvdb_DeleteKey(s_key, n_key);
    memset(s_value, 0, 128);
    if (Kvdb_GetKeyToBuffer(s_key, n_key, s_value, 127) == KVDB_SUCCEED) {
      printf("get %s \t %s.\n", s_key, s_value);
    }

    Kvdb_RestoreDatabase();
  }

  getchar();
  Kvdb_Stop();
  return 0;
}
