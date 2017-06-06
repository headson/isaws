/************************************************************************/
/* Author      : Sober.Peng 19:5:2017
/* Description : VZLOG    C格式输出字符串
/*               VZLOGB   C格式输出二进制
/*               VZLOGSTR C++流输出日志
/************************************************************************/
#ifndef SRC_LIB_VZLOGGING_INCLUDE_VZLOGGINGCPP_H_
#define SRC_LIB_VZLOGGING_INCLUDE_VZLOGGINGCPP_H_

#include <time.h>
#include <string>

#include "vzlogging/logging/vzlogging.h"

/************************************************************************/
/* Description : C++流输出方式实现日志打包
/* Param       : level[IN] 日志等级
                 file[IN]  __FILE__
                 line[IN]  __LINE__
                 b_local_print[IN] true本地打印\不上传,
                                   false不直接打印(-v实现打印)\上传
/************************************************************************/
namespace vzlogging {

class CVzLogStream {
 public:
  CVzLogStream(int          n_level,
               const char*  p_file,
               int          n_line,
               unsigned int b_local_print);
  virtual ~CVzLogStream();

  CVzLogStream& operator<< (char t);
  CVzLogStream& operator<< (unsigned char t);
  CVzLogStream& operator<< (bool val);
  CVzLogStream& operator<< (short val);
  CVzLogStream& operator<< (unsigned short val);
  CVzLogStream& operator<< (int val);
  CVzLogStream& operator<< (unsigned int val);
  CVzLogStream& operator<< (long val);
  CVzLogStream& operator<< (unsigned long val);
  CVzLogStream& operator<< (long long val);
  CVzLogStream& operator<< (unsigned long long val);
  CVzLogStream& operator<< (float val);
  CVzLogStream& operator<< (double val);
  CVzLogStream& operator<< (long double val);

  // CVzLogStream& operator<< (char* t);
  CVzLogStream& operator<< (const char* t);
  CVzLogStream& operator<< (std::string str);

  //CVzLogStream& operator<< (time_t tt);

  CVzLogStream& write(const char* s_msg, int n_msg);
 private:
  void*         p_tls_;
  unsigned int  n_level_;
  unsigned int  b_local_print_;
};

}  // namespace vzlogging

/*C++ Style*/
#define VZLOGSTR(V)   vzlogging::CVzLogStream(V, __FILE__, __LINE__, false)
#define VZDLOGSTR(V)  vzlogging::CVzLogStream(V, __FILE__, __LINE__, true)

#define VZLOGSTR_INFO()     VZLOGSTR(L_INFO)
#define VZLOGSTR_WARNING()  VZLOGSTR(L_WARNING)
#define VZLOGSTR_ERROR()    VZLOGSTR(L_ERROR)

#define LOG(V)              VZLOGSTR(V)
#define DLOG(V)             VZDLOGSTR(V)
#endif  // SRC_LIB_VZLOGGING_INCLUDE_VZLOGGINGCPP_H_

