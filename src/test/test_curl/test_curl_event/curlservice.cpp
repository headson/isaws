/************************************************************************
*Author      : Sober.Peng 17-06-30
*Description :
************************************************************************/
#include "ccurlmulti.h"

#include "vzbase/helper/stdafx.h"

CurlServices::CurlServices(const vzconn::EVT_LOOP* p_evt_loop)
  : p_evt_loop_(p_evt_loop)
  , p_curl_multi_(NULL) {
}

CurlServices::~CurlServices() {

}

bool CurlServices::InitCurlServices() {
  if (p_curl_multi_ == NULL) {
    p_curl_multi_ = curl_multi_init();
    if (p_curl_multi_ == NULL) {
      LOG(L_ERROR) << "curl multi is null.";
      return false;
    }

    /* setup the generic multi interface options we want */
    curl_multi_setopt(p_curl_multi_, CURLMOPT_SOCKETFUNCTION, sock_cb);
    curl_multi_setopt(p_curl_multi_, CURLMOPT_SOCKETDATA, this);
    curl_multi_setopt(p_curl_multi_, CURLMOPT_TIMERFUNCTION, multi_timer_cb);
    curl_multi_setopt(p_curl_multi_, CURLMOPT_TIMERDATA,  this);
    return true;
  }
  return true;
}

bool CurlServices::UninitCurlServices() {
  if (p_curl_multi_) {

  }
}

HttpConn *CurlServices::CreateHttpConn(const std::string  s_url,
                                       uint16             s_url_port,
                                       const std::string  s_post_data,
                                       bool               b_ssl_enable,
                                       uint32             n_timeout,
                                       uint32             n_resend_times) {
  return NULL;
}

bool CurlServices::PostData(HttpConn *p_http_conn) {
  return false;
}

int CurlServices::sock_cb(CURL *native_easy, curl_socket_t s, int what,
                          void *userp, void *sockp) {
  curl_context_t *curl_context;
  if (action == CURL_POLL_IN || action == CURL_POLL_OUT) {
    if (socketp) {
      curl_context = (curl_context_t*) socketp;
    } else {
      curl_context = create_curl_context(s);
      curl_multi_assign(curl_handle, s, (void *) curl_context);
    }
  }

  switch (action) {
  case CURL_POLL_IN:
    //开始polling文件描述符，一旦检测到读事件，则调用curl_perform函数，参数status设置为0
    uv_poll_start(&curl_context->poll_handle, UV_READABLE, curl_perform);
    break;
  case CURL_POLL_OUT:
    uv_poll_start(&curl_context->poll_handle, UV_WRITABLE, curl_perform);
    break;
  case CURL_POLL_REMOVE:
    if (socketp) {
      //停止polling文件描述符
      uv_poll_stop(&((curl_context_t*)socketp)->poll_handle);
      destroy_curl_context((curl_context_t*) socketp);
      curl_multi_assign(curl_handle, s, NULL);
    }
    break;
  default:
    abort();
  }

  return 0;
}

int CurlServices::multi_timer_cb(CURLM *native_multi,
                                 long   timeout_ms,
                                 void  *userp) {
  printf("multi_timer_cb: Setting timeout to %ld ms\n", timeout_ms);

  /* TODO
   * if timeout_ms is 0, call curl_multi_socket_action() at once!
   * if timeout_ms is -1, just delete the timer
   * for all other values of timeout_ms, this should set or *update*
   * the timer to the new value s
   */
  if (userp && timeout_ms > 0) {
    ((CurlServices*)userp)->evt_timer_.Start(timeout_ms, 0);
  }
}

int CurlServices::on_timeout(int fd, short events, void *userp) {
  CURLMcode     rc;
  CurlServices *p_srv = static_cast<CurlServices*>(userp);
  if (p_srv) {
    return -1;
  }

  rc = curl_multi_socket_action(p_srv->p_curl_multi_,
                                CURL_SOCKET_TIMEOUT,
                                0,
                                &p_srv->b_still_running_);
  p_srv->check_multi_info();
}

void CurlServices::check_multi_info() {
  CURLMsg *p_message = NULL;
  char    *s_done_url = NULL;

  int      pending = 0;
  while ((p_message = curl_multi_info_read(p_curl_multi_, &pending))) {
    switch (p_message->msg) {
    case CURLMSG_DONE:
      curl_easy_getinfo(p_message->easy_handle,
                        CURLINFO_EFFECTIVE_URL,
                        &s_done_url);
      printf("%s DONE\n", s_done_url);

      curl_multi_remove_handle(p_curl_multi_, p_message->easy_handle);
      curl_easy_cleanup(p_message->easy_handle);
      break;

    default:
      fprintf(stderr, "CURLMSG default\n");
      abort();
    }
  }
}

