/************************************************************************/
/* Author      : SoberPeng 2017-08-02
/* Description :
/************************************************************************/
#include "cmodulecontrol.h"
#include "vzbase/base/mysystem.h"

void CModuleMonitor::ReStartModule() {
  vzbase::my_system("killall ; killall watchdog;");
  vzbase::my_system("./watchdog &");

  vzbase::my_system("killall feeddog; killall watchdog;");
  vzbase::my_system("./watchdog &");
}

void CModuleMonitor::StopSomeModule() {

}

