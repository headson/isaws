/************************************************************************
*Author      : Sober.Peng 17-09-24
*Description :
************************************************************************/
#ifndef LIBCURLCLIENT_CMULTISERVER_H
#define LIBCURLCLIENT_CMULTISERVER_H

#include "curlclient/base/basedefines.h"

#include "curl/curl.h"
#include "vzconn/base/clibevent.h"

class CMultiServer {
 public:
  class CEvtCurl {
   public:
    explicit CEvtCurl(curl_socket_t fd);
    ~CEvtCurl();

    friend class CMultiServer;

   protected:
    static int32 event_cb(SOCKET fd, short kind, const void *userp);

   public:
    curl_socket_t  fd_sock_;
    vzconn::EVT_IO c_evt_io_;
  };

 protected:
  explicit CMultiServer(vzconn::EVT_LOOP *p_evt_loop);
  virtual ~CMultiServer();

 private:
  CMultiServer(CMultiServer &other) {};

 public:
  static CMultiServer *Create(vzconn::EVT_LOOP *p_evt_loop);
  static void Remove(CMultiServer *curl_service);

  bool PostData(HttpConn *p_conn);

  bool PostDevRegData(HttpConn *p_conn, DeviceRegData &regdata);

  bool PostImageFile(HttpConn       *p_conn,
                     UserGetImgInfo *p_img_info,
                     std::string     s_user,
                     std::string     s_password);

  static bool isSuccess(int errcode, std::string &serr);

 private:
  bool InitCurlServices();
  bool UninitCurlServices();

 protected:
  static int multi_sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp);
  static int multi_timer_cb(CURLM *multi, long timeout_ms, CMultiServer *g);

  static int32 timer_cb(SOCKET fd, short kind, const void *pthiz);

  /* CURLOPT_WRITEFUNCTION */
  static size_t easy_recv_cb(void *ptr, size_t size, size_t nmemb, void *p_thiz);

 public:
  void         easy_check_info();

 public:
  const vzconn::EVT_LOOP *p_evt_loop_;
  vzconn::EVT_TIMER       c_evt_timer_;

  CURLM                  *p_curl_multi_;
  int32                   n_still_running_;
};


#endif  // LIBCURLCLIENT_CMULTISERVER_H
