#ifndef VZBASE_BASE_LOGGINGSETTINGS_H_
#define VZBASE_BASE_LOGGINGSETTINGS_H_

#define USE_GLOG
#ifdef USE_GLOG

#include "vzlogging/logging/vzloggingcpp.h"

#else

#define LOG(L_INFO) std::cout
#define LOG(L_WARNING) std::cout
#define LOG(L_ERROR) std::cout

#endif

namespace vzbase {

void InitLoggingSystem(int argc, char *argv[]);

}

#endif // VZBASE_BASE_LOGGINGSETTINGS_H_
