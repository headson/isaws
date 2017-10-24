/************************************************************************/
/* Author      : SoberPeng 2017-07-28
/* Description :
/************************************************************************/
#include "cdatabase.h"

#include <string.h>

#include "vzbase/helper/stdafx.h"

#include "platform/basedefines.h"
#include "vzbase/base/vmessage.h"
#include "vzbase/base/timeutils.h"

static const char PCOUNT_SQL[] = "CREATE TABLE [pcount] ("
                                 " [ident_timet] INTEGER NOT NULL ON CONFLICT REPLACE,"
                                 " [insert_time] DATETIME NOT NULL ON CONFLICT REPLACE,"
                                 " [positive_number] INTEGER NOT NULL ON CONFLICT REPLACE, "
                                 " [positive_add_num] INTEGER NOT NULL ON CONFLICT REPLACE, "
                                 " [negative_number] INTEGER NOT NULL ON CONFLICT REPLACE, "
                                 " [negative_add_num] INTEGER NOT NULL ON CONFLICT REPLACE, "
                                 " [main_srv_send_flag]  INTEGER NOT NULL ON CONFLICT REPLACE, "
                                 " [minor_srv_send_flag] INTEGER NOT NULL ON CONFLICT REPLACE, "
                                 " CONSTRAINT [] PRIMARY KEY ([ident_timet]) ON CONFLICT REPLACE);";

static const char REPLACE_STMT_SQL[] = "REPLACE INTO "
                                       " pcount(ident_timet, insert_time, "
                                       " positive_number, positive_add_num, negative_number, negative_add_num, "
                                       " main_srv_send_flag, minor_srv_send_flag) "
                                       " VALUES (?, datetime(CURRENT_TIMESTAMP,'localtime'), ?, ?, ?, ?, 0, 0);";

static const char CLEAR_STMT_SQL[] = "DELETE FROM pcount";

static const char DELETE_STMT_SQL[] = "DELETE FROM pcount WHERE ident_timet < %d";

static const char SELECT_STMT_SQL_ALL_LAST_ONE[] = "SELECT insert_time, "
    " positive_number, negative_number "
    " FROM pcount ORDER BY ident_timet DESC LIMIT 0, 1";

#define  SELECT_STMT_SQL_NOT_SEND "SELECT ident_timet, positive_number, negative_number " \
  " main_srv_send_flag, minor_srv_send_flag "                   \
  " FROM pcount WHERE main_srv_send_flag == 0 OR minor_srv_send_flag == 0"  \
  " ORDER BY ident_timet DESC LIMIT 0, 1"

#define  SELECT_STMT_SQL_FIRST "SELECT positive_number, positive_add_num, "\
  " negative_number, negative_add_num " \
  " FROM pcount WHERE %d <= ident_timet AND ident_timet < %d " \
  " ORDER BY ident_timet DESC LIMIT 0, 1"

#define  SELECT_STMT_SQL_LAST "SELECT positive_number, positive_add_num, "\
  " negative_number, negative_add_num " \
  " FROM pcount WHERE %d <= ident_timet AND ident_timet < %d " \
  " ORDER BY ident_timet ASC LIMIT 0, 1"

bool CDataBase::CreatePCount() {
  char *sqlite_error_msg = NULL;
  int res = sqlite3_exec(db_instance_, PCOUNT_SQL, NULL, 0, &sqlite_error_msg);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to create exec this command" << PCOUNT_SQL;
    sqlite3_free(sqlite_error_msg);
    return false;
  }
  return true;
}

