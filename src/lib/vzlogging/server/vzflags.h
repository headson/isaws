#ifndef VZBASE_FLAGS_VZFLAGS_H_
#define VZBASE_FLAGS_VZFLAGS_H_

#ifdef WIN32
#include "vzlogging/server/getopt.h"
#else
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#endif

#endif  // VZBASE_FLAGS_VZFLAGS_H_