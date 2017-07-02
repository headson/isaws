#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include <sys/time.h>
#include <time.h>
//#include <unistd.h>
//#include <sys/poll.h>
#include <curl/curl.h>
#include <event2/event.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

#define MSG_OUT stdout

/* Global information, common to all connections */
typedef struct _GlobalInfo {
  struct event_base *evbase;
  struct event *timer_event;
  CURLM        *multi;
  int           still_running;
} GlobalInfo;

/* Information associated with a specific easy handle */
class ConnInfo {
public:
  ~ConnInfo() {
    fprintf(MSG_OUT, "~ConnInfo\n");
    fflush(MSG_OUT);
  }
  CURL        *easy;
  char        *url;
  GlobalInfo  *global;
  char         error[CURL_ERROR_SIZE];
};

/* Information associated with a specific socket */
typedef struct _SockInfo {
  curl_socket_t  sockfd;
  CURL          *easy;
  int            action;
  long           timeout;
  struct event  *ev;
  int            evset;
  GlobalInfo    *global;
} SockInfo;

/* Update the event timer after curl_multi library calls */
static int multi_timer_cb(CURLM *multi, long timeout_ms, GlobalInfo *g) {
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
  evtimer_add(g->timer_event, &timeout);
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
static void check_multi_info(GlobalInfo *g) {
  char *eff_url;
  CURLMsg *msg;
  int msgs_left;
  ConnInfo *conn;
  CURL *easy;
  CURLcode res;

  fprintf(MSG_OUT, "REMAINING: %d\n", g->still_running);
  while((msg = curl_multi_info_read(g->multi, &msgs_left))) {
    if(msg->msg == CURLMSG_DONE) {
      easy = msg->easy_handle;
      res = msg->data.result;
      curl_easy_getinfo(easy, CURLINFO_PRIVATE, &conn);
      curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL, &eff_url);
      fprintf(MSG_OUT, "DONE: %s => (%d) %s\n", eff_url, res, conn->error);
      curl_multi_remove_handle(g->multi, easy);
      free(conn->url);
      curl_easy_cleanup(easy);
      delete conn;
    }
  }
}



/* Called by libevent when we get action on a multi socket */
static void event_cb(int fd, short kind, void *userp) {
  GlobalInfo *g = (GlobalInfo*) userp;
  CURLMcode rc;

  int action =
    (kind & EV_READ ? CURL_CSELECT_IN : 0) |
    (kind & EV_WRITE ? CURL_CSELECT_OUT : 0);

  rc = curl_multi_socket_action(g->multi, fd, action, &g->still_running);
  mcode_or_die("event_cb: curl_multi_socket_action", rc);

  check_multi_info(g);
  if(g->still_running <= 0) {
    fprintf(MSG_OUT, "last transfer done, kill timeout\n");
    if(evtimer_pending(g->timer_event, NULL)) {
      evtimer_del(g->timer_event);
    }
  }
}



/* Called by libevent when our timeout expires */
static void timer_cb(int fd, short kind, void *userp) {
  GlobalInfo *g = (GlobalInfo *)userp;
  CURLMcode rc;
  (void)fd;
  (void)kind;

  rc = curl_multi_socket_action(g->multi,
                                CURL_SOCKET_TIMEOUT, 
                                0, 
                                &g->still_running);
  mcode_or_die("timer_cb: curl_multi_socket_action", rc);
  check_multi_info(g);
}

/* CURLMOPT_SOCKETFUNCTION */
static int sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp) {
  GlobalInfo *g = (GlobalInfo*)cbp;
  SockInfo *fdp = (SockInfo*)sockp;
  if (what == CURL_POLL_IN || what == CURL_POLL_OUT) {
    if (sockp == NULL) {
      fdp = (SockInfo *)malloc(sizeof(SockInfo));

      fdp->sockfd = s;
      curl_multi_assign(g->multi, s, (void *)fdp); // s与fdp关联
    }
  }

  switch (what) {
  case CURL_POLL_IN:
    if (fdp) {
      // 开始polling文件描述符，一旦检测到读事件，则调用curl_perform函数，参数status设置为0
      fdp->ev = event_new(g->evbase, fdp->sockfd, EV_READ | EV_PERSIST, event_cb, g);
      event_add(fdp->ev, NULL);
    }
    break;
  case CURL_POLL_OUT:
    if (fdp) {
      fdp->ev = event_new(g->evbase, fdp->sockfd, EV_WRITE | EV_PERSIST, event_cb, g);
      event_add(fdp->ev, NULL);
    }
    break;
  case CURL_POLL_REMOVE:
    if (fdp) {
      // 停止polling文件描述符
      event_free(fdp->ev);
      free(fdp);

      curl_multi_assign(g->multi, s, NULL);
    }
    break;
  default:
    abort();
  }
  return 0;
}

