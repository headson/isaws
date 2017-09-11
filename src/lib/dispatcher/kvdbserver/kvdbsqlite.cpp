#include "dispatcher/kvdbserver/kvdbsqlite.h"
#include "vzlogging/logging/vzloggingcpp.h"
#include "dispatcher/base/pkghead.h"

#include "vzbase/base/timeutils.h"

#include <string.h>
#include <stdio.h>
#include<stdlib.h>
#ifdef WIN32
#include <io.h>
#else
#include <unistd.h>
#endif // WIN32
namespace kvdb {

////////////////////////////////////////////////////////////////////////////////

const char KVDB_SQL[] = "CREATE TABLE [kvdb] ("
                        "  [key] CHAR(64) NOT NULL ON CONFLICT REPLACE,"
                        "  [value] BLOB NOT NULL ON CONFLICT REPLACE, "
                        "  CONSTRAINT [] PRIMARY KEY ([key]) ON CONFLICT REPLACE);";

const char REPLACE_STMT_SQL[] = "REPLACE INTO kvdb(key, value) VALUES (?, ?);";
const char DELETE_STMT_SQL[] = "DELETE FROM kvdb where kvdb.key = ?;";
const char SELECT_STMT_SQL[] = "SELECT kvdb.value from kvdb where kvdb.key = ?";
const char KVDB_SUFFIX[] = "_back";

bool KvdbSqlite::is_init_memory_use_ = false;

KvdbSqlite::KvdbSqlite()
  : db_instance_(NULL),
    replace_stmt_(NULL),
    delete_stmt_(NULL),
    select_stmt_(NULL),
    kvdb_path_(""),
    kvdb_bak_path_(""),
    backup_path_(""),
    is_db_update_(1) {
}

KvdbSqlite::~KvdbSqlite() {
  UinitKvdb();
}

void KvdbSqlite::RemoveDatabase(const char *kvdb_path) {
  UinitKvdb();
  remove(kvdb_path);
}

bool KvdbSqlite::InitKvdb(const char *kvdb_path, const char *backup_path) {
  //
  kvdb_path_ = kvdb_path;
  kvdb_bak_path_ = kvdb_path_;
  kvdb_bak_path_.append(KVDB_SUFFIX);
  if(backup_path) {
    backup_path_ = backup_path;
    InitBackupDatabase(backup_path);
  }
  CheckDBRecover();

  //
  bool file_exsits = CheckFileExsits(kvdb_path);
  //
  int res = sqlite3_open(kvdb_path, &db_instance_);
  if(res) {
    LOG(L_ERROR) << "Failure to open the database";
    sqlite3_close(db_instance_);
    return false;
  }
  if(!file_exsits) {
    char *sqlite_error_msg = 0;
    res = sqlite3_exec(db_instance_, KVDB_SQL, NULL, 0, &sqlite_error_msg);
    if(res != SQLITE_OK) {
      LOG(L_ERROR) << "Failure to create exec this command" << KVDB_SQL;
      sqlite3_free(sqlite_error_msg);
      return false;
    }
  }

  // add by pyh 2017-08-31 数据库损坏
  res = sqlite3_exec(db_instance_, "PRAGMA synchronous = FULL;", 0, 0, 0);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "PRAGMA journal_mode = MEMORY;";
    return false;
  }

  //res = sqlite3_exec(db_instance_, "PRAGMA journal_mode = MEMORY;", 0, 0, 0);
  //if (res != SQLITE_OK) {
  //  LOG(L_ERROR) << "PRAGMA journal_mode = MEMORY;";
  //  return false;
  //}

  //res = sqlite3_exec(db_instance_, "BEGIN TRANSACTION;", 0,0,0);
  //if(res != SQLITE_OK) {
  //  LOG(L_ERROR) << "Process transaction begin;";
  //  return false;
  //}

  return InitStmt();
}

