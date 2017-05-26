/************************************************************************/
/* Author      : Sober.Peng 19:5:2017
/* Description : VZLOG    C��ʽ����ַ���
/*               VZLOGB   C��ʽ���������
/*               VZLOGSTR C++�������־
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
/* Description : ��ʼ����־��
/* Param       : argc[IN] Ӧ��������������
/*               argv[IN] Ӧ�����������ַ�������
/* Return      : 0 success, <0 failed
/************************************************************************/
int  InitVzLogging(int argc, char* argv[]);

/************************************************************************/
/* Author      : Sober.Peng 2017-5-19
/* Description : ������־��
/* Param       :
/* Return      :
/************************************************************************/
void ExitVzLogging();

/************************************************************************/
/* Author      : Sober.Peng 2017-5-19
/* Description : ��ӡ��־
/* Param       : n_level[IN] ��־�ȼ�
/*               b_print[IN] ���ش�ӡ
/*               p_file[IN]  ���ô˺����ļ�
/*               n_line[IN]  ���ô˺����ļ���
/*               p_fmt[IN]   ��ʽ���ַ���
/*               ...[IN]     ����������
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
/* Description : C++�������ʽʵ����־���
/* Param       : level[IN] ��־�ȼ�
                 file[IN]  __FILE__
                 line[IN]  __LINE__
                 b_local_print[IN] true���ش�ӡ\���ϴ�,
                                   false��ֱ�Ӵ�ӡ(-vʵ�ִ�ӡ)\�ϴ�
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

/*C Style �����ƴ�ӡ*/
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

