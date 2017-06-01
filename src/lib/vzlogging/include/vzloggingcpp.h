/************************************************************************/
/* Author      : Sober.Peng 19:5:2017
/* Description : VZLOG    C��ʽ����ַ���
/*               VZLOGB   C��ʽ���������
/*               VZLOGSTR C++�������־
/************************************************************************/
#ifndef SRC_LIB_VZLOGGING_INCLUDE_VZLOGGINGCPP_H_
#define SRC_LIB_VZLOGGING_INCLUDE_VZLOGGINGCPP_H_

#include <time.h>
#include <string>

#include "vzlogging/include/vzlogging.h"

/************************************************************************/
/* Description : C++�������ʽʵ����־���
/* Param       : level[IN] ��־�ȼ�
                 file[IN]  __FILE__
                 line[IN]  __LINE__
                 b_local_print[IN] true���ش�ӡ\���ϴ�,
                                   false��ֱ�Ӵ�ӡ(-vʵ�ִ�ӡ)\�ϴ�
/************************************************************************/
namespace vzlogging {

class CVzLogStream {
 public:
  CVzLogStream(int          n_level,
               const char*  p_file,
               int          n_line,
               unsigned int b_local_print);
  virtual ~CVzLogStream();

  CVzLogStream& operator<<(const int t);
  CVzLogStream& operator<<(const unsigned int t);
#ifndef WIN32  // win32����time_t��ͻ
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

/*C++ Style*/
#define VZLOGSTR(V)   vzlogging::CVzLogStream(V, __FILE__, __LINE__, false)
#define VZDLOGSTR(V)  vzlogging::CVzLogStream(V, __FILE__, __LINE__, true)

#define VZLOGSTR_INFO()     VZLOGSTR(L_INFO)
#define VZLOGSTR_WARNING()  VZLOGSTR(L_WARNING)
#define VZLOGSTR_ERROR()    VZLOGSTR(L_ERROR)

#define LOG(V)              VZLOGSTR(V)
#define DLOG(V)             VZDLOGSTR(V)

#endif  // SRC_LIB_VZLOGGING_INCLUDE_VZLOGGINGCPP_H_

