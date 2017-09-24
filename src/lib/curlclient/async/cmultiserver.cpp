/************************************************************************
*Author      : Sober.Peng 17-09-24
*Description : 
************************************************************************/
#include "cmultiserver.h"

#include "vzbase/helper/stdafx.h"

CMultiServer::CMultiServer(vzconn::EVT_LOOP *p_evt_loop)
  : p_evt_loop_(p_evt_loop),
    c_evt_timer_(),
    p_curl_multi_(NULL),
    n_still_running_(0) {
  curl_global_init(CURL_GLOBAL_DEFAULT);
}

CMultiServer::~CMultiServer() {
  UninitCurlServices();
  curl_global_cleanup();
}

CMultiServer *CMultiServer::Create(vzconn::EVT_LOOP *p_evt_loop) {

  if (NULL == p_evt_loop) {
    LOG(L_ERROR) << "param is null.";
    return NULL;
  }

  CMultiServer *new_p = new CMultiServer(p_evt_loop);
  if (!new_p->InitCurlServices()) {
    LOG(L_ERROR) << "CurlServices InitCurlServices failure!";
    delete new_p;
    new_p = NULL;
  }
  return new_p;
}

void CMultiServer::Remove(CMultiServer *ins) {
  delete ins;
}

bool CMultiServer::InitCurlServices() {
  p_curl_multi_ = curl_multi_init();
  if (NULL == p_curl_multi_) {
    LOG(L_ERROR) << "curl multi init failed.";
    return false;
  }

  /* setup the generic multi interface options we want */
  CURLMcode ret = curl_multi_setopt(p_curl_multi_, CURLMOPT_SOCKETFUNCTION,
                                    multi_sock_cb);
  ret = curl_multi_setopt(p_curl_multi_, CURLMOPT_SOCKETDATA, this);
  ret = curl_multi_setopt(p_curl_multi_, CURLMOPT_TIMERFUNCTION,
                          multi_timer_cb);
  ret = curl_multi_setopt(p_curl_multi_, CURLMOPT_TIMERDATA, this);

  c_evt_timer_.Init(p_evt_loop_, timer_cb, this);
  return true;
}

bool CMultiServer::UninitCurlServices() {
  c_evt_timer_.Stop();

  if (p_curl_multi_) {
    curl_multi_cleanup(p_curl_multi_);
    p_curl_multi_ = NULL;
  }
  return true;
}

bool CMultiServer::PostData(HttpConn *cfg) {
  // LOG(INFO) << "curl memsize " << galloc_mem_size;
  if (cfg == NULL) {
    return false;
  }

  if (NULL == cfg) {
    LOG(L_ERROR) << "curl TryPop is NULL, exiting!";
    return false;
  }

  //*cfg = *conn;
  CURL *curl = cfg->curl();
  int timeout_ms = cfg->n_timeout_ * 1000;
  if (timeout_ms < 1100) 
	  timeout_ms = 1100;

  curl_easy_setopt(curl, CURLOPT_URL,            cfg->s_url_.c_str());

  curl_easy_setopt(curl, CURLOPT_POST,           1);
  curl_easy_setopt(curl, CURLOPT_PORT,           cfg->n_url_port_);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS,     cfg->s_post_data_.c_str());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,  cfg->post_data_size_);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION,  easy_recv_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA,      cfg);
  //curl_easy_setopt(curl, CURLOPT_READFUNCTION,   easy_send_cb);
  //curl_easy_setopt(curl, CURLOPT_READDATA,       p_conn);

  if (!cfg->b_ssl_enabel_) {
    // 非SSL加密传输
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
  } else {
    // SSL加密传输
  }

  curl_easy_setopt(curl, CURLOPT_TIMEOUT_MS,     timeout_ms);
  curl_easy_setopt(curl, CURLOPT_PRIVATE,        cfg);
  curl_easy_setopt(curl, CURLOPT_VERBOSE,        0L);
  //curl_easy_setopt(easy, CURLOPT_ERRORBUFFER,               error);
  //curl_easy_setopt(easy, CURLOPT_NOPROGRESS,                0L);
  //curl_easy_setopt(easy, CURLOPT_PROGRESSFUNCTION,          prog_cb);
  //curl_easy_setopt(easy, CURLOPT_PROGRESSDATA,              conn);

  CURLMcode rc = curl_multi_add_handle(p_curl_multi_, curl);
  LOG_INFO("Adding easy %p to multi %p (%s), %d\n",
           curl, p_curl_multi_, cfg->s_url_.c_str(), rc);
  return (rc == CURLM_OK);
}


