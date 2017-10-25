/************************************************************************
*Author      : Sober.Peng 17-07-25
*Description : 利用popen实现system功能,规避system返回值判断问题
************************************************************************/
#ifndef _MYSYSTEM_H_
#define _MYSYSTEM_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>
#include <fstream>

#include <json/json.h>

#include "vzbase/base/vmessage.h"
#include "vzbase/helper/stdafx.h"

#ifdef __cplusplus
namespace vzbase {
extern "C" {
#endif

inline int my_system(const char * cmd) {
  if (cmd == NULL) {
    printf("my_system cmd is NULL!\n");
    return -1;
  }
  LOG(L_ERROR) << cmd;
  return system(cmd);

#ifdef _LINUX
  FILE *fp;
  if ((fp = popen(cmd, "r")) != NULL) {
    int res;
    char buf[1024];
    while(fgets(buf, sizeof(buf), fp)) {
      printf("%s", buf);
    }
    res = pclose(fp);

    if (WIFEXITED(res)) {
      printf("subprocess exited, exit code: %d\n", WEXITSTATUS(res));
      if (0 == WEXITSTATUS(res)) {
        // if command returning 0 means succeed
        printf("command succeed\n");
      } else {
        if (127 == WEXITSTATUS(res)) {
          printf("command not found\n");
          return WEXITSTATUS(res);
        } else {
          printf("command failed: %s\n", strerror(WEXITSTATUS(res)));
          return WEXITSTATUS(res);
        }
      }
    } else {
      printf("subprocess exit failed\n");
      return -1;
    }
    return res;
  }

  perror("popen");
  printf("popen error: %s error %s./n",
         cmd, strerror(errno));
#endif  // _LINUX
  return -1;
}

inline void get_hardware(std::string &hw, std::string &uuid) {
  static std::string shw = "";
  static std::string suid = "";

  if (shw.empty() || suid.empty()) {
    Json::Reader jr;
    Json::Value  jv;
    std::ifstream ifs;
    ifs.open(DEF_HARDWARE_FILE);
    if (ifs.is_open() && jr.parse(ifs, jv)) {
      shw = jv["hardware"].asString();
      suid = jv["dev_uuid"].asString();
    } else {
      shw = "1.0.0.1001707310";
      suid = "PC001170801220030";

      jv["hardware"] = shw;
      jv["dev_uuid"] = suid;

      FILE *file = fopen(DEF_HARDWARE_FILE, "wt+");
      if (file) {
        std::string ss = jv.toStyledString();

        fwrite(ss.c_str(), 1, ss.size(), file);
        fclose(file);
      }
    }
  }

  hw   = shw;
  uuid = suid;
}

inline void get_software(std::string &sw) {
  static std::string tsw = "";

  if (tsw.empty()) {
    std::ifstream ifs;
    ifs.open(DEF_SOFTWARE_FILE);
    if (ifs.is_open()) {
      ifs >> tsw;
    }
  }
  sw = tsw;
}

inline void file_remove(const char *filename) {
  char scmd[256] = {0};
  snprintf(scmd, 1023,
           "rm -rf /tmp/bak; rm %s",
           filename);

  LOG(L_INFO) << scmd;
  vzbase::my_system(scmd);
}

#ifdef __cplusplus
}
}  // namespace vzbase
#endif
#endif  // _MYSYSTEM_H_
