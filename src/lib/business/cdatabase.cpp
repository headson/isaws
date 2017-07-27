/************************************************************************
*Author      : Sober.Peng 17-07-27
*Description :
************************************************************************/
#include "cdatabase.h"
#include "vzbase/helper/stdafx.h"

CDataBase::CDataBase()
  : db_instance_(NULL)
  , replace_stmt_(NULL)
  , delete_stmt_(NULL)
  , select_stmt_(NULL) {
}

CDataBase::~CDataBase() {
}

bool CDataBase::Start(const char *db_path) {
  bool file_exist = CheckFileExsits(db_path);

  int res = sqlite3_open(db_path, &db_instance_);
  if (res) {
    LOG(L_ERROR) << "Failure to open the database";
    sqlite3_close(db_instance_);
    return false;
  }
  if (!file_exist) {
    if (!CreateTablePCount()) {
      return false;
    }
  }
  res = sqlite3_exec(db_instance_, "PRAGMA journal_mode = MEMORY;", 0, 0, 0);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "PRAGMA journal_mode = MEMORY;";
    return false;
  }
  return true;
}

void CDataBase::Stop() {

}

bool CDataBase::CheckFileExsits(const char *db_path) {
  FILE *fp = fopen(db_path, "rb");
  if (fp == NULL) {
    return false;
  }
  fclose(fp);
  return true;
}
