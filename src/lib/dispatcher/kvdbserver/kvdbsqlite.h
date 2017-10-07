#ifndef VZCONN_SERVER_KVDB_SQLITE_H_
#define VZCONN_SERVER_KVDB_SQLITE_H_

#include "vzbase/base/boost_settings.hpp"
#include "sqlite3/sqlite3.h"
#include <string>
#include <vector>

namespace kvdb {

class KvdbSqlite : public boost::noncopyable {
 public:
  KvdbSqlite();
  virtual ~KvdbSqlite();

  bool InitKvdb(const char *kvdb_path, const char *backup_path);
  void UinitKvdb();

  void RemoveDatabase(const char *kvdb_path);
  bool ReplaceKeyValue(const char *key, int key_size,
                       const char *value, int value_size);
  bool DeleteKeyValue(const char *key, int key_size);
  bool SelectKeyValue(const char *key, int key_size,
                      std::vector<char> &buffer);

  bool TransBegin();
  bool TransCommit();
  bool TransRollback();

  // Copy the current database to backup database
  bool BackupDatabase();
  // If the restore database is null, then delete all of the current data
  bool RestoreDatabase(std::string backup_path);

  inline std::string GetKvdbPath() {
    return kvdb_path_;
  }

  inline std::string GetBackupPath() {
    return backup_path_;
  }

  bool CheckDBBackup();
  bool CheckDBRecover();

  void ForceTransaction();

 public:
  /*
  @function: config sqlite soft max memory use and set sqlite
  page cache¡¢scrach cache¡¢lookback cache pre allocate
  size. sometimes what it work will bring diff result.
  @prameter1: soft_limit_heap_size(unit is kb) is the  max heap
  size we want to allocate to sqlite.default is 64.
  @prameter1: malloc_heap_size(unit is kb) is sqlite pre mallco
  from system. it must less than soft_limit_heap_size.
  */
  static void ConfigGlobalKVDB(int soft_limit_heap_size = 64,
                               int malloc_heap_size = 64);

 private:
  void CheckTransaction();

  bool InitStmt();

  bool CheckFileExsits(const char *kvdb_path);
  bool CopyDatabase(sqlite3 *from, sqlite3* to);
  bool InitBackupDatabase(const char *backup_path);

 private:
  static const unsigned int MAX_TRANSACTION_SIZE = 64;
  static const unsigned int SIZE_OF_HEADER       = 5;
  static bool   is_init_memory_use_;

  sqlite3      *db_instance_;
  sqlite3_stmt *replace_stmt_;
  sqlite3_stmt *delete_stmt_;
  sqlite3_stmt *select_stmt_;
  
  std::string   kvdb_path_;
  std::string   kvdb_bak_path_;
  std::string   backup_path_;

  unsigned int  is_db_update_;
  // unsigned int is_transaction_;
};

}

#endif //  VZCONN_SERVER_KVDB_SQLITE_H_