void KvdbSqlite::UinitKvdb() {
  int res = 0;
  if (replace_stmt_) {
    res = sqlite3_finalize(replace_stmt_);
    if (res != SQLITE_OK) {
      LOG(L_ERROR) << "Failure to finalize replace_stmt_";
    }
    replace_stmt_ = NULL;
  }

  if (delete_stmt_) {
    res = sqlite3_finalize(delete_stmt_);
    if (res != SQLITE_OK) {
      LOG(L_ERROR) << "Failure to finalize delete_stmt_";
    }
    delete_stmt_ = NULL;
  }

  if (select_stmt_) {
    res = sqlite3_finalize(select_stmt_);
    if (res != SQLITE_OK) {
      LOG(L_ERROR) << "Failure to finalize remove_stmt_";
    }
    select_stmt_ = NULL;
  }

  if (db_instance_) {
    //res = sqlite3_close(db_instance_);
    res = sqlite3_close_v2(db_instance_);
    db_instance_ = NULL;
  }
}

bool KvdbSqlite::InitBackupDatabase(const char *backup_path) {
  bool file_exsits = CheckFileExsits(backup_path);
  if(!file_exsits) {
    sqlite3 *db_backup;
    int res = sqlite3_open(backup_path, &db_backup);
    if(res) {
      LOG(L_ERROR) << "Failure to open the database";
      sqlite3_close(db_backup);
      return false;
    }
    char *sqlite_error_msg = 0;
    res = sqlite3_exec(db_backup, KVDB_SQL, NULL, 0, &sqlite_error_msg);
    if(res != SQLITE_OK) {
      LOG(L_ERROR) << "Failure to create exec this command" << KVDB_SQL;
      sqlite3_free(sqlite_error_msg);
      return false;
    }
    sqlite3_close(db_backup);
  }
  return true;
}

void KvdbSqlite::CheckTransaction() {
  //is_transaction_ ++;
  //if((is_transaction_ % MAX_TRANSACTION_SIZE) == 0) {
  //  int res = 0;
  //  res = sqlite3_exec(db_instance_, "END TRANSACTION;", 0,0,0);
  //  if(res != SQLITE_OK) {
  //    LOG(L_ERROR) << "Process transaction commit;";
  //    return;
  //  }
  //  res = sqlite3_exec(db_instance_, "BEGIN TRANSACTION;", 0,0,0);
  //  if(res != SQLITE_OK) {
  //    LOG(L_ERROR) << "Process transaction begin;";
  //    return;
  //  }
  //  is_transaction_ = 1;
  //}
}

void KvdbSqlite::ForceTransaction() {
  //int res = 0;
  //res = sqlite3_exec(db_instance_, "commit;", 0,0,0);
  //if(res != SQLITE_OK) {
  //  LOG(L_ERROR) << "Process transaction commit;";
  //  return;
  //}
  //res = sqlite3_exec(db_instance_, "begin;", 0,0,0);
  //if(res != SQLITE_OK) {
  //  LOG(L_ERROR) << "Process transaction begin;";
  //  return;
  //}
  //is_transaction_ = 1;
}

bool KvdbSqlite::InitStmt() {
  int res = 0;
  res = sqlite3_prepare_v2(db_instance_,
                           REPLACE_STMT_SQL,
                           strlen(REPLACE_STMT_SQL),
                           &replace_stmt_,
                           0);
  if(res != SQLITE_OK) {
    LOG(L_ERROR) << REPLACE_STMT_SQL;
    return false;
  }
  LOG_WARNING("replace_stmt_ 0x%x.\n", replace_stmt_);

  res = sqlite3_prepare_v2(db_instance_,
                           DELETE_STMT_SQL,
                           strlen(DELETE_STMT_SQL),
                           &delete_stmt_,
                           0);
  if(res != SQLITE_OK) {
    LOG(L_ERROR) << DELETE_STMT_SQL;
    return false;
  }
  LOG_WARNING("delete_stmt_ 0x%x.\n", delete_stmt_);

  res = sqlite3_prepare_v2(db_instance_,
                           SELECT_STMT_SQL,
                           strlen(SELECT_STMT_SQL),
                           &select_stmt_,
                           0);
  if(res != SQLITE_OK) {
    LOG(L_ERROR) << SELECT_STMT_SQL;
    return false;
  }
  LOG_WARNING("select_stmt_ 0x%x.\n", select_stmt_);
  return true;
}

