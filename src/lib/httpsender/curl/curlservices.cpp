/************************************************************************
*Author      : Sober.Peng 17-07-03
*Description : 封装curl multi+libevent实现异步http
************************************************************************/
#include "curlservices.h"

#include <string.h>

#include "vzbase/helper/stdafx.h"

#define MSG_OUT stdout

namespace hs {

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

CurlServices::CEvtCurl::CEvtCurl(curl_socket_t fd)
  : fd_sock_(fd)
  , c_evt_io_() {
}

CurlServices::CEvtCurl::~CEvtCurl() {
  LOG(L_INFO) << "for test.";
}

/* Called by libevent when we get action on a multi socket */
int32 CurlServices::CEvtCurl::event_cb(SOCKET fd, short kind, const void *userp) {
  CurlServices *p_multi = (CurlServices*)userp;
  int action = (kind & EVT_READ ? CURL_CSELECT_IN : 0) |
               (kind & EVT_WRITE ? CURL_CSELECT_OUT : 0);

  CURLMcode rc;
  rc = curl_multi_socket_action(p_multi->p_curl_multi_, fd, action, &p_multi->n_still_running_);
  mcode_or_die("event_cb: curl_multi_socket_action", rc);

  p_multi->easy_check_info();
  if (p_multi->n_still_running_ <= 0) {
    fprintf(MSG_OUT, "last transfer done, kill timeout\n");
    p_multi->c_evt_timer_.Stop();
  }
  return 0;
}

//////////////////////////////////////////////////////////////////////////
CurlServices::CurlServices(vzconn::EVT_LOOP *p_evt_loop,
                           CHttpInterface *p_http_interface)
  : p_evt_loop_(p_evt_loop)
  , c_evt_timer_()
  , p_curl_multi_(NULL)
  , n_still_running_(0)
  , p_http_interface_(p_http_interface) {
}

CurlServices::~CurlServices() {
  UninitCurlServices();
}

CurlServices *CurlServices::Create(vzconn::EVT_LOOP *p_evt_loop,
                                   CHttpInterface *p_http_interface) {
  if (NULL == p_evt_loop || NULL == p_http_interface) {
    LOG(L_ERROR) << "param is null.";
    return NULL;
  }

  return (new CurlServices(p_evt_loop, p_http_interface));
}

bool CurlServices::InitCurlServices() {
  p_curl_multi_ = curl_multi_init();
  if (NULL == p_curl_multi_) {
    LOG(L_ERROR) << "curl multi init failed.";
    return false;
  }

  CURLMcode ret;
  /* setup the generic multi interface options we want */
  ret = curl_multi_setopt(p_curl_multi_, CURLMOPT_SOCKETFUNCTION, multi_sock_cb);
  ret = curl_multi_setopt(p_curl_multi_, CURLMOPT_SOCKETDATA, this);
  ret = curl_multi_setopt(p_curl_multi_, CURLMOPT_TIMERFUNCTION, multi_timer_cb);
  ret = curl_multi_setopt(p_curl_multi_, CURLMOPT_TIMERDATA, this);

  c_evt_timer_.Init(p_evt_loop_, timer_cb, this);
  return true;
}

bool CurlServices::UninitCurlServices() {
  c_evt_timer_.Stop();

  if (p_curl_multi_) {
    curl_multi_cleanup(p_curl_multi_);
    p_curl_multi_ = NULL;
  }
  return true;
}

hs::HttpConn *CurlServices::CreateHttpConn(
  const std::string s_url,
  const std::string s_post_data,
  bool              b_ssl_enable,
  uint16            n_url_port,
  uint32            n_timeout,
  uint32            n_resend_times,
  uint32            n_cb_type) {

  HttpConn *p_conn = new HttpConn(s_url.c_str(),
                                  s_post_data.c_str(),
                                  s_post_data.size(),
                                  b_ssl_enable,
                                  n_url_port,
                                  n_timeout,
                                  n_resend_times,
                                  n_cb_type);

  return p_conn;
}

bool CurlServices::PostData(HttpConn *p_conn) {
  if (p_conn == NULL) {
    return false;
  }

  if (!p_conn->GetEasy()) {
    p_conn->SetEasy(curl_easy_init());
  }
  if (!p_conn->GetEasy()) {
    LOG(L_ERROR) << "curl_easy_init() failed, exiting!";
    return false;
  }

  CURLMcode rc;
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_URL,            p_conn->GetUrl().c_str());

  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_POST,           1);
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_PORT,           p_conn->GetUrlPort());

  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_POSTFIELDS,     p_conn->GetPostData().c_str());
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_POSTFIELDSIZE,  p_conn->GetPostData().size());

  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_WRITEFUNCTION,  easy_recv_cb);
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_WRITEDATA,      p_conn);

  //curl_easy_setopt(p_conn->GetEasy(), CURLOPT_READFUNCTION,   easy_send_cb);
  //curl_easy_setopt(p_conn->GetEasy(), CURLOPT_READDATA,       p_conn);

  if (!p_conn->isSslEnable()) {
    // 非SSL加密传输
    curl_easy_setopt(p_conn->GetEasy(), CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(p_conn->GetEasy(), CURLOPT_SSL_VERIFYHOST, 0L);
  } else {
    // SSL加密传输
  }

  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_TIMEOUT,        p_conn->GetTimeout());
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_PRIVATE,        p_conn);
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_VERBOSE,        1L);
  //curl_easy_setopt(easy, CURLOPT_ERRORBUFFER,               error);
  //curl_easy_setopt(easy, CURLOPT_NOPROGRESS,                0L);
  //curl_easy_setopt(easy, CURLOPT_PROGRESSFUNCTION,          prog_cb);
  //curl_easy_setopt(easy, CURLOPT_PROGRESSDATA,              conn);

  rc = curl_multi_add_handle(p_curl_multi_, p_conn->GetEasy());
  LOG_INFO("Adding easy %p to multi %p (%s), %d\n",
           p_conn->GetEasy(), p_curl_multi_, p_conn->GetUrl().c_str(), rc);
  return (rc == CURLM_OK);
}

