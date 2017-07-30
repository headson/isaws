/************************************************************************/
/* Author      : SoberPeng 2017-07-28
/* Description :
/************************************************************************/
#include "cdatabase.h"
#include "vzbase/helper/stdafx.h"

#include "alg/basedefine.h"
#include "vzbase/base/vmessage.h"
#include "vzbase/base/timeutils.h"

static const char PCOUNT_SQL[] = "CREATE TABLE [pcount] ("
                                 " [ident_timet] INTEGER NOT NULL ON CONFLICT REPLACE,"
                                 " [positive_number] INTEGER NOT NULL ON CONFLICT REPLACE, "
                                 " [negative_number] INTEGER NOT NULL ON CONFLICT REPLACE, "
                                 " [main_srv_send_flag]  INTEGER NOT NULL ON CONFLICT REPLACE, "
                                 " [minor_srv_send_flag] INTEGER NOT NULL ON CONFLICT REPLACE, "
                                 " CONSTRAINT [] PRIMARY KEY ([ident_timet]) ON CONFLICT REPLACE);";

static const char REPLACE_STMT_SQL[] = "REPLACE INTO "
                                       " pcount(ident_timet, positive_number, negative_number) "
                                       " VALUES (?, ?, ?);";

static const char DELETE_STMT_SQL[] = "DELETE FROM pcount";

#define  SELECT_STMT_SQL_NOT_SEND "SELECT ident_timet, positive_number, negative_number " \
  " main_srv_send_flag, minor_srv_send_flag "                   \
  " FROM pcount WHERE main_srv_send_flag == 0 OR minor_srv_send_flag == 0"  \
  " ORDER BY ident_timet DESC LIMIT 0, 1"

#define  SELECT_STMT_SQL_FIRST "SELECT positive_number, negative_number " \
  " FROM pcount WHERE %d <= ident_timet AND ident_timet < %d " \
  " ORDER BY ident_timet DESC LIMIT 0, 1"

#define  SELECT_STMT_SQL_LAST "SELECT positive_number, negative_number " \
  " FROM pcount WHERE %d <= ident_timet AND ident_timet < %d " \
  " ORDER BY ident_timet ASC LIMIT 0, 1"

bool CDataBase::CreatePCount() {
  char *sqlite_error_msg = 0;
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
    LOG(L_ERROR) << REPLACE_STMT_SQL;
    return false;
  }

  res = sqlite3_prepare_v2(db_instance_,
                           DELETE_STMT_SQL,
                           strlen(DELETE_STMT_SQL),
                           &pcount_.delete_stmt_,
                           0);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << DELETE_STMT_SQL;
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

  if (!jreq.isMember(MSG_BODY)) {
    LOG(L_ERROR) << "param is null";
    return false;
  }
  if (!jreq[MSG_BODY].isMember(ALG_EVT_OUT_TIME_MS)
      || !jreq[MSG_BODY][ALG_EVT_OUT_TIME_MS].isInt()) {
    LOG(L_ERROR) << "param is null";
    return false;
  }
  if (!jreq[MSG_BODY].isMember(ALG_POSITIVE_NUMBER)
      || !jreq[MSG_BODY][ALG_POSITIVE_NUMBER].isInt()) {
    LOG(L_ERROR) << "param is null";
    return false;
  }
  if (!jreq[MSG_BODY].isMember(ALG_NEGATIVE_NUMBER)
      || !jreq[MSG_BODY][ALG_NEGATIVE_NUMBER].isInt()) {
    LOG(L_ERROR) << "param is null";
    return false;
  }

#if 1
  res = sqlite3_reset(pcount_.replace_stmt_);
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to reset replace_stmt_*";
    return false;
  }

  res = sqlite3_bind_int(pcount_.replace_stmt_, 1,
                         jreq[MSG_BODY][ALG_EVT_OUT_TIME_MS].asInt());
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to call sqlite3_bind_*";
    return false;
  }

  res = sqlite3_bind_int(pcount_.replace_stmt_, 2,
                         jreq[MSG_BODY][ALG_POSITIVE_NUMBER].asInt());
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to call sqlite3_bind_*";
    return false;
  }

  res = sqlite3_bind_int(pcount_.replace_stmt_, 3,
                         jreq[MSG_BODY][ALG_NEGATIVE_NUMBER].asInt());
  if (res != SQLITE_OK) {
    LOG(L_ERROR) << "Failure to call sqlite3_bind_*";
    return false;
  }
#endif

  res = sqlite3_step(pcount_.replace_stmt_);
  if (res != SQLITE_DONE) {
    LOG(L_ERROR) << "Failure to call sqlite3_step";
    return false;
  }
  LOG(L_INFO) << "Replace pcount " << res;
  return true;
}

