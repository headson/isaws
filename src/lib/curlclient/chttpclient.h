/************************************************************************
* Author      : SoberPeng 2017-09-02
* Description :
************************************************************************/
#ifndef LIBCURLCLIENT_CHTTPCLIENT_H
#define LIBCURLCLIENT_CHTTPCLIENT_H

#include <string>

#define HTTP_TIMEOUT_CONN    5
#define HTTP_TIMEOUT_RESP    5

class CHttpClient {
 public:
  CHttpClient(void);
  ~CHttpClient(void);
 public:
  /**
  * @brief HTTP POST请求
  * @param surl 输入参数,请求的Url地址,如:http://www.baidu.com
  * @param spost 输入参数,使用如下格式para1=val1?2=val2&…
  * @param sresp 输出参数,返回的内容
  * @return 返回是否Post成功
  */
  int Post(const std::string &surl, unsigned int nport,
           const std::string &spost, std::string &sresp);

  /**
  * @brief HTTP GET请求
  * @param surl 输入参数,请求的Url地址,如:http://www.baidu.com
  * @param sresp 输出参数,返回的内容
  * @return 返回是否Post成功
  */
  int Get(const std::string &surl, unsigned int nport,
          std::string &sresp);

  /**
  * @brief HTTPS POST请求,无证书版本
  * @param surl 输入参数,请求的Url地址,如:https://www.alipay.com
  * @param spost 输入参数,使用如下格式para1=val1?2=val2&…
  * @param sresp 输出参数,返回的内容
  * @param pCaPath 输入参数,为CA证书的路径.如果输入为NULL,则不验证服务器端证书的有效性.
  * @return 返回是否Post成功
  */
  int Posts(const std::string &surl, unsigned int nport,
            const std::string &spost, std::string &sresp,
            const char *pCaPath = NULL);

  /**
  * @brief HTTPS GET请求,无证书版本
  * @param surl 输入参数,请求的Url地址,如:https://www.alipay.com
  * @param sresp 输出参数,返回的内容
  * @param pCaPath 输入参数,为CA证书的路径.如果输入为NULL,则不验证服务器端证书的有效性.
  * @return 返回是否Post成功
  */
  int Gets(const std::string &surl, unsigned int nport,
           std::string &sresp,
           const char *pCaPath = NULL);

 public:
  void SetDebug(bool bDebug);
  static const char *GetErr(int error);

 private:
  unsigned int is_debug_;
};

#endif  // LIBCURLCLIENT_CHTTPCLIENT_H
