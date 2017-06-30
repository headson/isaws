/************************************************************************
*Author      : Sober.Peng 17-06-30
*Description : 封装curl multi+libevent实现异步http
************************************************************************/
#ifndef CURL_LIBEVENT_CCURLMULTI_H_
#define CURL_LIBEVENT_CCURLMULTI_H_

#include "vzbase/base/basictypes.h"

#include <curl/curl.h>
#include "vzconn/base/clibevent.h"

class HttpConn {
 public:
  HttpConn(const char *s_url,             // http url
           uint16      n_usl_port,        // http url端口
           const char *p_post_data,       // post数据内容
           uint32      n_post_data,       // post数据长度
           bool        b_ssl_enable,      // ssl使能;true=启用,false=不启用
           uint32      n_timeout,         // 超时时间(ms)
           uint32      n_resend_times);   // 重传次数

  ~HttpConn() {
  }

 protected:
  std::string   s_url_;                   //
  uint32        n_url_port_;              //

  std::string   s_port_data_;             //

  uint32        b_ssl_enable_;            //

  uint32        n_timeout_;               //
  uint32        n_resend_times_;          //
};

class CurlServices {
 public:
  CurlServices(const vzconn::EVT_LOOP* p_evt_loop);
  virtual ~CurlServices();

  //
  bool InitCurlServices();
  bool UninitCurlServices();

  //
  HttpConn *CreateHttpConn(const std::string s_url,
                           uint16            s_url_port,
                           const std::string s_post_data,
                           bool              b_ssl_enable,
                           uint32            n_timeout,
                           uint32            n_resend_times);

  bool PostData(HttpConn *p_http_conn);

 protected:
  static int sock_cb(CURL          *native_easy,
                     curl_socket_t  s,
                     int            what,
                     void          *userp,
                     void          *sockp);
  // 启动定时器,调用on_timeout
  static int multi_timer_cb(CURLM *native_multi,
                            long   timeout_ms,
                            void  *userp);

  // 
  static int on_timeout(int fd, short events, void *userp);

  /* Check for completed transfers, and remove their easy handles */
  void check_multi_info();


 private:
  const vzconn::EVT_LOOP  *p_evt_loop_;

  CURLM                   *p_curl_multi_;
  int                      b_still_running_;
  
 private:
  vzconn::EVT_TIMER        evt_timer_;
};

#endif  // CURL_LIBEVENT_CCURLMULTI_H_
