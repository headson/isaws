/************************************************************************
*Author      : Sober.Peng 17-07-12
*Description : 
************************************************************************/
#ifndef LIBSYSTEMSERVER_PKGHEAD_H
#define LIBSYSTEMSERVER_PKGHEAD_H

#include "vzbase/base/basictypes.h"

#include <string>

typedef struct _TAG_SYS_INFO{
  std::string   dev_name;
  std::string   ins_addr;     // °²×°µØÖ·

  unsigned int  dev_type;

  unsigned int  sw_ver;
  unsigned int  hw_ver;
  unsigned int  iva_ver;

  unsigned int  web_port;
  unsigned int  rtsp_port;
  unsigned int  rec_size;
} TAG_SYS_INFO;

#endif  // LIBSYSTEMSERVER_PKGHEAD_H
