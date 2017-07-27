/************************************************************************
*Author      : Sober.Peng 17-07-27
*Description :
************************************************************************/
#ifndef _CDATABASE_H_
#define _CDATABASE_H_

#include "vzbase/base/basictypes.h"
#include "vzbase/base/noncoypable.h"

#include "json/json.h"

class CDataBase : public vzbase::noncopyable {
 public:
  CDataBase();
  virtual ~CDataBase();

  bool Start(const char *db_path);
  void Stop();

 public:
  bool ReplaceTablePCount(const Json::Value *p_root);
  bool DeleteTablePCount(const Json::Value *p_root);
  bool SelectTablePCount(Json::Value *p_root);

 private:
  sqlite3      *db_instance_;
  sqlite3_stmt *replace_stmt_;
  sqlite3_stmt *delete_stmt_;
  sqlite3_stmt *select_stmt_;
};

#endif  // _CDATABASE_H_
