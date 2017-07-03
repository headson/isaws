/************************************************************************
*Author      : Sober.Peng 17-06-30
*Description : 封装curl multi+libevent实现异步http
************************************************************************/
#ifndef CURL_LIBEVENT_CCURLMULTI_H_
#define CURL_LIBEVENT_CCURLMULTI_H_

#include "vzbase/base/basictypes.h"

#include <curl/curl.h>
#include "vzconn/base/clibevent.h"

class CCurlMulti {
 public:
  class CCurlSock {
   public:
    CCurlSock(curl_socket_t fd);
    ~CCurlSock();

    friend class CCurlMulti;

   protected:
    static int32 event_cb(SOCKET fd, short kind, const void *userp);

   public:
    curl_socket_t  fd_sock_;
    vzconn::EVT_IO c_evt_io_;
  };

 protected:
  CCurlMulti(vzconn::EVT_LOOP *p_evt_loop);

 public:
  virtual ~CCurlMulti();
  static CCurlMulti *Create(vzconn::EVT_LOOP *p_evt_loop);

  bool InitCurlServices();
  bool UninitCurlServices();

 protected:
  static int   sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp);
  static int   multi_timer_cb(CURLM *multi, long timeout_ms, CCurlMulti *g);
  static int32 timer_cb(SOCKET fd, short kind, const void *userp);

 public:
  void         check_multi_info();

 public:
  const vzconn::EVT_LOOP *p_evt_loop_;
  vzconn::EVT_TIMER       c_evt_timer_;

  CURLM                  *p_curl_multi_;
  int32                   n_still_running_;
};

#endif  // CURL_LIBEVENT_CCURLMULTI_H_