static size_t OnReadCallBack(void *ptr, size_t size, size_t nmemb, void *stream) {
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

bool CMultiServer::PostImageFile(HttpConn *p_conn, 
                                 UserGetImgInfo *p_img_info, 
                                 std::string s_user, 
                                 std::string s_password) {
  if (p_conn == NULL) {
    return false;
  }

  CURL *curl = p_conn->curl();
  int timeout_ms = p_conn->n_timeout_ * 1000;
  if (timeout_ms < 1100)
    timeout_ms = 1100;

  CURLMcode rc;
  curl_easy_setopt(curl, CURLOPT_URL, p_conn->s_url_.c_str());
  curl_easy_setopt(curl, CURLOPT_PORT, p_conn->n_url_port_);

  curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
  curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
  curl_easy_setopt(curl, CURLOPT_USERNAME, s_user.c_str());
  curl_easy_setopt(curl, CURLOPT_PASSWORD, s_password.c_str());

  curl_easy_setopt(curl, CURLOPT_FTP_RESPONSE_TIMEOUT, 10L);

  curl_easy_setopt(curl, CURLOPT_READFUNCTION, OnReadCallBack);
  curl_easy_setopt(curl, CURLOPT_READDATA, p_img_info);

  curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, p_img_info->datasize);
  curl_easy_setopt(curl, CURLOPT_FTP_CREATE_MISSING_DIRS, 1L);

  curl_easy_setopt(curl, CURLOPT_APPEND, 1L);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, easy_recv_cb);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, p_conn);

  curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout_ms);
  curl_easy_setopt(curl, CURLOPT_PRIVATE, p_conn);
  curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);

  rc = curl_multi_add_handle(p_curl_multi_, curl);
  LOG_INFO("Adding easy %p to multi %p (%s), %d\n",
           curl, p_curl_multi_, p_conn->s_url_.c_str(), rc);
  return (rc == CURLM_OK);
}

bool CMultiServer::isSuccess(int errcode, std::string &serr) {
  if (CURLM_OK != errcode) {
    const char *s;
    switch (errcode) {
    case     CURLM_BAD_HANDLE:
      serr = "CURLM_BAD_HANDLE";
      break;
    case     CURLM_BAD_EASY_HANDLE:
      serr = "CURLM_BAD_EASY_HANDLE";
      break;
    case     CURLM_OUT_OF_MEMORY:
      serr = "CURLM_OUT_OF_MEMORY";
      break;
    case     CURLM_INTERNAL_ERROR:
      serr = "CURLM_INTERNAL_ERROR";
      break;
    case     CURLM_UNKNOWN_OPTION:
      serr = "CURLM_UNKNOWN_OPTION";
      break;
    case     CURLM_LAST:
      serr = "CURLM_LAST";
      break;
    case     CURLM_BAD_SOCKET:
      serr = "CURLM_BAD_SOCKET";
      break;
    default:
      serr = "CURLM_unknown";
      break;
    }
    return false;
  }
  return true;
}

