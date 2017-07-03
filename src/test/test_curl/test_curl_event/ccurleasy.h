/************************************************************************
*Author      : Sober.Peng 17-07-03
*Description : 
************************************************************************/
#ifndef CURL_LIBEVENT_CURLCONN_H_
#define CURL_LIBEVENT_CURLCONN_H_

#include "vzbase/base/basictypes.h"

#include <curl/curl.h>
#include "vzconn/base/clibevent.h"

class CCurlMulti;
class CCurlEasy {
public:
  CCurlEasy();
  ~CCurlEasy();

public:
  bool Start(CCurlMulti* p_curl_multi,
             const char *p_url,
             const char *p_post_data);

public:
  static size_t ret_resp_cb(void *ptr, size_t size, size_t nmemb, void *data);

public:
  CURL        *p_curl_easy_;
  CCurlMulti  *p_curl_multi_;
};

#endif  // CURL_LIBEVENT_CURLCONN_H_
