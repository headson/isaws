/************************************************************************
*Author      : Sober.Peng 17-06-30
*Description :
************************************************************************/
#include "ccurlmulti.h"

#include "vzbase/helper/stdafx.h"

#include "ccurleasy.h"

#define MSG_OUT stdout

/* Die if we get a bad CURLMcode somewhere */
static void mcode_or_die(const char *where, CURLMcode code) {
  if(CURLM_OK != code) {
    const char *s;
    switch(code) {
    case     CURLM_BAD_HANDLE:
      s="CURLM_BAD_HANDLE";
      break;
    case     CURLM_BAD_EASY_HANDLE:
      s="CURLM_BAD_EASY_HANDLE";
      break;
    case     CURLM_OUT_OF_MEMORY:
      s="CURLM_OUT_OF_MEMORY";
      break;
    case     CURLM_INTERNAL_ERROR:
      s="CURLM_INTERNAL_ERROR";
      break;
    case     CURLM_UNKNOWN_OPTION:
      s="CURLM_UNKNOWN_OPTION";
      break;
    case     CURLM_LAST:
      s="CURLM_LAST";
      break;
    default:
      s="CURLM_unknown";
      break;
    case     CURLM_BAD_SOCKET:
      s="CURLM_BAD_SOCKET";
      fprintf(MSG_OUT, "ERROR: %s returns %s\n", where, s);
      /* ignore this error */
      return;
    }
    fprintf(MSG_OUT, "ERROR: %s returns %s\n", where, s);
    exit(code);
  }
}

CCurlMulti::CCurlSock::CCurlSock(curl_socket_t fd)
  : fd_sock_(fd)
  , c_evt_io_() {

}

CCurlMulti::CCurlSock::~CCurlSock() {
  LOG(L_INFO) << "for test.";
}

