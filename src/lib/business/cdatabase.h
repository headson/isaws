/************************************************************************
*Author      : Sober.Peng 17-07-27
*Description :
************************************************************************/
#ifndef _CDATABASE_H_
#define _CDATABASE_H_

#include "vzbase/base/basictypes.h"
#include "vzbase/base/noncoypable.h"

#include "json/json.h"
#include "sqlite3/sqlite3.h"

#include <string>
#include <vector>

class CDataBase : public vzbase::noncopyable {
 public:
  CDataBase();
  virtual ~CDataBase();

  bool InitDB(const char *db_path);
  void UninitDB();

 public:
  bool CreatePCount();
  bool InitPCountStmt();
  void UninitPCountStmt();
  bool ReplacePCount(const Json::Value &jreq);
  bool ClearPCount(const Json::Value &jreq);
  bool RemovePCount(unsigned int some_days_ago);
  bool SelectLastPCount(std::string *stm, int *posi_num, int *nega_num);
  bool SelectPCount(Json::Value &jresp, const Json::Value &jreq);

 protected:
  bool CheckFileExsits(const char *db_path);

 private:
  sqlite3      *db_instance_;
  struct {
    sqlite3_stmt *replace_stmt_;
    sqlite3_stmt *delete_stmt_;
    //sqlite3_stmt *select_stmt_;

    int           last_positive_number;
    int           last_negative_number;
  } pcount_;
};

#endif  // _CDATABASE_H_