bool CurlServices::PostDevRegData(HttpConn *p_conn, DeviceRegData &regdata) {
  if (p_conn == NULL) {
    return false;
  }

  if (!p_conn->GetEasy()) {
    p_conn->SetEasy(curl_easy_init());
  }
  if (!p_conn->GetEasy()) {
    LOG(L_ERROR) << "curl_easy_init() failed, exiting!";
    return false;
  }

  CURLMcode rc;
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_URL,  p_conn->GetUrl().c_str());
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_PORT, p_conn->GetUrlPort());

  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_POST, 1);

  struct curl_httppost* post = NULL;
  struct curl_httppost* last = NULL;
  curl_formadd(&post, &last, CURLFORM_COPYNAME, "device_name",
               CURLFORM_COPYCONTENTS, regdata.device_name.c_str(), CURLFORM_END);
  curl_formadd(&post, &last, CURLFORM_COPYNAME, "ipaddr",
               CURLFORM_COPYCONTENTS, regdata.ipaddr.c_str(), CURLFORM_END);
  curl_formadd(&post, &last, CURLFORM_COPYNAME, "port",
               CURLFORM_COPYCONTENTS, regdata.port.c_str(), CURLFORM_END);
  curl_formadd(&post, &last, CURLFORM_COPYNAME, "user_name",
               CURLFORM_COPYCONTENTS, regdata.user_name.c_str(), CURLFORM_END);
  curl_formadd(&post, &last, CURLFORM_COPYNAME, "pass_wd",
               CURLFORM_COPYCONTENTS, regdata.pass_wd.c_str(), CURLFORM_END);
  curl_formadd(&post, &last, CURLFORM_COPYNAME, "serialno",
               CURLFORM_COPYCONTENTS, regdata.serialno.c_str(), CURLFORM_END);
  curl_formadd(&post, &last, CURLFORM_COPYNAME, "channel_num",
               CURLFORM_COPYCONTENTS, regdata.channel_num.c_str(), CURLFORM_END);

  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_HTTPPOST,     post);
  //curl_easy_setopt(p_conn->GetEasy(), CURLOPT_HTTPHEADER,   "Expect:");
  struct curl_slist *header_list = NULL;
  header_list = curl_slist_append(header_list,
                                  "Expect:");  // 消除curl默认Expect http头
  // header_list  =  curl_slist_append(header_list,  "Connection: Keep-Alive");
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_HTTPHEADER, header_list);

  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_WRITEFUNCTION, easy_recv_cb);
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_WRITEDATA, p_conn);

  //curl_easy_setopt(p_conn->GetEasy(), CURLOPT_READFUNCTION,   easy_send_cb);
  //curl_easy_setopt(p_conn->GetEasy(), CURLOPT_READDATA,       p_conn);

  if (!p_conn->isSslEnable()) {
    // 非SSL加密传输
    curl_easy_setopt(p_conn->GetEasy(), CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(p_conn->GetEasy(), CURLOPT_SSL_VERIFYHOST, 0L);
  } else {
    // SSL加密传输
  }

  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_TIMEOUT, p_conn->GetTimeout());
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_PRIVATE, p_conn);
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_VERBOSE, 1L);
  //curl_easy_setopt(easy, CURLOPT_ERRORBUFFER,               error);
  //curl_easy_setopt(easy, CURLOPT_NOPROGRESS,                0L);
  //curl_easy_setopt(easy, CURLOPT_PROGRESSFUNCTION,          prog_cb);
  //curl_easy_setopt(easy, CURLOPT_PROGRESSDATA,              conn);

  rc = curl_multi_add_handle(p_curl_multi_, p_conn->GetEasy());
  LOG_INFO("Adding easy %p to multi %p (%s), %d\n",
           p_conn->GetEasy(), p_curl_multi_, p_conn->GetUrl().c_str(), rc);
  return (rc == CURLM_OK);
}