bool CDataBase::InitPCountStmt() {
  int res = 0;
  res = sqlite3_prepare_v2(db_instance_,
                           REPLACE_STMT_SQL,
                           strlen(REPLACE_STMT_SQL),
                           &pcount_.replace_stmt_,
                           0);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << REPLACE_STMT_SQL ;
    return false;
  }

  res = sqlite3_prepare_v2(db_instance_,
                           CLEAR_STMT_SQL,
                           strlen(CLEAR_STMT_SQL),
                           &pcount_.delete_stmt_,
                           0);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << CLEAR_STMT_SQL;
    return false;
  }

  //res = sqlite3_prepare_v2(db_instance_,
  //                         SELECT_STMT_SQL,
  //                         strlen(SELECT_STMT_SQL),
  //                         &pcount_.select_stmt_,
  //                         0);
  //if (res != SQLITE_OK) {
  //  LOG(L_ERROR) << SELECT_STMT_SQL;
  //  return false;
  //}

  pcount_.last_positive_number = 0;
  pcount_.last_negative_number = 0;
  return true;
}

void CDataBase::UninitPCountStmt() {
  if (pcount_.replace_stmt_) {
    sqlite3_finalize(pcount_.replace_stmt_);
    pcount_.replace_stmt_ = NULL;
  }

  if (pcount_.delete_stmt_) {
    sqlite3_finalize(pcount_.delete_stmt_);
    pcount_.delete_stmt_ = NULL;
  }

  //if (pcount_.select_stmt_) {
  //  sqlite3_finalize(pcount_.select_stmt_);
  //  pcount_.select_stmt_ = NULL;
  //}
}

bool CDataBase::ReplacePCount(const Json::Value &jreq) {
  int res = 0;
#if 1
  res = sqlite3_reset(pcount_.replace_stmt_);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to reset replace_stmt_*";
    return false;
  }

  res = sqlite3_bind_int64(pcount_.replace_stmt_, 1,
                           jreq[MSG_BODY][ALG_EVT_OUT_TIMET].asInt64());
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to call sqlite3_bind_*";
    return false;
  }

  int posi_num = jreq[MSG_BODY][ALG_POSITIVE_NUMBER].asInt();
  res = sqlite3_bind_int(pcount_.replace_stmt_, 2, posi_num);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to call sqlite3_bind_*";
    return false;
  }

  res = sqlite3_bind_int(pcount_.replace_stmt_, 3,
                         posi_num - pcount_.last_positive_number);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to call sqlite3_bind_*";
    return false;
  }
  pcount_.last_positive_number = posi_num;

  int nega_num = jreq[MSG_BODY][ALG_NEGATIVE_NUMBER].asInt();
  res = sqlite3_bind_int(pcount_.replace_stmt_, 4, nega_num);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to call sqlite3_bind_*";
    return false;
  }

  res = sqlite3_bind_int(pcount_.replace_stmt_, 5,
                         nega_num - pcount_.last_negative_number);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to call sqlite3_bind_*";
    return false;
  }
  pcount_.last_negative_number = nega_num;
#endif

  res = sqlite3_step(pcount_.replace_stmt_);
  if (res != SQLITE_DONE) {
    LOG(L_ERROR) << "Failure to call sqlite3_step"
                 << sqlite3_errstr(res);
    return false;
  }
  LOG(L_INFO) << "Replace pcount " << res;
  return true;
}

bool CDataBase::RemovePCount(unsigned int some_days_ago) {
  char *sqlite_error_msg = 0;
  char sql[1024] = {0};
  snprintf(sql, 1023, DELETE_STMT_SQL, time(NULL) - 90*24*60*60);
  int res = sqlite3_exec(db_instance_, sql, NULL, 0, &sqlite_error_msg);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to create exec this command" << PCOUNT_SQL;
    sqlite3_free(sqlite_error_msg);
    return false;
  }
  return true;
}

bool CDataBase::ClearPCount(const Json::Value &jreq) {
  int res = -1;
  res = sqlite3_step(pcount_.delete_stmt_);
  if (res != SQLITE_DONE) {
    LOG(L_ERROR) << "Failure to call sqlite3_step";
    return false;
  }
  LOG(L_INFO) << "Delete pcount " << res;
  return true;
}

