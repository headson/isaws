/************************************************************************/
/* Author      : SoberPeng 2017-06-14
/* Description :
/************************************************************************/
#ifndef CMULTICAST_H
#define CMULTICAST_H

#include "base/vtypes.h"

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#include <ws2ipdef.h>
#endif

#include "csocketinterface.h"

#define DETECT_DEVICE_ADDR "239.0.0.30"
#define DETECT_DEVICE_PORT 20002

class CMultiCast : public CSocketInterface {
 public:
  CMultiCast();
  virtual ~CMultiCast();

  int32_t Open(const CInetAddr *p_mcast_addr, const EVT_LOOP* p_evt_loop);

  //virtual int32_t Recv(void* pData, uint32_t nData);
  //virtual int32_t Send(const void* pData, uint32_t nData);

 protected:
  virtual int32_t OnRecv(int32_t n_evt, const void* p_usr_arg);
  virtual int32_t OnSend(int32_t n_evt, const void* p_usr_arg);

 private:
  struct ip_mreq  st_mreq_;
  CInetAddr       c_mcast_addr_;
};

#endif  // CMULTICAST_H
