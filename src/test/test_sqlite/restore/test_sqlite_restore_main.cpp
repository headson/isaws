#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "sqlite3/sqlite3.h"

#include "vzbase/helper/stdafx.h"

bool CopyDatabase(sqlite3 *from, sqlite3* to) {
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

bool RestoreDatabase(sqlite3 *db_thiz, std::string backup_path) {
  if (backup_path.empty()) {
    return true;
  }
  LOG(L_WARNING) << "backup database to local ....";

  sqlite3 *db_backup = NULL;
  int res = sqlite3_open(backup_path.c_str(), &db_backup);
  if(res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to open the database " << backup_path;
    return false;
  }
  bool copy_res = CopyDatabase(db_backup, db_thiz);
  /* Close the database connection opened on database file zFilename
  ** and return the result of this function. */
  sqlite3_close(db_backup);
  LOG(L_ERROR)<<"restore database.";
  return copy_res;
}

int main(int argc, char* argv[]) {
  InitVzLogging(argc, argv);
#ifdef WIN32
  ShowVzLoggingAlways();
#endif
#ifdef _WIN32
  const char *db_path = "c:\\tools\\kvdb.db";
  const char *db_path_bak = "c:\\tools\\kvdb_bak.db";
#else
  const char *db_path = "/mnt/usr/kvdb1.db";
  const char *db_path_bak = "/mnt/usr/kvdb1_bak.db";
#endif

RETEST:
  //rename("c:\\tools\\kvdb_backup.db", db_path);
  sqlite3 *db_inst_ = NULL;
  int res = sqlite3_open(db_path, &db_inst_);
  if (res) {
    LOG(L_ERROR) << "Failure to open the database";
    sqlite3_close(db_inst_);
    return false;
  }

  res = sqlite3_exec(db_inst_, "PRAGMA synchronous = FULL;", 0, 0, 0);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "PRAGMA journal_mode = MEMORY;";
    return false;
  }

  //res = sqlite3_exec(db_inst_, "PRAGMA journal_mode = MEMORY;", 0, 0, 0);
  //if (res != SQLITE_OK) {
  //  LOG(L_ERROR) << "PRAGMA journal_mode = MEMORY;";
  //  return false;
  //}
  RestoreDatabase(db_inst_, db_path_bak);

  sqlite3_stmt *select_stmt_ = NULL;
  const char SELECT_STMT_SQL[] = "SELECT kvdb.value from kvdb where kvdb.key = ?";
  res = sqlite3_prepare_v2(db_inst_,
                           SELECT_STMT_SQL,
                           strlen(SELECT_STMT_SQL),
                           &select_stmt_,
                           0);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << SELECT_STMT_SQL;
    return false;
  }
  sqlite3_stmt *replace_stmt_ = NULL;
  const char REPLACE_STMT_SQL[] = "REPLACE INTO kvdb(key, value) VALUES (?, ?);";
  res = sqlite3_prepare_v2(db_inst_,
                           REPLACE_STMT_SQL,
                           strlen(REPLACE_STMT_SQL),
                           &replace_stmt_,
                           0);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << REPLACE_STMT_SQL;
    return false;
  }
  for (int i = 0; i < 10; i++) {
    std::string sval = "";
    char   skey[64] = {0};
    snprintf(skey, 63, "key-%d", rand() % ((unsigned int)-1));

    res = sqlite3_reset(select_stmt_);
    if (res != SQLITE_OK) {
      LOG(L_ERROR) << "Failure to reset select_stmt_*";
      return false;
    }
    res = sqlite3_bind_text(select_stmt_, 1,
                            skey,
                            strlen(skey),
                            SQLITE_STATIC);
    if (res != SQLITE_OK) {
      LOG(L_ERROR) << "Failure to call sqlite3_bind_*";
      return false;
    }
    res = sqlite3_step(select_stmt_);
    if (res == SQLITE_ROW) {
      const void *data = sqlite3_column_blob(select_stmt_, 0);
      int data_size = sqlite3_column_bytes(select_stmt_, 0);
      sval.append((char*)data, data_size);
      LOG_INFO("key %s, value %s.\n", skey, data);
    }

    //////////////////////////////////////////////////////////////////////////
    res = sqlite3_reset(replace_stmt_);
    if (res != SQLITE_OK) {
      LOG(L_ERROR) << "Failure to reset replace_stmt_*";
      return false;
    }
    res = sqlite3_bind_text(replace_stmt_, 1,
                            skey,
                            strlen(skey),
                            SQLITE_STATIC);
    if (res != SQLITE_OK) {
      LOG(L_ERROR) << "Failure to call sqlite3_bind_*";
      return false;
    }
    sval.append("hello");
    res = sqlite3_bind_blob(replace_stmt_, 2,
                            (const void *)sval.c_str(),
                            sval.size(),
                            SQLITE_STATIC);
    if (res != SQLITE_OK) {
      LOG(L_ERROR) << "Failure to call sqlite3_bind_*";
      return false;
    }
    res = sqlite3_step(replace_stmt_);
    if (res != SQLITE_DONE) {
      LOG(L_ERROR) << "Failure to call sqlite3_step";
      return false;
    }
  }
  sqlite3_finalize(select_stmt_);
  sqlite3_finalize(replace_stmt_);
  sqlite3_close(db_inst_);
  db_inst_ = NULL;

  goto RETEST;

  getchar();
  getchar();
  return 0;
}