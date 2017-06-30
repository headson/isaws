/************************************************************************
*Author      : Sober.Peng 17-06-30
*Description : ��װcurl multi+libeventʵ���첽http
************************************************************************/
#ifndef CURL_LIBEVENT_CCURLMULTI_H_
#define CURL_LIBEVENT_CCURLMULTI_H_

#include "vzbase/base/basictypes.h"

#include <curl/curl.h>
#include "vzconn/base/clibevent.h"

class HttpConn {
 public:
  HttpConn(const char *s_url,             // http url
           uint16      n_usl_port,        // http url�˿�
           const char *p_post_data,       // post��������
           uint32      n_post_data,       // post���ݳ���
           bool        b_ssl_enable,      // sslʹ��;true=����,false=������
           uint32      n_timeout,         // ��ʱʱ��(ms)
           uint32      n_resend_times);   // �ش�����

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
  // ������ʱ��,����on_timeout
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