bool CDataBase::SelectLastPCount(std::string *tm) {
  int res;
  sqlite3_stmt *stmt;
  char sql[1024] = {0};

  res = sqlite3_prepare_v2(db_instance_,
                           SELECT_STMT_SQL_ALL_LAST_ONE,
                           strlen(SELECT_STMT_SQL_ALL_LAST_ONE),
                           &stmt, 0);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "failed sqlite3_prepare_v2 " << sql;
    sqlite3_exec(db_instance_, "COMMIT", 0, 0, NULL);
    return false;
  }

  res = sqlite3_step(stmt);
  if (res == SQLITE_ROW) {
    *tm       = (char*)sqlite3_column_text(stmt, 0);
    pcount_.last_positive_number = sqlite3_column_int(stmt, 1);
    pcount_.last_negative_number = sqlite3_column_int(stmt, 2);
  }
  sqlite3_finalize(stmt);
  return res == SQLITE_ROW;
}

static bool GetNum(sqlite3 *db, const char *fmt,
                   unsigned int nbng, unsigned int nend,
                   int *positive_number, int *positive_add_num,
                   int *negative_number, int *negative_add_num) {
  int res;
  sqlite3_stmt *stmt;
  char sql[1024] = {0};

  snprintf(sql, 1023, fmt, nbng, nend);
  res = sqlite3_prepare_v2(db, sql, strlen(sql), &stmt, 0);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "failed sqlite3_prepare_v2 " << sql;
    sqlite3_exec(db, "COMMIT", 0, 0, NULL);
    return false;
  }

  res = sqlite3_step(stmt);
  if (res == SQLITE_ROW) {
    *positive_number  = sqlite3_column_int(stmt, 0);
    *positive_add_num = sqlite3_column_int(stmt, 1);
    *negative_number  = sqlite3_column_int(stmt, 2);
    *negative_add_num = sqlite3_column_int(stmt, 3);
  }
  sqlite3_finalize(stmt);
  return true;
}

static void CreateNumber(Json::Value &jret, sqlite3 *db,
                         int nnum,
                         unsigned int nbng, unsigned int nend) {
  int positive_number_first = 0, positive_number_last = 0;
  int positive_add_num_first = 0, positive_add_num_last = 0;
  int negative_number_first = 0, negative_number_last = 0;
  int negative_add_num_first = 0, negative_add_num_last = 0;

  bool res = false;

  // fisrt
  res = GetNum(db, SELECT_STMT_SQL_FIRST, nbng, nend,
               &positive_number_first, &positive_add_num_first,
               &negative_number_first, &negative_add_num_first);
  LOG(L_INFO) <<" number " << nnum
              << " \npositive_number_first "<<positive_number_first
              << " positive_add_num_first " << positive_add_num_first
              << " \nnegative_number_first " << negative_number_first
              << " negative_add_num_first " << negative_add_num_first;
  // last
  res = GetNum(db, SELECT_STMT_SQL_LAST, nbng, nend,
               &positive_number_last, &positive_add_num_last,
               &negative_number_last, &negative_add_num_last);
  if (positive_number_first == positive_number_last &&
      negative_number_first == negative_number_last) {
    positive_number_last = 0;
    positive_add_num_last = positive_add_num_first;
    negative_number_last = 0;
    negative_add_num_last = negative_add_num_first;
  }
  LOG(L_INFO) <<" number " << nnum
              << " \npositive_number_last " << positive_number_last
              << " positive_add_num_last " << positive_add_num_last
              << " \nnegative_number_last " << negative_number_last
              << " negative_add_num_last " << negative_add_num_last;

  Json::Value jone;
  jone["number"] = nnum;
  jone["positive"] = positive_number_first - positive_number_last + positive_add_num_last;
  jone["negative"] = negative_number_first - negative_number_last + negative_add_num_last;
  jret.append(jone);
}