/* Called by libevent when we get action on a multi socket */
int32 CCurlMulti::CCurlSock::event_cb(SOCKET fd, short kind, const void *userp) {
  CCurlMulti *g = (CCurlMulti*)userp;
  int action = (kind & EVT_READ ? CURL_CSELECT_IN : 0) |
               (kind & EVT_WRITE ? CURL_CSELECT_OUT : 0);

  CURLMcode rc;
  rc = curl_multi_socket_action(g->p_curl_multi_, fd, action, &g->n_still_running_);
  mcode_or_die("event_cb: curl_multi_socket_action", rc);

  g->check_multi_info();
  if (g->n_still_running_ <= 0) {
    fprintf(MSG_OUT, "last transfer done, kill timeout\n");
    g->c_evt_timer_.Stop();
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////
CCurlMulti::CCurlMulti(vzconn::EVT_LOOP *p_evt_loop)
  : p_evt_loop_(p_evt_loop)
  , c_evt_timer_()
  , p_curl_multi_(NULL)
  , n_still_running_(0) {
}

CCurlMulti::~CCurlMulti() {
  UninitCurlServices();
}

CCurlMulti *CCurlMulti::Create(vzconn::EVT_LOOP *p_evt_loop) {
  if (p_evt_loop) {
    LOG(L_ERROR) << "param is null.";
    return NULL;
  }

  return (new CCurlMulti(p_evt_loop));
}

bool CCurlMulti::InitCurlServices() {
  p_curl_multi_ = curl_multi_init();
  if (NULL == p_curl_multi_) {
    LOG(L_ERROR) << "curl multi init failed.";
    return false;
  }

  CURLMcode ret;
  /* setup the generic multi interface options we want */
  ret = curl_multi_setopt(p_curl_multi_, CURLMOPT_SOCKETFUNCTION, sock_cb);
  ret = curl_multi_setopt(p_curl_multi_, CURLMOPT_SOCKETDATA, this);
  ret = curl_multi_setopt(p_curl_multi_, CURLMOPT_TIMERFUNCTION, multi_timer_cb);
  ret = curl_multi_setopt(p_curl_multi_, CURLMOPT_TIMERDATA, this);

  c_evt_timer_.Init(p_evt_loop_, timer_cb, this);
  return true;
}

bool CCurlMulti::UninitCurlServices() {
  c_evt_timer_.Stop();

  if (p_curl_multi_) {
    curl_multi_cleanup(p_curl_multi_);
    p_curl_multi_ = NULL;
  }
}

/* CURLMOPT_SOCKETFUNCTION */
int CCurlMulti::sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp) {
  CCurlMulti *g = (CCurlMulti*)cbp;
  CCurlSock *fdp = (CCurlSock*)sockp;
  if (what == CURL_POLL_IN || what == CURL_POLL_OUT) {
    if (sockp == NULL) {
      fdp = new CCurlSock(s);
      if (NULL == fdp) {
        return -1;
      }
      curl_multi_assign(g->p_curl_multi_, s, (void *)fdp); // s与fdp关联
      fdp->c_evt_io_.Init(g->p_evt_loop_, CCurlMulti::CCurlSock::event_cb, g);
    }
  }

  switch (what) {
  case CURL_POLL_IN:
    if (fdp) {
      // 开始polling文件描述符，一旦检测到读事件，则调用curl_perform函数，参数status设置为0
      //fdp->ev = event_new(g->evbase, fdp->sockfd, EV_READ | EV_PERSIST, event_cb, g);
      //event_add(fdp->ev, NULL);
      fdp->c_evt_io_.Start(fdp->fd_sock_, EVT_READ | EVT_PERSIST);
    }
    break;
  case CURL_POLL_OUT:
    if (fdp) {
      /*fdp->ev = event_new(g->evbase, fdp->sockfd, EV_WRITE | EV_PERSIST, event_cb, g);
      event_add(fdp->ev, NULL);*/
      fdp->c_evt_io_.Start(fdp->fd_sock_, EVT_WRITE | EVT_PERSIST);
    }
    break;
  case CURL_POLL_REMOVE:
    if (fdp) {
      // 停止polling文件描述符
      fdp->c_evt_io_.Stop();
      delete fdp;

      curl_multi_assign(g->p_curl_multi_, s, NULL);
    }
    break;
  default:
    abort();
  }
  return 0;
}

/* Update the event timer after curl_multi library calls */
int CCurlMulti::multi_timer_cb(CURLM *multi, long timeout_ms, CCurlMulti *g) {
  struct timeval timeout;
  (void)multi; /* unused */

  timeout.tv_sec = timeout_ms/1000;
  timeout.tv_usec = (timeout_ms%1000)*1000;
  fprintf(MSG_OUT, "multi_timer_cb: Setting timeout to %ld ms\n", timeout_ms);

  /* TODO
   *
   * if timeout_ms is 0, call curl_multi_socket_action() at once!
   *
   * if timeout_ms is -1, just delete the timer
   *
   * for all other values of timeout_ms, this should set or *update*
   * the timer to the new value
   */
  if (timeout_ms < 0) {
    return -1;
  }
  g->c_evt_timer_.Start(timeout_ms, 0);
  return 0;
}

/* Called by libevent when our timeout expires */
int32 CCurlMulti::timer_cb(SOCKET fd, short kind, const void *userp) {
  CCurlMulti *g = (CCurlMulti *)userp;
  CURLMcode rc;
  (void)fd;
  (void)kind;

  rc = curl_multi_socket_action(g->p_curl_multi_,
                                CURL_SOCKET_TIMEOUT,
                                0,
                                &g->n_still_running_);
  mcode_or_die("timer_cb: curl_multi_socket_action", rc);
  g->check_multi_info();
  return 0;
}

/* Check for completed transfers, and remove their easy handles */
void CCurlMulti::check_multi_info() {
  char        *eff_url;
  
  int          msgs_left;
  
  CURLMsg     *msg;
  CURLcode     res;

  CURL        *p_curl;
  CCurlEasy   *p_easy;
  fprintf(MSG_OUT, "REMAINING: %d\n", n_still_running_);
  while ((msg = curl_multi_info_read(p_curl_multi_, &msgs_left))) {
    if (msg->msg == CURLMSG_DONE) {
      p_curl = msg->easy_handle;
      res    = msg->data.result;
      curl_easy_getinfo(p_curl, CURLINFO_PRIVATE,       &p_easy);
      curl_easy_getinfo(p_curl, CURLINFO_EFFECTIVE_URL, &eff_url);
      curl_multi_remove_handle(p_curl_multi_, p_curl);
      curl_easy_cleanup(p_curl);

      printf("check_multi_info res %d.\n", res);
      delete p_easy;
    }
  }
}
