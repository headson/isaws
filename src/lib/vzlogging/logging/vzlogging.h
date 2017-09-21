/************************************************************************
*Author      : Sober.Peng 19 : 5 : 2017
*Description : VZLOG    C��ʽ����ַ���
*              VZLOGB   C��ʽ���������
*              VZLOGSTR C++�������־
*************************************************************************/
#ifndef SRC_LIB_VZLOGGING_INCLUDE_VZLOGGING_H_
#define SRC_LIB_VZLOGGING_INCLUDE_VZLOGGING_H_

#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/************************************************************************
*Description : ��ʼ����־��
*Param       : argc[IN] Ӧ��������������
*              argv[IN] Ӧ�����������ַ�������
*Return      : 0 success, <0 failed
************************************************************************/
int  InitVzLogging(int argc, char* argv[]);

/************************************************************************
*Description : ������־��
*Param       :
*Return      :
************************************************************************/
void ExitVzLogging();

/************************************************************************
*Description : һֱ��ʾ��־
*Parameters :
*Return :
************************************************************************/
void ShowVzLoggingAlways();

/************************************************************************
*Author      : Sober.Peng 2017 - 5 - 19
*Description : ��ӡ��־
*Param       : n_level[IN] ��־�ȼ�
*              b_print[IN] ���ش�ӡ
*              p_file[IN]  ���ô˺����ļ�
*              n_line[IN]  ���ô˺����ļ���
*              p_fmt[IN]   ��ʽ���ַ���
*              ...[IN]     ����������
*Return      : 0 success, -1 failed
************************************************************************/
int VzLog(unsigned int  n_level,
          int           b_local_print,
          const char    *p_file,
          int           n_line,
          const char    *p_fmt,
          ...);
int VzLogBin(unsigned int n_level,
             int            b_local_print,
             const char     *p_file,
             int            n_line,
             const char     *p_data,
             int            n_size);

#ifdef __cplusplus
}
#endif

//////////////////////////////////////////////////////////////////////////
enum {
  L_DEBUG     = 0,
  L_INFO      = 1,
  L_WARNING   = 2,
  L_ERROR     = 3,
  L_HEARTBEAT = 4,
  L_RESET     = 5,
};
#define L_C_DEBUG     'D'
#define L_C_INFO      'I'
#define L_C_WARNING   'W'
#define L_C_ERROR     'E'
#define L_C_HEARTBEAT 'H'
#define L_C_RESET     'S'

/*C Style*/
#define VZLOG(V, VZ_FM, ...)        \
  VzLog(V, 0, __FILE__, __LINE__, VZ_FM, ##__VA_ARGS__)
#define VZDLOG(V, VZ_FM, ...)        \
  VzLog(V, 1, __FILE__, __LINE__, VZ_FM, ##__VA_ARGS__)

#define LOG_INFO(VZ_FM, ...)      VZLOG(L_INFO, VZ_FM, ##__VA_ARGS__)
#define LOG_WARNING(VZ_FM, ...)   VZLOG(L_WARNING, VZ_FM, ##__VA_ARGS__)
#define LOG_ERROR(VZ_FM, ...)     VZLOG(L_ERROR, VZ_FM, ##__VA_ARGS__)
#define LOG_RESET(VZ_FM, ...)     VZLOG(L_RESET, VZ_FM, ##__VA_ARGS__)

#define DLOG_INFO(VZ_FM, ...)     VZDLOG(L_INFO, VZ_FM, ##__VA_ARGS__)
#define DLOG_WARNING(VZ_FM, ...)  VZDLOG(L_WARNING, VZ_FM, ##__VA_ARGS__)
#define DLOG_ERROR(VZ_FM, ...)    VZDLOG(L_ERROR, VZ_FM, ##__VA_ARGS__)

/*C Style �����ƴ�ӡ*/
#define VZLOGB(V, data, size)       \
  VzLogBin(V, 0, __FILE__, __LINE__, data, size)
#define VZDLOGB(V, data, size)       \
  VzLogBin(V, 1, __FILE__, __LINE__, data, size)

#define LOGB_INFO(data, size)     VZLOGB(L_INFO,    data, size)
#define LOGB_WARING(data, size)   VZLOGB(L_WARNING, data, size)
#define LOGB_ERROR(data, size)    VZLOGB(L_ERROR,   data, size)

#define DLOGB_INFO(data, size)    VZDLOGB(L_INFO,    data, size)
#define DLOGB_WARING(data, size)  VZDLOGB(L_WARNING, data, size)
#define DLOGB_ERROR(data, size)   VZDLOGB(L_ERROR,   data, size)
#endif  // SRC_LIB_VZLOGGING_INCLUDE_VZLOGGING_H_

