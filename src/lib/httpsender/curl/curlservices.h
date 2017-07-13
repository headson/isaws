/************************************************************************
*Author      : Sober.Peng 17-07-03
*Description : 封装curl multi+libevent实现异步http
************************************************************************/
#ifndef LIBHTTPSENDER_CCURLMULTI_H_
#define LIBHTTPSENDER_CCURLMULTI_H_

#include "vzbase/base/basictypes.h"

#include <curl/curl.h>

#include "vzconn/base/clibevent.h"

#include "httpsender/base/basedefines.h"

namespace hs {

class HttpConn {
 public:
  HttpConn(const char   *s_url,
           const char   *s_post_data,
           uint32        n_post_data,
           bool          b_ssl_enable,
           uint16        n_url_port,
           uint32        n_timeout,
           uint32        n_resend_times,
           uint32        n_conn_type);
  ~HttpConn();

  void SetEasy(CURL *p_easy) {
    p_easy_ = p_easy;
  }
  CURL *GetEasy() {
    return p_easy_;
  }

  std::string &GetUrl() {
    return s_url_;
  }
  uint32      GetUrlPort() {
    return n_url_port_;
  }
  const std::string &GetPostData() {
    return s_post_data_;
  }

  void SetRespData(const void* p_data, uint32 n_data) {
    s_resp_data_.append((char*)p_data, n_data);
    printf("recv buffer %s.\n", s_resp_data_.c_str());
  }
  const std::string &GetRespData() {
    return s_resp_data_;
  }

  uint32 GetTimeout() {
    return n_timeout_;
  }
  bool   isSslEnable() {
    return ((b_ssl_enabel_ > 0) ? true : false);
  }
  uint32 GetResendTimes() {
    return n_resend_times_;
  }

  uint32 GetConnType() {
    return n_conn_type_;
  }

 private:
  CURL        *p_easy_;

 private:
  uint32       n_conn_type_;        // 链接类型

  std::string  s_url_;             // URL地址
  uint32       n_url_port_;        // URL端口
  std::string  s_post_data_;       // 上传数据

  std::string  s_resp_data_;       // 返回数据

  uint32       n_timeout_;         // 超时
  uint32       b_ssl_enabel_;      // 使能ssl
  uint32       n_resend_times_;    // 重发次数
};

class CHttpInterface {
 public:
  /************************************************************************
  *Description : 接收完成回调,n_result=0发送成功,n_result!=0接收失败,
  *              需要用户在回调中释放HttpConn
  *Parameters  :
  *Return      :
  ************************************************************************/
  virtual bool OnHttpResponse(HttpConn* p_conn, bool b_success) = 0;
};

class CurlServices {
 public:
  class CEvtCurl {
   public:
    CEvtCurl(curl_socket_t fd);
    ~CEvtCurl();

    friend class CurlServices;

   protected:
    static int32 event_cb(SOCKET fd, short kind, const void *userp);

   public:
    curl_socket_t  fd_sock_;
    vzconn::EVT_IO c_evt_io_;
  };

 protected:
  CurlServices(vzconn::EVT_LOOP *p_evt_loop, CHttpInterface *p_http_interface);

 public:
  virtual ~CurlServices();
  static CurlServices *Create(vzconn::EVT_LOOP *p_evt_loop,
                              CHttpInterface *p_http_interface);

  bool InitCurlServices();
  bool UninitCurlServices();

  HttpConn *CreateHttpConn(const std::string s_url,
                           const std::string s_post_data,
                           bool              b_ssl_enable,
                           uint16            n_url_port,
                           uint32            n_timeout,
                           uint32            n_resend_times,
                           uint32            n_cb_type);

  bool PostData(HttpConn *p_conn);
  bool PostDevRegData(HttpConn *p_conn, DeviceRegData &regdata);

  bool PostImageFile(HttpConn       *p_conn,
                     UserGetImgInfo *p_img_info,
                     std::string     s_user,
                     std::string     s_password);

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

 private:
  CHttpInterface         *p_http_interface_;
};

}  // namespace hs

#endif  // LIBHTTPSENDER_CCURLMULTI_H_

