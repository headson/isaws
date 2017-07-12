#include "vzbase/base/loggingsettings.h"
#include <stdio.h>
#include <stdlib.h>

namespace vzbase {

void PrintUsage() {
  printf("\n");
  printf("\n");
  printf("  usage:\n");
  printf("\n");
  printf(
    "        -e  <logtostderr>     Defualt value is 0 (google::GLOG_INFO)\n");
  printf(
    "        -m  <miniloglevel>    Default value is 2 (google::GLOG_ERROR)\n");
  printf(
    "        -v                    Verbose Mode\n");
  printf("\n");
  printf(
    "       [-h]                   print usage\n");
  printf("\n");
}

void InitLoggingSystem(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
  return ;
}

}  // namespace vzbase