static size_t OnReadCallBack(
  void *ptr,
  size_t size,
  size_t nmemb,
  void *stream) {
  LOG(L_WARNING) << "OnReadCallBack";
  if (stream == NULL)
    return 0;
  UserGetImgInfo *puserdata = (UserGetImgInfo *)stream;
  if ((puserdata->pdata == NULL)
      || (puserdata->datasize == 0)
      || (puserdata->datasize == puserdata->curpose)
      || puserdata->curpose < 0) {
    LOG(L_WARNING) << "Error Read";
    return 0;
  }
  int stealsize = puserdata->datasize - puserdata->curpose;
  size_t retcode = (stealsize > size*nmemb) ? size*nmemb : stealsize;
  LOG(L_WARNING) << "OnReadCallBack : "
    << puserdata->datasize
    << " : " << puserdata->curpose
    << " : " << retcode;
  if (puserdata->curpose + retcode >= MAX_BUFFER_SIZE - 1) {
    LOG(L_ERROR) << "retcode is too bigger than buffer";
    return 0;
  }
  memcpy(ptr, (puserdata->pdata + puserdata->curpose), retcode);
  puserdata->curpose += retcode;
  return retcode;
}

bool CurlServices::PostImageFile(HttpConn *p_conn, 
                                 UserGetImgInfo *p_img_info, 
                                 std::string s_user, 
                                 std::string s_password) {
  if (p_conn == NULL) {
    return false;
  }

  if (!p_conn->GetEasy()) {
    p_conn->SetEasy(curl_easy_init());
  }
  if (!p_conn->GetEasy()) {
    LOG(L_ERROR) << "curl_easy_init() failed, exiting!";
    return false;
  }

  CURLMcode rc;
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_URL, p_conn->GetUrl().c_str());
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_PORT, p_conn->GetUrlPort());

  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_UPLOAD, 1);
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_USERNAME, s_user.c_str());
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_PASSWORD, s_password.c_str());

  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_FTP_RESPONSE_TIMEOUT, 10L);
  
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_READFUNCTION,   OnReadCallBack);
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_READDATA,       p_img_info);

  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_INFILESIZE_LARGE,  p_img_info->datasize);
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_FTP_CREATE_MISSING_DIRS,  1L);
  
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_VERBOSE,  1L);
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_APPEND,  1L);

  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_WRITEFUNCTION, easy_recv_cb);
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_WRITEDATA, p_conn);

  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_PRIVATE, p_conn);
  curl_easy_setopt(p_conn->GetEasy(), CURLOPT_VERBOSE, 1L);
  
  rc = curl_multi_add_handle(p_curl_multi_, p_conn->GetEasy());
  LOG_INFO("Adding easy %p to multi %p (%s), %d\n",
           p_conn->GetEasy(), p_curl_multi_, p_conn->GetUrl().c_str(), rc);
  return (rc == CURLM_OK);
}

