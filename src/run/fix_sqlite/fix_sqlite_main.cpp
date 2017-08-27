#include <stdio.h>
#include <string.h>

#ifndef WIN32
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#endif

#include "sqlite3/sqlite3.h"

extern "C" {
  extern void db_export(const char *db_path, const char *sql_path);
  extern void db_import(const char *db_path, const char *sql_path);
};

bool is_err_db = true;
int Sqlite3CB(void *user_data, int a, char** b, char** c) {
  printf("%s\n", *b);
  if (strncmp(*b, "ok", 2) == 0) {
    is_err_db = false;
  }
  return 0;
}

#ifndef _WIN32
#define MAX_BUFFER_SIZE   32 * 1024
#define DEF_LOG_FILEPATH  "/mnt/log/fix_sqlite.log"

int log_size = 0;
static char log_buffer[MAX_BUFFER_SIZE];
void InitLog() {
  FILE * fp = fopen(DEF_LOG_FILEPATH, "rb");
  if (fp == NULL) {
    printf("Failure to getting phy cehck log\n");
    return;
  }

  while (!feof(fp)) {
    int read_size = fread(log_buffer + log_size, sizeof(char), 256, fp);
    log_size += read_size;
    printf("log_size = %d\n", log_size);
  }
  //printf("================================\n");
  //printf("%s\n", log_buffer);
  //printf("================================\n");
  fclose(fp);
}

void LoggingBufferCehck(int size) {
  if ((MAX_BUFFER_SIZE - log_size) < size) {
    int move_size = size - (MAX_BUFFER_SIZE - log_size);
    memmove(log_buffer, log_buffer + move_size, MAX_BUFFER_SIZE - move_size);
    log_size = MAX_BUFFER_SIZE - size;
    printf("%d\n%s\n", log_size, log_buffer);
  }
}

void WriteLog(const char *str_msg) {
  FILE * fp = fopen(DEF_LOG_FILEPATH, "wb");
  if (fp == NULL) {
    printf("Failure to getting phy cehck log\n");
    return;
  }
  struct timeval tv;
  gettimeofday(&tv, NULL);
  time_t tt = tv.tv_sec;
  struct tm *wtm = localtime(&tt);
  LoggingBufferCehck(256);
  int msg_size = snprintf(log_buffer + log_size,
                          256,
                          "[%04d-%02d-%02d %02d:%02d:%02d.%03d] %s\n",
                          wtm->tm_year + 1900,
                          wtm->tm_mon + 1,
                          wtm->tm_mday,
                          wtm->tm_hour,
                          wtm->tm_min,
                          wtm->tm_sec,
                          (int)(tv.tv_usec / 1000),
                          str_msg);

  printf("%d %s\n", msg_size, log_buffer + log_size);
  log_size += msg_size;

  fwrite(log_buffer, sizeof(char), log_size, fp);

  printf("================================\n");
  printf("%s\n", log_buffer);
  printf("================================\n");
  fclose(fp);
}
#else
void InitLog() {}
void WriteLog(const char *str_msg) {}

#define usleep
#define snprintf _snprintf
#endif

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("applet db_filename\n");
    return -1;
  }
  const char *db_file = argv[1];

  sqlite3 *db_instance_ = NULL;
  int res = sqlite3_open(db_file, &db_instance_);
  if (res) {
    printf("Failure to open the database");
    sqlite3_close(db_instance_);
    ::remove(db_file);
    return -1;
  }

  char *error_msg = NULL;
  res = sqlite3_exec(db_instance_, "PRAGMA integrity_check;",
                     Sqlite3CB,
                     NULL,
                     &error_msg);
  if (res != SQLITE_OK) {
    sqlite3_close(db_instance_);
    printf("\nPRAGMA integrity_check;\n");
    ::remove(db_file);
    return -2;
  }

  InitLog();
  if (is_err_db) {
    sqlite3_close(db_instance_);

#ifdef WIN32
    const char *sql_path = "C:/tools/db.sql";
#else
    const char *sql_path = "/tmp/db.sql";
#endif

    // dump sql file
    printf("dump db file %s.\n", db_file);
    db_export(db_file, sql_path);

    ::remove(db_file);
    usleep(1000*1000);

    printf("read sql to db %s.\n", db_file);
    db_import(db_file, sql_path);

    ::remove(sql_path);
    printf("%s[%d].\n", __FILE__, __LINE__);
    char slog[64] = {0};
    snprintf(slog, 63, "fix the %s sqlite database.\n", db_file);

    WriteLog(slog);
  } else {
    sqlite3_close(db_instance_);
    printf("%s[%d].\n", __FILE__, __LINE__);
    WriteLog("have checked database, but not find a error.\n");
  }
  return 0;
}

