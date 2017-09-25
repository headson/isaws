/************************************************************************
*Author      : Sober.Peng 17-09-25
*Description :
************************************************************************/
#include "chttpasync.h"
#include "basedefines.h"

#include "cmultiserver.h"
#include "vzbase/helper/stdafx.h"

CHttpAsync::CHttpAsync(CHttpRespose *post_callback)
  : post_callback_(post_callback) {
  clist_ = NULL;
  cpost_ = NULL;
  ceasy_ = curl_easy_init();
}

CHttpAsync::~CHttpAsync() {
  if (clist_) {
    curl_slist_free_all(clist_); /* free the list again */
    clist_ = NULL;
  }
  if (cpost_) {
    curl_formfree(cpost_);
    cpost_ = NULL;
  }
  if (ceasy_) {
    curl_easy_cleanup(ceasy_);
    ceasy_ = NULL;
  }
}

bool CHttpAsync::Post(std::string   url,
                      int           url_port,
                      const char   *post_data,
                      int           post_data_size, 
                      CMultiServer *multi_server) {
  if (NULL == multi_server) {
    LOG(L_ERROR) << "curl TryPop is NULL, exiting!";
    return false;
  }

  // 发送数据
  resp_data_ = "";
  post_pos_  = 0;
  post_data_.resize(post_data_size);
  memcpy(&post_data_[0], post_data, post_data_size);

  // 超时
  int resp_timeout = DEF_RESPONSE_TIMEOUT;
  if (resp_timeout < 1100)
    resp_timeout = 1100;

  curl_easy_setopt(ceasy_, CURLOPT_URL,           url.c_str());
  curl_easy_setopt(ceasy_, CURLOPT_PORT,          url_port);

  clist_ = curl_slist_append(clist_, "Expect:");
  clist_ = curl_slist_append(clist_, "Content-Type: application/Json");
  curl_easy_setopt(ceasy_, CURLOPT_HTTPHEADER,     clist_);

  curl_easy_setopt(ceasy_, CURLOPT_POST,          1);

  curl_easy_setopt(ceasy_, CURLOPT_WRITEFUNCTION, easy_write_cb);
  curl_easy_setopt(ceasy_, CURLOPT_WRITEDATA,     this);

  curl_easy_setopt(ceasy_, CURLOPT_READFUNCTION,  easy_read_cb);
  curl_easy_setopt(ceasy_, CURLOPT_READDATA,      this);

  //curl_easy_setopt(ceasy_, CURLOPT_POSTFIELDS,    post_data_.c_str());
  //curl_easy_setopt(ceasy_, CURLOPT_POSTFIELDSIZE, post_data_.size());

  //if (conn->ssl_enabel_ > 0) {
  //  LOG(L_WARNING) << "ssl send ----------------------- " << conn->url_port_
  //    << " ca path " << conn->ssl_ca_path_;
  //  // SSL加密传输
  //  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
  //  curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

  //  if (!conn->ssl_ca_path_.empty()) {
  //    curl_easy_setopt(curl, CURLOPT_SSLCERT, conn->ssl_ca_path_.c_str());
  //  }
  //}

  curl_easy_setopt(ceasy_, CURLOPT_TIMEOUT_MS,       resp_timeout);
  curl_easy_setopt(ceasy_, CURLOPT_PRIVATE,          this);
  curl_easy_setopt(ceasy_, CURLOPT_VERBOSE,          0L);
  //curl_easy_setopt(easy, CURLOPT_ERRORBUFFER,      error);
  //curl_easy_setopt(easy, CURLOPT_NOPROGRESS,       0L);
  //curl_easy_setopt(easy, CURLOPT_PROGRESSFUNCTION, prog_cb);
  //curl_easy_setopt(easy, CURLOPT_PROGRESSDATA,     conn);

  CURLMcode rc = curl_multi_add_handle(multi_server->curlm(), ceasy_);
  LOG_INFO("Adding easy %p to multi %p (%s), %d\n",
           ceasy_, multi_server->curlm(), url.c_str(), rc);
  return (rc == CURLM_OK);
}

size_t CHttpAsync::easy_write_cb(void *ptr, size_t size, size_t nmemb, void *pthis) {
  if (!ptr || !pthis) {
    LOG(L_ERROR) << "param is null.";
    return -1;
  }

  size_t realsize = size * nmemb;
  ((CHttpAsync*)pthis)->resp_data_.append((char *)ptr, realsize);

  return realsize;
}

size_t CHttpAsync::easy_read_cb(void *ptr, size_t size, size_t nitems, void *pthiz) {
  CHttpAsync *psrv = (CHttpAsync *)pthiz;
  if (psrv == NULL) {
    return -1;
  }

  int nrlen = psrv->post_data_.size() - psrv->post_pos_;
  if (nrlen > size * nitems)
    nrlen = size * nitems;
  memcpy(ptr, &psrv->post_data_[psrv->post_pos_], nrlen);
  psrv->post_pos_ += nrlen;
  return nrlen;
}