bool KvdbSqlite::CheckFileExsits(const char *kvdb_path) {
  FILE *fp = fopen(kvdb_path, "rb");
  if(fp == NULL) {
    return false;
  }
  fclose(fp);
  return true;
}

bool KvdbSqlite::ReplaceKeyValue(const char *key, int key_size,
                                 const char *value, int value_size) {
  int res = 0;
  if (NULL == replace_stmt_) {
    LOG(L_ERROR) << "ReplaceKeyValue use NULL stmt";
    return false;
  }

  // CheckTransaction();
  LOG(L_INFO) << "ReplaceKeyValue key " << key
              << " key_size " << key_size
              << " value_size " << value_size;

  res = sqlite3_reset(replace_stmt_);
  if(res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to reset replace_stmt_*";
    return false;
  }

  res = sqlite3_bind_text(replace_stmt_, 1,
                          key,
                          key_size,
                          SQLITE_STATIC);
  if(res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to call sqlite3_bind_*";
    return false;
  }

  res = sqlite3_bind_blob(replace_stmt_, 2,
                          (const void *)value,
                          value_size,
                          SQLITE_STATIC);
  if(res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to call sqlite3_bind_*";
    return false;
  }
  res = sqlite3_step(replace_stmt_);
  if(res != SQLITE_DONE) {
    LOG(L_ERROR) << "Failure to call sqlite3_step";
    return false;
  }
  LOG(L_INFO) << "Replace Key " << res;
  LOG(L_ERROR).write(key, key_size);
  LOGB_INFO(key, key_size);

  is_db_update_++;
  return true;
}

bool KvdbSqlite::DeleteKeyValue(const char *key, int key_size) {
  int res = 0;
  if (NULL == delete_stmt_) {
    LOG(L_ERROR) << "DeleteKeyValue use NULL stmt";
    return false;
  }

  CheckTransaction();
  res = sqlite3_reset(delete_stmt_);
  LOG(L_INFO) << "Delete Key";
  LOG(L_INFO).write(key, key_size);
  if(res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to reset delete_stmt_*";
    return false;
  }
  res = sqlite3_bind_text(delete_stmt_, 1,
                          key,
                          key_size,
                          SQLITE_STATIC);
  if(res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to call sqlite3_bind_*";
    return false;
  }
  res = sqlite3_step(delete_stmt_);
  if(res != SQLITE_DONE) {
    LOG(L_ERROR) << "Failure to call sqlite3_step";
    return false;
  }
  LOG(L_ERROR).write(key, key_size);

  is_db_update_++;
  return true;
}

bool KvdbSqlite::SelectKeyValue(const char *key, int key_size,
                                std::vector<char> &buffer) {
  int res = 0;
  if (NULL == select_stmt_) {
    LOG(L_ERROR) << "SelectKeyValue use NULL stmt";
    return false;
  }

  CheckTransaction();
  LOG(L_WARNING) << "SelectKeyValue key " << key
                 << " key_size " << key_size;

  res = sqlite3_reset(select_stmt_);
  if(res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to reset select_stmt_*";
    return false;
  }
  res = sqlite3_bind_text(select_stmt_, 1,
                          key,
                          key_size,
                          SQLITE_STATIC);
  if(res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to call sqlite3_bind_*";
    return false;
  }

  res = sqlite3_step(select_stmt_);

  if(res == SQLITE_ROW) {
    const void *data = sqlite3_column_blob(select_stmt_, 0);
    int data_size = sqlite3_column_bytes(select_stmt_, 0);
    if (data != NULL) {
      buffer.resize(data_size);
      memcpy(&buffer[0], data, data_size);
      return true;
    }
  }
  LOGB_INFO(key, key_size);
  LOG(L_INFO).write(key, key_size);
  LOG(L_INFO) << "Failure find this key " << res;
  return false;
}

