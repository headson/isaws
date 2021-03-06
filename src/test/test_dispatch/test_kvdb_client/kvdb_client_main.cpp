#include "vzbase/helper/stdafx.h"

#include <vector>

#include "dispatcher/sync/dpclient_c.h"
#include "dispatcher/sync/ckvdbclient.h"

void kvdb_GetKeyCallBack(const char *p_key,
                         int         n_key,
                         const char *p_value,
                         int         n_value,
                         void       *p_usr_arg) {
  ((char*)p_key)[n_key] = '\0';
  ((char*)p_value)[n_value] = '\0';
  //printf("callback get %s \t %s \ t %d.\n", p_key, p_value, n_value);
}

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);
#ifdef WIN32
  ShowVzLoggingAlways();
#endif

  uint32 n_key = 0;
  char   s_key[64]   = "NetworkInterface_Cfg";

#if 1
  Kvdb_Start("127.0.0.1", 5299);

  uint32 n_value = 0;
  char   s_value[128];

  Kvdb_Transaction(KVDB_TRANS_BEGIN);

  
  while(1) {
    snprintf(s_key, 63, "key_work-%d", time(NULL));
    snprintf(s_key, 63, "key-%d", rand() % ((uint32)-1));
    n_key = strlen(s_key);

    snprintf(s_value, 127, "key_work-%d", time(NULL));
    snprintf(s_value, 127, "value-%d", rand() % ((uint32)-1));
    n_value = strlen(s_value);
    Kvdb_SetKey(s_key, n_key, s_value, n_value);

    memset(s_value, 0, 128);
    Kvdb_GetKeyToBuffer(s_key, n_key, s_value, 127);
    printf("get %s \t %s.\n", s_key, s_value);
    //memset(s_value, 0, 128);
    //Kvdb_GetKeyToBuffer(s_key, n_key, s_value, 127);
    //printf("get %s \t %s.\n", s_key, s_value);

    Kvdb_BackupDatabase();
    //Kvdb_BackupDatabase();

    memset(s_value, 0, 128);
    Kvdb_GetKey(s_key, n_key, kvdb_GetKeyCallBack, NULL);

    Kvdb_DeleteKey(s_key, n_key);
    memset(s_value, 0, 128);
    if (Kvdb_GetKeyToBuffer(s_key, n_key, s_value, 127) == KVDB_SUCCEED) {
      printf("get %s \t %s.\n", s_key, s_value);
    }

    Kvdb_RestoreDatabase();
  }
#endif

  //CKvdbClient* p_kvdb = CKvdbClient::Create("192.168.103.15", 5299);
  CKvdbClient* p_kvdb = CKvdbClient::Create("192.168.6.8", 5299);

  n_key = strlen(s_key);

  std::string s_val = "asdasdasdasdasdasdasd";
  int res = p_kvdb->SetKey(s_key, s_val.c_str(), s_val.size());

  while (p_kvdb) {
    Sleep(1000);
    //snprintf(s_key, 63, "key_work-%d", time(NULL));
    //n_key = strlen(s_key);

    //snprintf(s_value, 127, "key_work-%d", time(NULL));
    //n_value = strlen(s_value);
    //p_kvdb->SetKey(s_key, n_key, s_value, n_value);

    ////memset(s_value, 0, 128);
    ////p_kvdb->GetKey(s_key, n_key, s_value, 127);
    ////printf("get %s \t %s.\n", s_key, s_value);

    //p_kvdb->BackupDatabase();





































    //memset(s_value, 0, 128);
    //p_kvdb->GetKey(s_key, n_key, kvdb_GetKeyCallBack, NULL);
    //Kvdb_GetKey(s_key, n_key, kvdb_GetKeyCallBack, NULL);

    //std::string s_val;
    //p_kvdb->GetKey(s_key, n_key, &s_val);
    //printf("get %s \t %s.\n", s_key, s_val.c_str());

    //p_kvdb->Delete(s_key, n_key);
    //Kvdb_DeleteKey(s_key, n_key);
    //memset(s_value, 0, 128);
    //if (p_kvdb->GetKey(s_key, n_key, s_value, 127) == KVDB_SUCCEED) {
    //if (Kvdb_GetKeyToBuffer(s_key, n_key, s_value, 127) == KVDB_SUCCEED) {
    //  printf("get %s \t %s.\n", s_key, s_value);
    //}
    s_val = "asdasdasdasdasdasdasd";
    res = p_kvdb->SetKey(s_key, s_val.c_str(), s_val.size());

    p_kvdb->RestoreDatabase();

    s_val = "asdasdasdasdasdasdasd";
    res = p_kvdb->SetKey(s_key, s_val.c_str(), s_val.size());
    //Kvdb_RestoreDatabase();



  }

  getchar();
  Kvdb_Stop();
  return 0;
}
