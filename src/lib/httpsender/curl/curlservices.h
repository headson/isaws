/************************************************************************
*Author      : Sober.Peng 17-07-03
*Description : ��װcurl multi+libeventʵ���첽http
************************************************************************/
#ifndef LIBHTTPSENDER_CCURLMULTI_H_
#define LIBHTTPSENDER_CCURLMULTI_H_

#include "vzbase/base/basictypes.h"

#include <curl/curl.h>

#include "vzconn/base/clibevent.h"

#include "httpsender/base/basedefines.h"

namespace hs {

class HttpConn;

class PostInterface {
 public:
  virtual void PostCallBack(HttpConn *cfg, int errcode) = 0;
};

class HttpConn {
 public:
  uint32       request_type_;      // ������������
  void        *user_data_;         // �û�����

  std::string  s_url_;             // URL��ַ
  uint32       n_url_port_;        // URL�˿�

  std::string  s_post_data_;       // �ϴ�����
  int          post_data_size_;

  uint32       n_timeout_;         // ��ʱ
  uint32       b_ssl_enabel_;      // ʹ��ssl
  uint32       n_resend_times_;    // �ط�����

  PostInterface *post_callback_;  // �ص�

 private:
  CURL        *curl_easy_;         // curl ���
  curl_slist  *slist_;

 public:
  std::string  s_resp_data_;       // ��������

  CURL *curl() {
    return curl_easy_;
  }
  void Clean();

 public:
  HttpConn(const char    *s_url,
           uint16         n_url_port,
           const char    *s_post_data,
           int            post_data_size,
           uint32         n_timeout,
           uint32         n_resend_times,
           bool           b_ssl_enable,
           uint32         n_conn_type,
           PostInterface *post_callback);
  ~HttpConn();

 private:
  HttpConn(HttpConn &other) {}
};

class CurlServices {
 public:
  class CEvtCurl {
   public:
    explicit CEvtCurl(curl_socket_t fd);
    ~CEvtCurl();

    friend class CurlServices;

   protected:
    static int32 event_cb(SOCKET fd, short kind, const void *userp);

   public:
    curl_socket_t  fd_sock_;
    vzconn::EVT_IO c_evt_io_;
  };

 protected:
  explicit CurlServices(vzconn::EVT_LOOP *p_evt_loop);
  virtual ~CurlServices();

 private:
  CurlServices(CurlServices &other) {};

 public:
  static CurlServices *Create(vzconn::EVT_LOOP *p_evt_loop);
  static void Remove(CurlServices *curl_service);

  HttpConn *CreateHttpConn(const std::string s_url,
                           const std::string s_post_data,
                           bool              b_ssl_enable,
                           uint16            n_url_port,
                           uint32            n_timeout,
                           uint32            n_resend_times,
                           uint32            n_conn_type,
                           PostInterface    *post_callback);

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
  static int   multi_sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp);
  static int   multi_timer_cb(CURLM *multi, long timeout_ms, CurlServices *g);

  static int32  timer_cb(SOCKET fd, short kind, const void *p_thiz);

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

}  // namespace hs

#endif  // LIBHTTPSENDER_CCURLMULTI_H_

