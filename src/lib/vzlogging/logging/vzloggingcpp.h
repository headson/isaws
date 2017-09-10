/************************************************************************
*Author      : Sober.Peng 19:5:2017
*Description : VZLOG    C格式输出字符串
*              VZLOGB   C格式输出二进制
*              VZLOGSTR C++流输出日志
************************************************************************/
#ifndef SRC_LIB_VZLOGGING_INCLUDE_VZLOGGINGCPP_H_
#define SRC_LIB_VZLOGGING_INCLUDE_VZLOGGINGCPP_H_

#include <time.h>
#include <string>

#include "vzlogging/logging/vzlogging.h"

/************************************************************************
*Description : C++流输出方式实现日志打包
*Param       : level[IN] 日志等级
*                file[IN]  __FILE__
*                line[IN]  __LINE__
*                b_local_print[IN] true本地打印\不上传,
*                                  false不直接打印(-v实现打印)\上传
*************************************************************************/
namespace vzlogging {

class CVzLogStream {
 public:
  CVzLogStream(int n_level,
               const char* p_file, int n_line,
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

#if 0
#define LOG(V)              VZLOGSTR(V)
#define DLOG(V)             VZDLOGSTR(V)
#else
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
class FatalMsg {
public:
  FatalMsg(const char *filename, int in_file_line)
    : file_name_(filename),
    in_file_line_(in_file_line) {
    msg_buf_[0] = '\0';
    back_ = 0;
  }
  ~FatalMsg() {
    msg_buf_[back_++] = '\n';
    msg_buf_[back_] = '\0';
    vzlogging::CVzLogStream(L_ERROR, file_name_,
                            in_file_line_,
                            false) << msg_buf_;
    fprintf(stderr,
            "%s\n\n\n\t%s:%d  fatal message!!\n\n\n",
            msg_buf_, shortname(file_name_), in_file_line_);
    abort();
  }
  const char *shortname(const char *path) {
    if (path == NULL) return NULL;
    const char *short_name = path;
    for (int i = strlen(path) - 1; i >= 0; --i) {
      if (path[i] == '\\' || path[i] == '/') {
        short_name = &path[i + 1];
        break;
      }
    }
    return short_name;
  }

#ifdef WIN32
#define jsnprintf _snprintf
#else
#define jsnprintf snprintf
#endif

  FatalMsg & operator<<(const char *msg) {
    int ret = jsnprintf(msg_buf_ + back_, kMaxMsgBufSize - back_,
                        "%s", msg);
    if (ret > 0) back_ += ret;
    return *this;
  }
  FatalMsg & operator<<(long msg) {
    int ret = jsnprintf(msg_buf_ + back_, kMaxMsgBufSize - back_,
                        "%ld", msg);
    if (ret > 0) back_ += ret;
    return *this;
  }

private:
  static const int kMaxMsgBufSize = 1024;

private:
  const char *file_name_;
  int in_file_line_;
  char msg_buf_[kMaxMsgBufSize];
  int back_;
};

class LogMsgVoidify {
public:
  LogMsgVoidify() {}
  // This has to be an operator with a precedence lower than << but
  // higher than ?:
  void operator&(vzlogging::CVzLogStream&) {}
};

#define JLOG_INFO()         VZLOGSTR(L_INFO)
#define JLOG_WARNING()      VZLOGSTR(L_WARNING)
#define JLOG_ERROR()        VZLOGSTR(L_ERROR)
#define JLOG_L_INFO()         VZLOGSTR(L_INFO)
#define JLOG_L_WARNING()      VZLOGSTR(L_WARNING)
#define JLOG_L_ERROR()        VZLOGSTR(L_ERROR)
#define JLOG_FATAL()        FatalMsg(__FILE__, __LINE__)

#define LOG(LEVEL)          JLOG_##LEVEL()
#define DLOG(LEVEL)         VZDLOGSTR(LEVEL)

#if defined(_DEBUG)
#define JDLOG(LEVEL)        JLOG_##LEVEL()
#else
#define JDLOG(LEVEL)  1 ? (void)0 : LogMsgVoidify() & JLOG_##LEVEL()
#endif

#endif  // 0
#endif  // SRC_LIB_VZLOGGING_INCLUDE_VZLOGGINGCPP_H_
