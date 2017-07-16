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

  std::string   sw_ver;
  std::string   hw_ver;
  std::string   iva_ver;

  unsigned int  dhcp_en;

  unsigned int  web_port;
  unsigned int  rtsp_port;
  unsigned int  rec_size;
} TAG_SYS_INFO;

#endif  // LIBSYSTEMSERVER_PKGHEAD_H
