/************************************************************************/
/* Author      : SoberPeng 2017-08-02
/* Description :
/************************************************************************/
#include "cmodulecontrol.h"
#include "vzbase/base/mysystem.h"

void CModuleMonitor::ReStartModule() {
  vzbase::my_system("killall feeddog;"
                    "chmod +x feeddog;"
                    "LD_LIBRARY_PATH=. ./feeddog &");

  vzbase::my_system("killall dispatcher_server;"
                    "chmod +x dispatcher_server;"
                    "LD_LIBRARY_PATH=. ./dispatcher_server &");
  vzbase::my_system("killall platform_app;"
                    "chmod +x platform_app;"
                    "LD_LIBRARY_PATH=. ./platform_app &; sleep 2");  
  vzbase::my_system("killall alg_app;"
                    "chmod +x alg_app;"
                    "LD_LIBRARY_PATH=. ./alg_app &");
  vzbase::my_system("killall business_app;"
                    "chmod +x business_app;"
                    "LD_LIBRARY_PATH=. ./business_app &");
  vzbase::my_system("killall web_server_app;"
                    "chmod +x web_server_app;"
                    "LD_LIBRARY_PATH=. ./web_server_app &");

  vzbase::my_system("killall watchdog;"
                    "chmod +x watchdog;"
                    "LD_LIBRARY_PATH=. ./watchdog &");
}

void CModuleMonitor::StopSomeModule() {
  vzbase::my_system("killall feeddog;"
                    "chmod +x feeddog;"
                    "LD_LIBRARY_PATH=. ./feeddog &");
}

