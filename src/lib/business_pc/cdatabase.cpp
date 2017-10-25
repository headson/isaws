/************************************************************************
*Author      : Sober.Peng 17-07-27
*Description :
************************************************************************/
#include "cdatabase.h"
#include "vzbase/helper/stdafx.h"

CDataBase::CDataBase()
  : db_instance_(NULL) {
}

CDataBase::~CDataBase() {
  UninitDB();
}

bool CDataBase::InitDB(const char *db_path) {
  bool file_exist = CheckFileExsits(db_path);

  /// 
  int res = sqlite3_open(db_path, &db_instance_);
  if (res) {
    LOG(L_ERROR) << "Failure to open the database";
    sqlite3_close(db_instance_);
    return false;
  }
  if (!file_exist) {
    if (!CreatePCount()) {
      return false;
    }
  }

  /// 
  res = sqlite3_exec(db_instance_, "PRAGMA journal_mode = MEMORY;", 0, 0, 0);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "PRAGMA journal_mode = MEMORY;";
    return false;
  }

  /// 
  bool bres = InitPCountStmt();
  if (!bres) {
    LOG(L_ERROR) << "init pcount stmt failed.";
    return false;
  }
  
  return true;
}

void CDataBase::UninitDB() {
  if (db_instance_) {
    sqlite3_close(db_instance_);
    db_instance_ = NULL;
  }
}

bool CDataBase::CheckFileExsits(const char *db_path) {
  FILE *fp = fopen(db_path, "rb");
  if (fp == NULL) {
    return false;
  }
  fclose(fp);
  return true;
}
