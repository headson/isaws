/************************************************************************/
/* Author      : Sober.Peng 19:5:2017
/* Description : VZLOG    C格式输出字符串
/*               VZLOGB   C格式输出二进制
/*               VZLOGSTR C++流输出日志
/************************************************************************/
#ifndef SRC_LIB_VZLOGGING_INCLUDE_VZLOGGING_H_
#define SRC_LIB_VZLOGGING_INCLUDE_VZLOGGING_H_

#include <time.h>
#include <string>

namespace vzlogging {

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************/
/* Author      : Sober.Peng 2017-5-19
/* Description : 初始化日志库
/* Param       : argc[IN] 应用启动参数个数
/*               argv[IN] 应用启动参数字符串数组
/* Return      : 0 success, <0 failed
/************************************************************************/
int  InitVzLogging(int argc, char* argv[]);

/************************************************************************/
/* Author      : Sober.Peng 2017-5-19
/* Description : 销毁日志库
/* Param       :
/* Return      :
/************************************************************************/
void ExitVzLogging();

/************************************************************************/
/* Author      : Sober.Peng 2017-5-19
/* Description : 打印日志
/* Param       : n_level[IN] 日志等级
/*               b_print[IN] 本地打印
/*               p_file[IN]  调用此函数文件
/*               n_line[IN]  调用此函数文件行
/*               p_fmt[IN]   格式化字符串
/*               ...[IN]     不定长参数
/* Return      : 0 success,-1 failed
/************************************************************************/
int VzLog(unsigned int  n_level,
          bool          b_local_print,
          const char    *p_file,
          int           n_line,
          const char    *p_fmt,
          ...);
int VzLogBin(unsigned int n_level,
           bool           b_local_print,
           const char     *p_file,
           int            n_line,
           const char     *p_data,
           int            n_size);

#ifdef __cplusplus
}
#endif

/************************************************************************/
/* Description : C++流输出方式实现日志打包
/* Param       : level[IN] 日志等级
                 file[IN]  __FILE__
                 line[IN]  __LINE__
                 b_local_print[IN] true本地打印\不上传,
                                   false不直接打印(-v实现打印)\上传
/************************************************************************/
class CVzLogStream {
 public:
  CVzLogStream(int         n_level,
               const char* p_file,
               int         n_line,
               bool        b_local_print);
  virtual ~CVzLogStream();

  CVzLogStream& operator<<(const int t);
  CVzLogStream& operator<<(const unsigned int t);
#ifndef WIN32  // win32中与time_t冲突
  CVzLogStream& operator<<(const long long t);
#endif  // WIN32
  CVzLogStream& operator<<(const unsigned long long t);
  CVzLogStream& operator<<(const double t);
  CVzLogStream& operator<<(const char t);
  CVzLogStream& operator<<(const char* t);

  CVzLogStream& operator<<(const time_t tt);
  CVzLogStream& operator<<(const std::string str);

  CVzLogStream& write(const char* s_msg, int n_msg);

 private:
  void*         p_tls_;
  bool          b_print_;
  unsigned int  n_level_;
};

}  // namespace vzlogging

//////////////////////////////////////////////////////////////////////////
enum {
  L_DEBUG     = 0,
  L_INFO      = 1,
  L_WARNING   = 2,
  L_ERROR     = 3,
  L_HEARTBEAT = 4,
};
#define L_C_DEBUG     'D'
#define L_C_INFO      'I'
#define L_C_WARNING   'W'
#define L_C_ERROR     'E'
#define L_C_HEARTBEAT 'H'

/*C Style*/
#define VZLOG(V, VZ_FM, ...)        \
  vzlogging::VzLog(V, false, __FILE__, __LINE__, VZ_FM, ##__VA_ARGS__)

#define VZDLOG(V, VZ_FM, ...)        \
  vzlogging::VzLog(V, true, __FILE__, __LINE__, VZ_FM, ##__VA_ARGS__)

#define LOG_INFO(VZ_FM, ...)      VZLOG(L_INFO, VZ_FM, ##__VA_ARGS__)
#define LOG_WARNING(VZ_FM, ...)   VZLOG(L_WARNING, VZ_FM, ##__VA_ARGS__)
#define LOG_ERROR(VZ_FM, ...)     VZLOG(L_ERROR, VZ_FM, ##__VA_ARGS__)

#define DLOG_INFO(VZ_FM, ...)     VZDLOG(L_INFO, VZ_FM, ##__VA_ARGS__)
#define DLOG_WARNING(VZ_FM, ...)  VZDLOG(L_WARNING, VZ_FM, ##__VA_ARGS__)
#define DLOG_ERROR(VZ_FM, ...)    VZDLOG(L_ERROR, VZ_FM, ##__VA_ARGS__)

/*C Style 二进制打印*/
#define VZLOGB(V, data, size)       \
  vzlogging::VzLogBin(V, false, __FILE__, __LINE__, data, size)
#define VZDLOGB(V, data, size)       \
  vzlogging::VzLogBin(V, true, __FILE__, __LINE__, data, size)

#define LOGB_INFO(data, size)     VZLOGB(L_INFO,    data, size)
#define LOGB_WARING(data, size)   VZLOGB(L_WARNING, data, size)
#define LOGB_ERROR(data, size)    VZLOGB(L_ERROR,   data, size)

#define DLOGB_INFO(data, size)    VZDLOGB(L_INFO,    data, size)
#define DLOGB_WARING(data, size)  VZDLOGB(L_WARNING, data, size)
#define DLOGB_ERROR(data, size)   VZDLOGB(L_ERROR,   data, size)

/*C++ Style*/
#define VZLOGSTR(V)   vzlogging::CVzLogStream(V, __FILE__, __LINE__, false)
#define VZDLOGSTR(V)  vzlogging::CVzLogStream(V, __FILE__, __LINE__, true)

#define VZLOGSTR_INFO()     VZLOGSTR(L_INFO)
#define VZLOGSTR_WARNING()  VZLOGSTR(L_WARNING)
#define VZLOGSTR_ERROR()    VZLOGSTR(L_ERROR)

#define LOG(V)              VZLOGSTR(V)
#define DLOG(V)             VZDLOGSTR(V)

#endif  // SRC_LIB_VZLOGGING_INCLUDE_VZLOGGING_H_