/* CURLOPT_WRITEFUNCTION */
static size_t ret_resp_cb(void *ptr, size_t size, size_t nmemb, void *data) {
  size_t realsize = size * nmemb;
  ConnInfo *conn = (ConnInfo*) data;
  (void)ptr;
  (void)conn;
  printf("---------------- resp %s.\n", (char*)ptr);
  return realsize;
}


/* CURLOPT_PROGRESSFUNCTION */
static int prog_cb(void *p, double dltotal, double dlnow, double ult,
                   double uln) {
  ConnInfo *conn = (ConnInfo *)p;
  (void)ult;
  (void)uln;

  fprintf(MSG_OUT, "Progress: %s (%g/%g)\n", conn->url, dlnow, dltotal);
  return 0;
}

/* Create a new easy handle, and add it to the global curl_multi */
static void new_conn(char *url, const char *s_data, GlobalInfo *g) {
  ConnInfo *conn;
  CURLMcode rc;

  conn = new ConnInfo();
  memset(conn, 0, sizeof(ConnInfo));
  conn->error[0]='\0';

  conn->easy = curl_easy_init();
  if(!conn->easy) {
    fprintf(MSG_OUT, "curl_easy_init() failed, exiting!\n");
    exit(2);
  }
  conn->global = g;
  conn->url = strdup(url);
  curl_easy_setopt(conn->easy, CURLOPT_URL, conn->url);
  curl_easy_setopt(conn->easy, CURLOPT_POST, 1);  
  curl_easy_setopt(conn->easy, CURLOPT_POSTFIELDS, s_data);
  curl_easy_setopt(conn->easy, CURLOPT_WRITEFUNCTION, ret_resp_cb);
  curl_easy_setopt(conn->easy, CURLOPT_WRITEDATA, conn);
  curl_easy_setopt(conn->easy, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(conn->easy, CURLOPT_ERRORBUFFER, conn->error);
  curl_easy_setopt(conn->easy, CURLOPT_PRIVATE, conn);
  curl_easy_setopt(conn->easy, CURLOPT_NOPROGRESS, 0L);
  curl_easy_setopt(conn->easy, CURLOPT_PROGRESSFUNCTION, prog_cb);
  curl_easy_setopt(conn->easy, CURLOPT_PROGRESSDATA, conn);
  fprintf(MSG_OUT,
          "Adding easy %p to multi %p (%s)\n", conn->easy, g->multi, url);
  rc = curl_multi_add_handle(g->multi, conn->easy);
  mcode_or_die("new_conn: curl_multi_add_handle", rc);

  /* note that the add_handle() will set a time-out to trigger very soon so
     that the necessary socket_action() call will be called by this app */
}

int main(int argc, char **argv) {
#ifdef WIN32
  WSADATA wsaData;
  WSAStartup(MAKEWORD(2, 2), &wsaData);
  srand((unsigned int)time(NULL));
#endif

  GlobalInfo g;

  memset(&g, 0, sizeof(GlobalInfo));
  g.evbase = event_base_new();
  g.multi = curl_multi_init();
  g.timer_event = evtimer_new(g.evbase, timer_cb, &g);

  /* setup the generic multi interface options we want */
  curl_multi_setopt(g.multi, CURLMOPT_SOCKETFUNCTION, sock_cb);
  curl_multi_setopt(g.multi, CURLMOPT_SOCKETDATA, &g);
  curl_multi_setopt(g.multi, CURLMOPT_TIMERFUNCTION, multi_timer_cb);
  curl_multi_setopt(g.multi, CURLMOPT_TIMERDATA, &g);

  new_conn("http://localhost:8080/bvs/index.jsp", 
    "json=helloworlds.", &g);

  /* we don't call any curl_multi_socket*() function yet as we have no handles
     added! */
  event_base_loop(g.evbase, 0);

  /* this, of course, won't get called since only way to stop this program is
     via ctrl-C, but it is here to show how cleanup /would/ be done. */
  event_free(g.timer_event);
  event_base_free(g.evbase);
  curl_multi_cleanup(g.multi);
  return 0;
}
