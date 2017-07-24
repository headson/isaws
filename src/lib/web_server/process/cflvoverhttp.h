/************************************************************************/
/* Author      : SoberPeng 2017-07-24
/* Description :
/************************************************************************/
#ifndef LIBWEBSERVER_CFLVOVERHTTP_H
#define LIBWEBSERVER_CFLVOVERHTTP_H

#include "vzconn/base/vsocket.h"
#include "vzconn/base/clibevent.h"
#include "vzconn/buffer/cblockbuffer.h"

#include "systemv/shm/vzshm_c.h"
#include "systemv/flvmux/cflvmux.h"

class CFlvOverHttp {
 public:
  CFlvOverHttp();
  virtual ~CFlvOverHttp();

  bool Open(SOCKET sock, vzconn::EVT_LOOP *evt_loop,
            const char* shm_key, unsigned int shm_size);

  void Close();

  int AsyncHeader(const void *phead, unsigned int nhead,
                  unsigned int nwidth, unsigned int nheight);

  int AsyncWrite(const void *p_data, unsigned int n_data);

 protected:
  static int EvtSend(SOCKET fd,
                     short events,
                     const void *p_usr_arg);

  int OnSend();

  static int EvtTimer(SOCKET fd, short events, const void *p_usr_arg);

  int32 OnTimer();

  char *nal_parse(const char *ph264, int nh264, int *frm_type, int *nal_bng);


 private:
  vzconn::EVT_LOOP      *evt_loop_;

  vzconn::VSocket        sock_;

  vzconn::EVT_IO         evt_send_;
  vzconn::CBlockBuffer   send_data_;

  vzconn::EVT_TIMER      evt_timer_;

  CShmVdo                shm_vdo_;
  CFlvMux                flv_shm_;

  int                    avcc_data_size_;
  char                   avcc_data_[1024];

  FILE                  *file;
};


#endif  // LIBWEBSERVER_CFLVOVERHTTP_H