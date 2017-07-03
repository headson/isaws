/************************************************************************
*Author      : Sober.Peng 17-07-03
*Description :
************************************************************************/
#include "ccurleasy.h"
#include "ccurlmulti.h"

#include "vzlogging/logging/vzloggingcpp.h"

CCurlEasy::CCurlEasy()
  : p_curl_easy_(NULL)
  , p_curl_multi_(NULL) {

}

CCurlEasy::~CCurlEasy() {
  printf("~ConnInfo\n");
  fflush(stdout);
}

bool CCurlEasy::Start(CCurlMulti* p_curl_multi, const char *p_url, const char *p_post_data) {
  p_curl_easy_ = curl_easy_init();
  if (!p_curl_easy_) {
    LOG(L_ERROR) << "curl_easy_init() failed, exiting!";
    return false;
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
  //curl_easy_setopt(easy, CURLOPT_ERRORBUFFER,         error);
  curl_easy_setopt(p_curl_easy_, CURLOPT_PRIVATE,       this);
  //curl_easy_setopt(easy, CURLOPT_NOPROGRESS,          0L);
  //curl_easy_setopt(easy, CURLOPT_PROGRESSFUNCTION,    prog_cb);
  //curl_easy_setopt(easy, CURLOPT_PROGRESSDATA,        conn);

  rc = curl_multi_add_handle(p_curl_multi_->p_curl_multi_, p_curl_easy_);
  LOG_INFO("Adding easy %p to multi %p (%s), %d\n",
          p_curl_easy_, p_curl_multi_, p_url, rc);
  return (rc==CURLM_OK);
}

/* CURLOPT_WRITEFUNCTION */
size_t CCurlEasy::ret_resp_cb(void *ptr, size_t size, size_t nmemb, void *data) {
  size_t realsize = size * nmemb;
  CCurlEasy *conn = (CCurlEasy*)data;
  printf("---------------- resp %s.\n", (char*)ptr);
  return (size * nmemb);
}

