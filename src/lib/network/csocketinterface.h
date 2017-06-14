/************************************************************************/
/* Author      : Sober.Peng 17-06-14
/* Description :
/************************************************************************/
#ifndef LIBNETWORK_CSOCKETINTERFACE_H
#define LIBNETWORK_CSOCKETINTERFACE_H

#include "base/vtypes.h"

#include "network/vevent.h"
#include "network/vsocket.h"

class CSocketInterface : public VSocket {
 public:
  CSocketInterface()
    : VSocket() {
  }

  virtual ~CSocketInterface() {
  }

  int32_t InitEvent(const EVT_LOOP* p_evt_loop) {
    int32_t n_ret = -1;

    c_evt_recv_.Init(p_evt_loop, RecvCallback, this);
    n_ret = c_evt_recv_.Start(GetSocket(), EVT_READ);
    if (n_ret != 0) {
      return n_ret;
    }

    c_evt_send_.Init(p_evt_loop, SendCallback, this);
    //n_ret = c_evt_send.Start(GetSocket(), EVT_WRITE);
    return n_ret;
  }

  int32_t StartSend() {
    return c_evt_send_.Start(GetSocket(), EVT_WRITE);
  }

  void    StopSend() {
    return c_evt_send_.Stop();
  }

 protected:
  // recv
  static int32_t RecvCallback(int32_t n_evt, const void* p_usr_arg) {
    if (p_usr_arg) {
      return ((CSocketInterface*)p_usr_arg)->OnRecv(n_evt, p_usr_arg);
    }
    return -1;
  }
  virtual int32_t OnRecv(int32_t n_evt, const void* p_usr_arg) = 0;

  // send
  static int32_t SendCallback(int32_t n_evt, const void* p_usr_arg) {
    if (p_usr_arg) {
      return ((CSocketInterface*)p_usr_arg)->OnSend(n_evt, p_usr_arg);
    }
    return -1;
  }
  virtual int32_t OnSend(int32_t n_evt, const void* p_usr_arg) = 0;

 protected:
  EVT_IO  c_evt_recv_;
  EVT_IO  c_evt_send_;
};



#endif  // LIBNETWORK_CSOCKETINTERFACE_H
