#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#include <curl/curl.h>

#include "vzbase/helper/stdafx.h"
#include "vzconn/base/clibevent.h"

#define MSG_OUT stdout

/* Global information, common to all connections */
class CCurlMulti {
 public:
  class CCurlSock {
   public:
    CCurlSock(curl_socket_t fd)
      : fd_sock_(fd)
      , c_evt_io_() {
    }
    ~CCurlSock() {
      LOG(L_INFO) << "for test.";
    }

    friend class CCurlMulti;

   protected:
    static int32 event_cb(SOCKET fd, short kind, const void *userp);

   public:
    curl_socket_t  fd_sock_;
    vzconn::EVT_IO c_evt_io_;
  };

 public:
  CCurlMulti(vzconn::EVT_LOOP *p_evt_loop)
    : p_evt_loop_(p_evt_loop)
    , c_evt_timer_()
    , p_curl_multi_(NULL)
    , still_running(0) {
  }

  bool  Start() {
    p_curl_multi_ = curl_multi_init();
    if (NULL == p_curl_multi_) {
      LOG(L_ERROR) << "curl multi init failed.";
      return false;
    }

    CURLMcode ret;
    /* setup the generic multi interface options we want */
    ret = curl_multi_setopt(p_curl_multi_, CURLMOPT_SOCKETFUNCTION, sock_cb);
    ret = curl_multi_setopt(p_curl_multi_, CURLMOPT_SOCKETDATA,     this);
    ret = curl_multi_setopt(p_curl_multi_, CURLMOPT_TIMERFUNCTION,  multi_timer_cb);
    ret = curl_multi_setopt(p_curl_multi_, CURLMOPT_TIMERDATA,      this);

    c_evt_timer_.Init(p_evt_loop_, timer_cb, this);
    return true;
  }

 protected:
  static int   sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp);
  static int   multi_timer_cb(CURLM *multi, long timeout_ms, CCurlMulti *g);
  static int32 timer_cb(SOCKET fd, short kind, const void *userp);

 public:
  void check_multi_info();

 public:
  const vzconn::EVT_LOOP *p_evt_loop_;
  vzconn::EVT_TIMER       c_evt_timer_;

  CURLM                  *p_curl_multi_;
  int                     still_running;
};



/* Information associated with a specific easy handle */
class CCurlEasy {
 public:
  CCurlEasy()
    : p_curl_easy_(NULL)
    , p_curl_multi_(NULL) {
  }
  ~CCurlEasy() {
    fprintf(MSG_OUT, "~ConnInfo\n");
    fflush(MSG_OUT);
  }

 public:
  bool Start(CCurlMulti* p_curl_multi,
             const char *p_url,
             const char *p_post_data) {
    p_curl_easy_ = curl_easy_init();
    if (!p_curl_easy_) {
      fprintf(MSG_OUT, "curl_easy_init() failed, exiting!\n");
      exit(2);
    }
    p_curl_multi_ = p_curl_multi;
    //p_srv_url_ = strdup(p_url);

    CURLMcode rc;
    curl_easy_setopt(p_curl_easy_, CURLOPT_URL,           p_url);
    curl_easy_setopt(p_curl_easy_, CURLOPT_POST,          1);
    curl_easy_setopt(p_curl_easy_, CURLOPT_POSTFIELDS,    p_post_data);
    curl_easy_setopt(p_curl_easy_, CURLOPT_WRITEFUNCTION, ret_resp_cb);
    curl_easy_setopt(p_curl_easy_, CURLOPT_WRITEDATA,     this);
    curl_easy_setopt(p_curl_easy_, CURLOPT_VERBOSE,       1L);
    //curl_easy_setopt(easy, CURLOPT_ERRORBUFFER,   error);
    curl_easy_setopt(p_curl_easy_, CURLOPT_PRIVATE,       this);
    //curl_easy_setopt(easy, CURLOPT_NOPROGRESS,        0L);
    //curl_easy_setopt(easy, CURLOPT_PROGRESSFUNCTION,  prog_cb);
    //curl_easy_setopt(easy, CURLOPT_PROGRESSDATA,      onn);
    rc = curl_multi_add_handle(p_curl_multi_->p_curl_multi_, p_curl_easy_);
    fprintf(MSG_OUT,
            "Adding easy %p to multi %p (%s), %d\n",
            p_curl_easy_, p_curl_multi_, p_url, rc);
  }

