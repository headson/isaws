/************************************************************************/
/* Author      : SoberPeng 2017-08-02
/* Description :
/************************************************************************/
#include "cmodulecontrol.h"

void CModuleMonitor::ReStartModule() {
  //vzbase::my_system("killall -9 feeddog;sleep 1;"
  //                  "chmod +x feeddog;"
  //                  "LD_LIBRARY_PATH=. ./feeddog &");

  //vzbase::my_system("killall -9 dispatcher_server;sleep 1;"
  //                  "chmod +x dispatcher_server;"
  //                  "LD_LIBRARY_PATH=. ./dispatcher_server &");
  //vzbase::my_system("killall -9 platform_app;sleep 1;"
  //                  "chmod +x platform_app;"
  //                  "LD_LIBRARY_PATH=. ./platform_app &; sleep 2");  
  //vzbase::my_system("killall -9 alg_app;sleep 1;"
  //                  "chmod +x alg_app;"
  //                  "LD_LIBRARY_PATH=. ./alg_app &");
  //vzbase::my_system("killall -9 business_app;sleep 1;"
  //                  "chmod +x business_app;"
  //                  "LD_LIBRARY_PATH=. ./business_app &");
  //vzbase::my_system("killall -9 web_server_app;sleep 1;"
  //                  "chmod +x web_server_app;"
  //                  "LD_LIBRARY_PATH=. ./web_server_app &");

  //vzbase::my_system("killall -9 watchdog;sleep 3;"
  //                  "chmod +x watchdog;"
  //                  "LD_LIBRARY_PATH=. ./watchdog &");
}

void CModuleMonitor::StopSomeModule() {
  //vzbase::my_system("killall -9 feeddog;sleep 1;"
  //                  "chmod +x feeddog;"
  //                  "LD_LIBRARY_PATH=. ./feeddog &");
}