bool KvdbSqlite::TransBegin() {
  int res = sqlite3_exec(db_instance_, "begin;", 0,0,0);
  LOG(L_WARNING) << "TransBegin " << res;
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "trans begin;";
    return false;
  }
  return true;
}

bool KvdbSqlite::TransCommit() {
  int res = sqlite3_exec(db_instance_, "commit;", 0,0,0);
  LOG(L_WARNING) << "TransCommit " << res;
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "trans commit;";
    return false;
  }
  return true;
}

bool KvdbSqlite::TransRollback() {
  char *error_msg = NULL;
  int res = sqlite3_exec(db_instance_,
                         "rollback transaction",
                         0, 0, &error_msg);
  LOG(L_WARNING) << "TransRollback " << res;
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "PRAGMA integrity_check;";
    sqlite3_free(error_msg);
    return false;
  }
  return true;
}

bool KvdbSqlite::BackupDatabase() {
  if(backup_path_.empty()) {
    return true;
  }
  CheckTransaction();
#if 0
  sqlite3 *db_backup = NULL;
  int res = sqlite3_open(backup_path_.c_str(), &db_backup);
  if(res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to open the database " << backup_path_;
    return false;
  }
  bool copy_res = CopyDatabase(db_instance_, db_backup);
  /* Close the database connection opened on database file zFilename
  ** and return the result of this function. */
  res = sqlite3_close(db_backup);
  LOG(L_ERROR)<<"backup database." << res;
  return copy_res;
#else
  UinitKvdb();
  remove(backup_path_.c_str());

  char scmd[1024] = { 0 };
#ifdef _WIN32
  sprintf(scmd, "copy /Y %s %s",
#else
  sprintf(scmd, "cp -arf %s %s;sync",
#endif
          kvdb_path_.c_str(),
          backup_path_.c_str());

  LOG(L_ERROR) << "cmd " << scmd;
  system(scmd);

  return InitKvdb(kvdb_path_.c_str(), backup_path_.c_str());
#endif
}

bool KvdbSqlite::RestoreDatabase(std::string backup_path) {
  if (backup_path.empty() ||
      kvdb_path_.empty()) {
    return true;
  }
#if 0
  LOG(L_WARNING) << "backup database to local ....";
  CheckTransaction();
  sqlite3 *db_backup = NULL;
  int res = sqlite3_open(backup_path.c_str(), &db_backup);
  if(res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to open the database " << backup_path;
    return false;
  }
  bool copy_res = CopyDatabase(db_backup, db_instance_);
  /* Close the database connection opened on database file zFilename
  ** and return the result of this function. */
  res = sqlite3_close(db_backup);
  LOG(L_ERROR)<<"restore database." << res;

  UinitKvdb();
  InitKvdb(kvdb_path_.c_str(), backup_path_.c_str());
  return copy_res;
#else
  UinitKvdb();
  remove(kvdb_path_.c_str());

  char scmd[1024] = { 0 };
#ifdef _WIN32
  sprintf(scmd, "copy /Y %s %s",
#else
  sprintf(scmd, "cp -arf %s %s;sync",
#endif
          backup_path_.c_str(),
          kvdb_path_.c_str());

  LOG(L_ERROR) << "cmd " << scmd;
  system(scmd);

  return InitKvdb(kvdb_path_.c_str(), backup_path_.c_str());
#endif
}

bool KvdbSqlite::CopyDatabase(sqlite3 *from, sqlite3* to) {
  sqlite3_backup *backup_stmt = NULL;
  /* Set up the backup procedure to copy from the "main" database of
  ** connection pFile to the main database of connection pInMemory.
  ** If something goes wrong, pBackup will be set to NULL and an error
  ** code and message left in connection pTo.
  **
  ** If the backup object is successfully created, call backup_step()
  ** to copy data from pFile to pInMemory. Then call backup_finish()
  ** to release resources associated with the pBackup object.  If an
  ** error occurred, then an error code and message will be left in
  ** connection pTo. If no error occurred, then the error code belonging
  ** to pTo is set to SQLITE_OK.
  */
  backup_stmt = sqlite3_backup_init(to, "main",
                                    from, "main");
  if(backup_stmt == NULL) {
    LOG(L_ERROR) << "Failure to call sqlite3_backup_init";
    return false;
  }

  sqlite3_backup_step(backup_stmt, -1);
  sqlite3_backup_finish(backup_stmt);

  /* Close the database connection opened on database file zFilename
  ** and return the result of this function. */
  return true;
}

// check database is damage callback
static int Sqlite3CheckKVDBCallBack(void *user_data, int a, char** b, char** c) {
  if (NULL == user_data) {
    LOG(L_ERROR) << "param is failed.";
    return 0;
  }

  LOG(L_ERROR) << *b;
  if (strncmp(*b, "ok", 2) == 0) {
    *((unsigned int*)user_data) = 0;
  } else {
    *((unsigned int*)user_data) = 1;
  }
  return 0;
}

// true = error, false = no error
static bool Sqlite3CheckError(std::string &db_path) {
  sqlite3 *db = NULL;
  int res = sqlite3_open(db_path.c_str(), &db);
  if (res) {
    LOG(L_ERROR) << "Failure to open the database";
    sqlite3_close(db);
    return true;
  }

  char *error_msg = NULL;
  unsigned int is_error = 0;
  res = sqlite3_exec(db,
                     "PRAGMA integrity_check;",
                     Sqlite3CheckKVDBCallBack,
                     &is_error,
                     &error_msg);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "PRAGMA integrity_check;" << error_msg;
    sqlite3_free(error_msg);
    is_error = 1;
  }
  sqlite3_close(db);

  return ((is_error == 1) ? true : false);
}

bool KvdbSqlite::CheckDBBackup() {
  LOG(L_WARNING) << "to check kvdb and backup ...";
  if (NULL == db_instance_) {
    LOG(L_ERROR) << "have not open database ...";
    return false;
  }

  //sqlite3 *db = NULL;
  //int res = sqlite3_open(kvdb_path_.c_str(), &db);
  //if (res) {
  //  LOG(L_ERROR) << "Failure to open the database";
  //  sqlite3_close(db);
  //  return false;
  //}
  char *error_msg = NULL;
  unsigned int is_error = 0;
  int res = sqlite3_exec(db_instance_,
                         "PRAGMA integrity_check;",
                         Sqlite3CheckKVDBCallBack,
                         &is_error,
                         &error_msg);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "PRAGMA integrity_check;" << error_msg;
    sqlite3_free(error_msg);
    is_error = 1;
  }

  if (1 == is_error) {
    LOG(L_ERROR) << "recover from backup database."<<is_error;
    UinitKvdb();
    // TODO 从备份中恢复数据库
    char scmd[1024] = { 0 };
#ifdef _WIN32
    sprintf(scmd, "copy /Y %s %s",
#else
    sprintf(scmd, "cp -arf %s %s;sync",
#endif
            kvdb_bak_path_.c_str(),
            kvdb_path_.c_str());

    LOG(L_ERROR) << "cmd "<<scmd;
    system(scmd);

    return InitKvdb(kvdb_path_.c_str(), backup_path_.c_str());
  } else {
    if (is_db_update_ > 0) {
      LOG(L_ERROR) << "backup database."<<is_error;
      UinitKvdb();
      // TODO 备份数据库
      char scmd[1024] = { 0 };
#ifdef _WIN32
      sprintf(scmd, "copy /Y %s %s",
#else
      sprintf(scmd, "cp -arf %s %s;sync",
#endif
              kvdb_path_.c_str(),
              kvdb_bak_path_.c_str());

      LOG(L_ERROR) << "cmd "<<scmd;
      system(scmd);

      is_db_update_ = 0;
      return InitKvdb(kvdb_path_.c_str(), backup_path_.c_str());
    }
  }
  return true;
}

bool KvdbSqlite::CheckDBRecover() {
  LOG(L_WARNING) << "to check kvdb and recover ...";
  if (CheckFileExsits(kvdb_path_.c_str()) == false) {
    return false;
  }

  bool kvdb_error = Sqlite3CheckError(kvdb_path_);

  LOG(L_ERROR) << "check database have error." << kvdb_error;
  if (kvdb_error) {
    char scmd[1024] = { 0 };
    if (CheckFileExsits(kvdb_bak_path_.c_str()) == false) {
      LOG_ERROR("there is no %s, then remove the error %s.",
                kvdb_bak_path_.c_str(),
                kvdb_path_.c_str());
      remove(kvdb_path_.c_str());
      return false;
    }

    bool kvdb_back_error = Sqlite3CheckError(kvdb_bak_path_);
    if (kvdb_back_error == false) {
      // TODO 从备份中恢复数据库
      memset(scmd, 0, 1024);
      LOG(L_ERROR) << "recover from backup database.";
#ifdef _WIN32
      sprintf(scmd, "copy /Y %s %s",
#else
      sprintf(scmd, "cp -arf %s %s;sync",
#endif
              kvdb_bak_path_.c_str(),
              kvdb_path_.c_str());

      LOG(L_ERROR) << "cmd " << scmd;
      system(scmd);
    } else {
      // TODO 备份数据库也有问题,删除两个数据库
      LOG_ERROR("%s is error database, then remove all db file.",
                kvdb_bak_path_.c_str());
      remove(kvdb_path_.c_str());
      remove(kvdb_bak_path_.c_str());
      return false;
    }
  }
  return true;
}

void KvdbSqlite::ConfigGlobalKVDB(int soft_limit_heap_size,
                                  int malloc_heap_size) {
  int max_use_heap_size = 0;
  if (soft_limit_heap_size < 0) {
    // default 128k
    max_use_heap_size = 128 * 1024;
  } else {
    if (malloc_heap_size > soft_limit_heap_size) {
      malloc_heap_size = soft_limit_heap_size;
    }
    // init sofe heap memory limit  soft_heap_size(k)
    max_use_heap_size = soft_limit_heap_size * 1024;
  }
  // open sqlite memory check
  sqlite3_config(SQLITE_CONFIG_MEMSTATUS, true);
  // set heap size limit
  sqlite3_soft_heap_limit64(max_use_heap_size);

  // lookback cache. malloc 4k for sqlite
  sqlite3_config(SQLITE_CONFIG_LOOKASIDE, 128, 32);
  // page cache. malloc 64k for sqlite
  // psqlite_pagecache_buffer = (void*)(malloc(1024 * 64));
  sqlite3_config(SQLITE_CONFIG_PAGECACHE, 1024, 64);
  // temporary cache. malloc 8k for sqlite
  sqlite3_config(SQLITE_CONFIG_SCRATCH, 256, 32);

  // SQLITE_CONFIG_MEMSTATUS config sqlite use memory as psqlite_heap_buffer
  sqlite3_config(SQLITE_CONFIG_HEAP, malloc_heap_size * 1024, 128);

  /* total malloc (malloc_heap_size kb + 76 kb) memory for sqlite.
  ** default is 148 kb.*/
  is_init_memory_use_ = true;

  LOG(L_INFO) << "Config sqlite(kvdb) soft limite size: "
              << soft_limit_heap_size
              << " kb";
}

}