 public:
  static size_t ret_resp_cb(void *ptr, size_t size, size_t nmemb, void *data);

 public:
  CURL        *p_curl_easy_;
  CCurlMulti  *p_curl_multi_;
};

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

/* Check for completed transfers, and remove their easy handles */
void CCurlMulti::check_multi_info() {
  char *eff_url;
  CURLMsg *msg;
  int msgs_left;
  CCurlEasy *conn;
  CURL *easy;
  CURLcode res;

  fprintf(MSG_OUT, "REMAINING: %d\n", still_running);
  while((msg = curl_multi_info_read(p_curl_multi_, &msgs_left))) {
    if(msg->msg == CURLMSG_DONE) {
      easy = msg->easy_handle;
      res  = msg->data.result;
      curl_easy_getinfo(easy, CURLINFO_PRIVATE,       &conn);
      curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL, &eff_url);
      curl_multi_remove_handle(p_curl_multi_, easy);
      curl_easy_cleanup(easy);

      printf("check_multi_info res %d.\n", res);
      delete conn;
    }
  }
}

/* Called by libevent when we get action on a multi socket */
int32 CCurlMulti::CCurlSock::event_cb(SOCKET fd, short kind, const void *userp) {
  CCurlMulti *g = (CCurlMulti*) userp;
  CURLMcode rc;

  int action =
    (kind & EV_READ ? CURL_CSELECT_IN : 0) |
    (kind & EV_WRITE ? CURL_CSELECT_OUT : 0);

  rc = curl_multi_socket_action(g->p_curl_multi_, fd, action, &g->still_running);
  mcode_or_die("event_cb: curl_multi_socket_action", rc);

  g->check_multi_info();
  if(g->still_running <= 0) {
    fprintf(MSG_OUT, "last transfer done, kill timeout\n");
    if(g->c_evt_timer_.IsPending(NULL)) {
      g->c_evt_timer_.Stop();
    }
  }
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
                                &g->still_running);
  mcode_or_die("timer_cb: curl_multi_socket_action", rc);
  g->check_multi_info();
  return 0;
}

/* CURLMOPT_SOCKETFUNCTION */
int CCurlMulti::sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp) {
  CCurlMulti *g = (CCurlMulti*)cbp;
  CCurlSock *fdp = (CCurlSock*)sockp;
  if (what == CURL_POLL_IN || what == CURL_POLL_OUT) {
    if (sockp == NULL) {
      fdp = new CCurlSock(s);

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

/* CURLOPT_WRITEFUNCTION */
size_t CCurlEasy::ret_resp_cb(void *ptr, size_t size, size_t nmemb, void *data) {
  size_t realsize = size * nmemb;
  CCurlEasy *conn = (CCurlEasy*) data;
  (void)ptr;
  (void)conn;
  printf("---------------- resp %s.\n", (char*)ptr);
  return realsize;
}

int main(int argc, char **argv) {
#ifdef WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
  srand((unsigned int)time(NULL));
#endif

  vzconn::EVT_LOOP c_evt_loop;
  c_evt_loop.Start();


  CCurlMulti g(&c_evt_loop);
  if (g.Start() == false) {
    return -1;
  }

  CCurlEasy *p_curl_easy = new CCurlEasy();
  if (p_curl_easy) {
    p_curl_easy->Start(&g,
                       "http://localhost:8080/vz/index.jsp",
                       "json=helloworlds.");
  }

  /* we don't call any curl_multi_socket*() function yet as we have no handles
     added! */
  c_evt_loop.RunLoop(10000);
  /* this, of course, won't get called since only way to stop this program is
     via ctrl-C, but it is here to show how cleanup /would/ be done. */

  g.c_evt_timer_.Stop();
  //g.p_evt_loop_->Stop();
  c_evt_loop.Stop();
  curl_multi_cleanup(g.p_curl_multi_);
  return 0;
}
