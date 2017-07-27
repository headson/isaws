/************************************************************************/
/* Author      : SoberPeng 2017-07-28
/* Description :
/************************************************************************/
#include "cdatabase.h"
#include "vzbase/helper/stdafx.h"

const char PCOUNT_SQL[] = "CREATE TABLE [pcount] ("
                          " [ident_timet] INTEGER NOT NULL ON CONFLICT REPLACE,"
                          " [positive_number] INTEGER NOT NULL ON CONFLICT REPLACE, "
                          " [negative_number] INTEGER NOT NULL ON CONFLICT REPLACE, "
                          " CONSTRAINT [] PRIMARY KEY ([ident_timet]) ON CONFLICT REPLACE);";

const char REPLACE_STMT_SQL[] = "REPLACE INTO "
                                " pcount(ident_timet, positive_number, negative_number) "
                                " VALUES (?, ?, ?);";

const char DELETE_STMT_SQL[] = "DELETE FROM pcount "
                               " WHERE ? <= ident_timet AND ident_timet < ?;";

const char SELECT_STMT_SQL[] = "SELECT ident_timet, positive_number, negative_number "
                               " FROM pcount WHERE ? <= ident_timet AND ident_timet < ?";


bool CDataBase::CreateTablePCount() {
  char *sqlite_error_msg = 0;
  int res = sqlite3_exec(db_instance_, PCOUNT_SQL, NULL, 0, &sqlite_error_msg);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to create exec this command" << PCOUNT_SQL;
    sqlite3_free(sqlite_error_msg);
    return false;
  }
  return true;
}

bool CDataBase::ReplaceTablePCount(const Json::Value *jroot) {
  return true;
}

bool CDataBase::DeleteTablePCount(const Json::Value *jroot) {
  return true;
}

bool CDataBase::SelectTablePCount(Json::Value *jroot) {
  return true;
}