/* CURLMOPT_SOCKETFUNCTION */
int CurlServices::multi_sock_cb(CURL *e, curl_socket_t s, int what, void *cbp, void *sockp) {
  CurlServices *g = (CurlServices*)cbp;
  CEvtCurl *fdp = (CEvtCurl*)sockp;
  if (what == CURL_POLL_IN || what == CURL_POLL_OUT) {
    if (sockp == NULL) {
      fdp = new CEvtCurl(s);
      if (NULL == fdp) {
        return -1;
      }
      curl_multi_assign(g->p_curl_multi_, s, (void *)fdp); // s与fdp关联
      fdp->c_evt_io_.Init(g->p_evt_loop_, CurlServices::CEvtCurl::event_cb, g);
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
int CurlServices::multi_timer_cb(CURLM *multi, long timeout_ms, CurlServices *g) {
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
int32 CurlServices::timer_cb(SOCKET fd, short kind, const void *p_thiz) {
  CurlServices *p_multi = (CurlServices*)(p_thiz);
  if (p_multi) {
    CURLMcode rc;
    rc = curl_multi_socket_action(p_multi->p_curl_multi_,
                                  CURL_SOCKET_TIMEOUT,
                                  0,
                                  &p_multi->n_still_running_);
    mcode_or_die("timer_cb: curl_multi_socket_action", rc);
    p_multi->easy_check_info();
  }
  return 0;
}

size_t CurlServices::easy_recv_cb(void *ptr,
                                  size_t size, size_t nmemb,
                                  void *p_easy) {
  if (!ptr || !p_easy) {
    LOG(L_ERROR) << "param is null.";
    return -1;
  }

  size_t realsize = size * nmemb;
  ((HttpConn*)p_easy)->SetRespData(ptr, realsize);

  return realsize;
}

/* Check for completed transfers, and remove their easy handles */
void CurlServices::easy_check_info() {
  char        *eff_url;

  int          msgs_left;

  CURLMsg     *msg;
  CURLcode     res;

  CURL        *p_curl;
  HttpConn    *p_easy;
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
      if (p_http_interface_) {
        p_http_interface_->OnHttpResponse(p_easy, (res == CURLE_OK));
      }
      //delete p_easy;
    }
  }
}

HttpConn::HttpConn(const char   *s_url,
                   const char   *s_post_data,
                   uint32        n_post_data,
                   bool          b_ssl_enable,
                   uint16        n_url_port,
                   uint32        n_timeout,
                   uint32        n_resend_times,
                   uint32        n_conn_type)
  : p_easy_(NULL)
  , s_url_("")
  , n_url_port_(0)
  , s_post_data_("")
  , s_resp_data_("")
  , n_timeout_(0)
  , b_ssl_enabel_(0)
  , n_resend_times_(0) {
  s_url_      = s_url;
  n_url_port_ = n_url_port;
  s_post_data_.append(s_post_data, n_post_data);

  n_timeout_ = n_timeout;
  b_ssl_enabel_ = b_ssl_enable ? 1 : 0;
  n_resend_times_ = n_resend_times;
  n_conn_type_    = n_conn_type;
}

HttpConn::~HttpConn() {
}

}  // namespace hs
