/************************************************************************
*Author      : Sober.Peng 17-09-25
*Description :
************************************************************************/
#ifndef CURL_CLIENT_CHTTPASYNC_H_
#define CURL_CLIENT_CHTTPASYNC_H_

#include <string>
#include <vector>

#include <curl/curl.h>

#include "cmultiserver.h"
#include "vzbase/base/boost_settings.hpp"

class CHttpAsync;

class CHttpRespose {
 public:
  virtual void RespCallvack(CHttpAsync *cfg, int errcode) = 0;
};

class CHttpAsync : boost::noncopyable {
 public:
  CHttpAsync(CHttpRespose *resp_callback);
  ~CHttpAsync();

  bool Post(std::string   url,
            int           url_port,
            const char   *post_data,
            int           post_data_size, 
            CMultiServer *multi_server);

 private:
  /* CURLOPT_WRITEFUNCTION 从服务器接收数据 */
  static size_t easy_write_cb(void *ptr,
                              size_t size, size_t nmemb,
                              void *pthiz);

  /* CURLOPT_READFUNCTION 发送数据到服务器 */
  static size_t easy_read_cb(void *ptr,
                             size_t size, size_t nmemb,
                             void *pthiz);

 public:
  CHttpRespose          *post_callback_;    // 回调

 public:
  struct curl_slist     *clist_;
  struct curl_httppost  *cpost_;
  CURL                  *ceasy_;            // curl 句柄

 public:
  unsigned int           post_pos_;         // 
  std::vector<char>      post_data_;        // 
  std::string            resp_data_;        // 返回数据
};


#endif  // CURL_CLIENT_CHTTPASYNC_H_
