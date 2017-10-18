/************************************************************************/
/* Author      : SoberPeng 2017-07-24
/* Description :
/************************************************************************/
#ifndef LIBWEBSERVER_CFLVOVERHTTP_H
#define LIBWEBSERVER_CFLVOVERHTTP_H

#include "vzconn/base/vsocket.h"
#include "vzconn/base/clibevent.h"
#include "vzconn/buffer/cblockbuffer.h"

#include "vzbase/system/vshm.h"
#include "web_server/process/cflvmux.h"

#include "vzbase/base/boost_settings.hpp"

class CFlvOverHttp : public vzbase::noncopyable {
 public:
  typedef boost::shared_ptr<CFlvOverHttp> Ptr;

 public:
  CFlvOverHttp();
  virtual ~CFlvOverHttp();

  bool Open(SOCKET sock, vzconn::EVT_LOOP *evt_loop, int dev_num);
  void Close();

  int AsyncHeader(const void *presp, unsigned int nresp);
  int AsyncWrite(const void *pdata, unsigned int ndata);

 protected:
  static int EvtSend(SOCKET fd, short events, const void *usr_arg);
  int OnSend();

  static int EvtTimer(SOCKET fd, short events, const void *usr_arg);
  int32 OnTimer();

  char *nal_parse(const char *ph264, int nh264, int *frm_type, int *nal_bng);

 private:
  CFlvOverHttp::Ptr      thiz_ptr_;
  unsigned int           exit_flag_;

 private:
  vzconn::EVT_LOOP      *evt_loop_;

  SOCKET                 sock_;

  vzconn::EVT_IO         evt_send_;
  vzconn::CBlockBuffer   send_data_;

  vzconn::EVT_TIMER      evt_timer_;

 private:
  int                    dev_num_;
  vzbase::VShm           shm_vdo_;
  TAG_SHM_VDO           *shm_vdo_ptr_;
  unsigned int           w_sec_, w_usec_;

 private:
  unsigned int           pts_;

  CFlvMux                flv_shm_;
  char                  *flv_data_;
  int                    flv_data_size_;

  int                    avcc_data_size_;
  char                   avcc_data_[1024];
};


#endif  // LIBWEBSERVER_CFLVOVERHTTP_H
