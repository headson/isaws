/************************************************************************
*Author      : Sober.Peng 17-09-24
*Description :
************************************************************************/
#ifndef LIBCURLCLIENT_CHTTPEASYASYNC_H
#define LIBCURLCLIENT_CHTTPEASYASYNC_H

#include "curlclient/base/basedefines.h"
#include "vzbase/base/boost_settings.hpp"

#include "curl/curl.h"
#include "cmultiserver.h"

class CHttpEasyAsync : boost::noncopyable,
  boost::enable_shared_from_this<CHttpEasyAsync> {
 public:
   boost::shared_ptr<CHttpEasyAsync> Ptr;

 public:
  CHttpEasyAsync();
  ~CHttpEasyAsync();
  
 protected:
   CURL       *easy_;
   curl_slist *slist_;
};

#endif  // LIBCURLCLIENT_CHTTPEASYASYNC_H
