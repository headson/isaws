/************************************************************************
*Author      : Sober.Peng 17-07-03
*Description : 封装curl multi+libevent实现异步http
************************************************************************/
#ifndef LIBHTTPSENDER_CCURLMULTI_H_
#define LIBHTTPSENDER_CCURLMULTI_H_

#include "vzbase/base/basictypes.h"

#include <string>
#include <curl/curl.h>

#include "vzconn/base/clibevent.h"

class CMultiServer {
 public:
  class CCurlEvent {
   public:
    explicit CCurlEvent(curl_socket_t fd);
    ~CCurlEvent();

    friend class CMultiServer;

   protected:
    static int event_cb(SOCKET fd, short kind, const void *userp);

   public:
    vzconn::EVT_IO evt_io_;
    curl_socket_t  curl_sock_;
  };

 protected:
  explicit CMultiServer(vzconn::EVT_LOOP *p_evt_loop);
  virtual ~CMultiServer();

 private:
  CMultiServer(CMultiServer &other) {};

 public:
  static CMultiServer *Create(vzconn::EVT_LOOP *p_evt_loop);
  static void Remove(CMultiServer *curl_service);

  CURLM *curlm() {
    return curl_multi_;
  }

  static const char *CurlEasyError(int error);

 private:
  bool InitCurlServices();
  bool UninitCurlServices();

 protected:
  static int multi_sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp);
  static int multi_timer_cb(CURLM *multi, long timeout_ms, CMultiServer *g);

  static int timer_cb(SOCKET fd, short kind, const void *pthiz);

 public:
  void easy_check_info();

 public:
  const vzconn::EVT_LOOP *evt_loop_;
  vzconn::EVT_TIMER       evt_timer_;

  CURLM                  *curl_multi_;
  int32                   still_running_;
};

#endif  // LIBHTTPSENDER_CCURLMULTI_H_