/* CURLMOPT_SOCKETFUNCTION */
int CMultiServer::multi_sock_cb(CURL *e, curl_socket_t s, int what, void *cbp,
                                void *sockp) {
  CMultiServer *g = (CMultiServer*)cbp;
  CEvtCurl *fdp = (CEvtCurl*)sockp;
  if (what & (CURL_POLL_IN | CURL_POLL_OUT)) {
    if (sockp == NULL) {
      fdp = new CEvtCurl(s);
      if (NULL == fdp) {
        return -1;
      }
      curl_multi_assign(g->p_curl_multi_, s, (void *)fdp); // s与fdp关联
      fdp->c_evt_io_.Init(g->p_evt_loop_, CMultiServer::CEvtCurl::event_cb, g);
    }
  }

  LOG(L_INFO) << "multi_sock_cb " << what;
  switch (what) {
  case CURL_POLL_IN:
    if (fdp) {
      fdp->c_evt_io_.Stop();
      fdp->c_evt_io_.Start(fdp->fd_sock_, EVT_READ | EVT_PERSIST);
    }
    break;
  case CURL_POLL_OUT:
    if (fdp) {
      fdp->c_evt_io_.Stop();
      fdp->c_evt_io_.Start(fdp->fd_sock_, EVT_WRITE | EVT_PERSIST);
    }
    break;
  case CURL_POLL_INOUT:
    if (fdp) {
      fdp->c_evt_io_.Stop();
      fdp->c_evt_io_.Start(fdp->fd_sock_, EVT_READ | EVT_WRITE | EVT_PERSIST);
    }
    break;
  case CURL_POLL_REMOVE:
    if (fdp) {
      fdp->c_evt_io_.Stop();
      delete fdp;

      curl_multi_assign(g->p_curl_multi_, s, NULL);
    }
    break;
  default:
    LOG(FATAL) << "multi_sock_cb recv exception code " << what;
  }
  return 0;
}

/* Update the event timer after curl_multi library calls */
int CMultiServer::multi_timer_cb(CURLM *multi, long timeout_ms, CMultiServer *g) {
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
  LOG(L_WARNING) << "set timeout " << timeout_ms;
  g->c_evt_timer_.Start(timeout_ms, 0);
  return 0;
}

/* Called by libevent when our timeout expires */
int32 CMultiServer::timer_cb(SOCKET fd, short kind, const void *p_thiz) {
  LOG(L_WARNING) << "timeout cb";
  CMultiServer *p_multi = (CMultiServer*)(p_thiz);
  if (p_multi) {
    CURLMcode rc = curl_multi_socket_action(p_multi->p_curl_multi_,
                                            CURL_SOCKET_TIMEOUT,
                                            0,
                                            &p_multi->n_still_running_);
    mcode_or_die("timer_cb: curl_multi_socket_action", rc);
    p_multi->easy_check_info();
  }
  return 0;
}

size_t CMultiServer::easy_recv_cb(void *ptr,
                                  size_t size, size_t nmemb,
                                  void *p_easy) {
  if (!ptr || !p_easy) {
    LOG(L_ERROR) << "param is null.";
    return -1;
  }

  size_t realsize = size * nmemb;
  ((HttpConn*)p_easy)->s_resp_data_.append((char *)ptr, realsize);

  return realsize;
}

/* Check for completed transfers, and remove their easy handles */
void CMultiServer::easy_check_info() {
  //char        *eff_url;

  int          msgs_left;

  CURLMsg     *msg;
  CURLcode     res;

  CURL        *p_curl;
  HttpConn     *p_easy;

  LOG(L_INFO) << "REMAINING: " << n_still_running_;
  while ((msg = curl_multi_info_read(p_curl_multi_, &msgs_left))) {
    if (msg->msg == CURLMSG_DONE) {
      p_curl = msg->easy_handle;
      res    = msg->data.result;
      curl_easy_getinfo(p_curl, CURLINFO_PRIVATE,       &p_easy);
      //curl_easy_getinfo(p_curl, CURLINFO_EFFECTIVE_URL, &eff_url);
      curl_multi_remove_handle(p_curl_multi_, p_curl);

      if (p_easy->post_callback_) {
        p_easy->post_callback_->PostCallBack(p_easy, res);
      }
    } else {
      LOG(L_WARNING) << "msg->msg != CURLMSG_DONE ! It's " << msg->msg;
    }
  }
}