bool CDataBase::DeletePCount(const Json::Value &jreq) {
  int res = -1;
  res = sqlite3_step(pcount_.delete_stmt_);
  if (res != SQLITE_DONE) {
    LOG(L_ERROR) << "Failure to call sqlite3_step";
    return false;
  }
  LOG(L_INFO) << "Delete pcount " << res;
  return true;
}

static bool GetNum(sqlite3 *db, const char *fmt,
                   unsigned int nbng, unsigned int nend,
                   int *positive_number, int *negative_number) {
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
    *positive_number = sqlite3_column_int(stmt, 0);
    *negative_number = sqlite3_column_int(stmt, 1);
  }
  sqlite3_finalize(stmt);
  return true;
}

static bool SelectPCountOfDay(Json::Value &jresp, sqlite3 *db,
                              int year, int month, int day) {
  bool res;
  sqlite3_stmt *stmt;
  char sql[1024] = {0};
  unsigned int nbng = 0, nend = 0;

  Json::Value jret(Json::arrayValue);
  sqlite3_exec(db, "BEGIN", 0, 0, NULL);
  for (int hour = 0; hour < 24; hour++) {
    nbng = vzbase::ToTimet(year, month, day, hour, 0, 0);
    nbng = vzbase::ToTimet(year, month, day, hour, 59, 59);

    int positive_number_first = 0, positive_number_last = 0;
    int negative_number_first = 0, negative_number_last = 0;

    // fisrt
    res = GetNum(db, SELECT_STMT_SQL_FIRST, nbng, nend,
                 &positive_number_first, &negative_number_first);

    // last
    res = GetNum(db, SELECT_STMT_SQL_LAST, nbng, nend,
                 &positive_number_last, &negative_number_last);

    Json::Value jone;
    jone["number"]   = hour;
    jone["positive"] = positive_number_first - positive_number_last;
    jone["negative"] = positive_number_last - positive_number_last;
    jret.append(jone);
  }
  sqlite3_exec(db, "COMMIT", 0, 0, NULL);

  jresp[MSG_BODY]["pcount"] = jret;
  return true;
}

static bool SelectPCountOfMonth(Json::Value &jresp, sqlite3 *db,
                                int year, int month) {
  int res;
  sqlite3_stmt *stmt;
  char sql[1024] = {0};
  unsigned int nbng = 0, nend = 0;
  const int day_count = vzbase::GetMonthOfDays(year, month);

  Json::Value jret(Json::arrayValue);
  sqlite3_exec(db, "BEGIN", 0, 0, NULL);
  for (int day = 1; day <= day_count; day++) {
    nbng = vzbase::ToTimet(year, month, day, 0, 0, 0);
    nbng = vzbase::ToTimet(year, month, day, 23, 59, 59);

    int positive_number_first = 0, positive_number_last = 0;
    int negative_number_first = 0, negative_number_last = 0;

    // fisrt
    res = GetNum(db, SELECT_STMT_SQL_FIRST, nbng, nend,
                 &positive_number_first, &negative_number_first);

    // last
    res = GetNum(db, SELECT_STMT_SQL_LAST, nbng, nend,
                 &positive_number_last, &negative_number_last);

    Json::Value jone;
    jone["number"]   = day;
    jone["positive"] = positive_number_first - positive_number_last;
    jone["negative"] = positive_number_last - positive_number_last;
    jret.append(jone);
  }
  sqlite3_exec(db, "COMMIT", 0, 0, NULL);

  jresp[MSG_BODY]["pcount"] = jret;
  return true;
}


static bool SelectPCountOfYear(Json::Value &jresp, sqlite3 *db,
                               int year) {
  int res;
  sqlite3_stmt *stmt;
  char sql[1024] = {0};
  unsigned int nbng = 0, nend = 0;
  Json::Value jret(Json::arrayValue);
  sqlite3_exec(db, "BEGIN", 0, 0, NULL);
  for (int month = 1; month <= 12; month++) {
    nbng = vzbase::ToTimet(year, month, 1, 0, 0, 0);
    const int days = vzbase::GetMonthOfDays(year, month);
    nbng = vzbase::ToTimet(year, month, days, 23, 59, 59);

    int positive_number_first = 0, positive_number_last = 0;
    int negative_number_first = 0, negative_number_last = 0;

    // fisrt
    res = GetNum(db, SELECT_STMT_SQL_FIRST, nbng, nend,
                 &positive_number_first, &negative_number_first);

    // last
    res = GetNum(db, SELECT_STMT_SQL_LAST, nbng, nend,
                 &positive_number_last, &negative_number_last);

    Json::Value jone;
    jone["number"] = month;
    jone["positive"] = positive_number_first - positive_number_last;
    jone["negative"] = positive_number_last - positive_number_last;
    jret.append(jone);
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