static bool SelectPCountOfDay(Json::Value &jresp, sqlite3 *db,
                              int year, int month, int day) {
  sqlite3_stmt *stmt = NULL;
  char sql[1024] = {0};
  unsigned int nbng = 0, nend = 0;

  Json::Value jret(Json::arrayValue);
  sqlite3_exec(db, "BEGIN", 0, 0, NULL);
  for (int hour = 0; hour < 24; hour++) {
    nbng = vzbase::ToTimet(year, month, day, hour, 0, 0);
    nend = vzbase::ToTimet(year, month, day, hour, 59, 59);

    CreateNumber(jret, db, hour, nbng, nend);
  }
  sqlite3_exec(db, "COMMIT", 0, 0, NULL);

  jresp[MSG_BODY]["pcount"] = jret;
  return true;
}

static bool SelectPCountOfMonth(Json::Value &jresp, sqlite3 *db,
                                int year, int month) {
  sqlite3_stmt *stmt = NULL;
  char sql[1024] = {0};
  unsigned int nbng = 0, nend = 0;
  const int day_count = vzbase::GetMonthOfDays(year, month);

  Json::Value jret(Json::arrayValue);
  sqlite3_exec(db, "BEGIN", 0, 0, NULL);
  for (int day = 1; day <= day_count; day++) {
    nbng = vzbase::ToTimet(year, month, day, 0, 0, 0);
    nend = vzbase::ToTimet(year, month, day, 23, 59, 59);

    CreateNumber(jret, db, day, nbng, nend);
  }
  sqlite3_exec(db, "COMMIT", 0, 0, NULL);

  jresp[MSG_BODY]["pcount"] = jret;
  return true;
}


static bool SelectPCountOfYear(Json::Value &jresp, sqlite3 *db,
                               int year) {
  sqlite3_stmt *stmt = NULL;
  char sql[1024] = {0};
  unsigned int nbng = 0, nend = 0;
  Json::Value jret(Json::arrayValue);
  sqlite3_exec(db, "BEGIN", 0, 0, NULL);
  for (int month = 1; month <= 12; month++) {
    nbng = vzbase::ToTimet(year, month, 1, 0, 0, 0);

    const int days = vzbase::GetMonthOfDays(year, month);
    nend = vzbase::ToTimet(year, month, days, 23, 59, 59);

    CreateNumber(jret, db, month, nbng, nend);
  }
  sqlite3_exec(db, "COMMIT", 0, 0, NULL);

  jresp[MSG_BODY]["pcount"] = jret;
  return true;
}

bool CDataBase::SelectPCount(Json::Value &jresp, const Json::Value &jreq) {
  if (!jreq.isMember(MSG_BODY) ||
      !jreq[MSG_BODY].isMember("unit") ||
      !jreq[MSG_BODY].isMember("year") ||
      !jreq[MSG_BODY].isMember("month") ||
      !jreq[MSG_BODY].isMember("day")) {
    LOG(L_ERROR) << "request's json format is error.";
    return false;
  }

  std::string sunit = jreq[MSG_BODY]["unit"].asString();
  int nyear = jreq[MSG_BODY]["year"].asInt();
  int nmonth = jreq[MSG_BODY]["month"].asInt();
  int nday = jreq[MSG_BODY]["day"].asInt();

  jresp[MSG_BODY]["unit"]  = sunit;
  jresp[MSG_BODY]["year"]  = nyear;
  jresp[MSG_BODY]["month"] = nmonth;
  jresp[MSG_BODY]["day"]   = nday;

  if (sunit == "day") {
    return SelectPCountOfDay(jresp, db_instance_, nyear, nmonth, nday);
  } else if (sunit == "month") {
    return SelectPCountOfMonth(jresp, db_instance_, nyear, nmonth);
  } else if (sunit == "year") {
    return SelectPCountOfYear(jresp, db_instance_, nyear);
  }
  LOG(L_ERROR) << "request's json unit is error.";
  return false;
}


