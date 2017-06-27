/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : 
************************************************************************/
#ifndef LIBVZLOGGING_VZLOGDEF_H_
#define LIBVZLOGGING_VZLOGDEF_H_

#include <stdio.h>
#define DEF_TAG_MARK             0x55AAEEFF

#define DEF_PER_PRO_THREAD_MAX    16    // ������������߳���
#define DEF_PER_PRO_WATCHDOG_MAX  4     // ����������࿴�Ź���
#define DEF_PER_DEV_PROCESS_MAX   32    // ���豸����������

#define DEF_PROCESS_NAME_MAX      32
#define DEF_USER_DESCREBE_MAX     16

#define DEF_LOG_FILE_NAME         256   // PATH+FILENAME
#define DEF_ERR_FILE_COUNT        2     // ��¼�����ļ�����+1;
// �ļ���СΪһ��Ĭ����־���洢�ռ�

#define DEF_SERVER_HOST          "127.0.0.1"     // Ĭ�Ϸ�������ַ
#define DEF_SERVER_PORT          5760            // Ĭ�Ϸ������˿�

#define DEF_LOG_MAX_SIZE         1024            // ������־�ַ����ߴ�

#define DEF_LOG_FILE_SIZE        2 * 1024 * 1024  // Ĭ����־�ļ����洢�ռ�
#define DEF_WDG_FILE_SIZE        64 * 1024        // Ĭ�Ͽ��Ź��ļ����洢�ռ�

#ifdef WIN32
#ifndef snprintf
#define snprintf    _snprintf
#endif

#define DEF_RECORD_PATH     "d:/log/"         // Ĭ����־��¼·��
// ��־�ļ���һ��Ϊ��,�洢��Ϊ runing_0.log, runing_1.log
#define DEF_LOG_REC_FILE    "logfile"         // Ĭ����־�洢��
// ���Ź��ļ���һ��Ϊ��,�洢��Ϊ watchdog_0.log, watchdog_1.log
#define DEF_WDG_REC_FILE    "watchdog"        // Ĭ�Ͽ��Ź��洢��

#define DEF_WDG_MODULE_FILE "E:/vz/git_work/src/run/vzlogserver/module.cfg"    // ���Ź����ģ�������ļ�

#else
typedef int         SOCKET;

#define DEF_RECORD_PATH     "/mnt/log/system_server_files/"
#define DEF_LOG_REC_FILE    "logfile"
#define DEF_WDG_REC_FILE    "watchdog"

#define DEF_WDG_MODULE_FILE "/mnt/usr/module.cfg"
#endif

#ifdef __cplusplus
extern "C" {
#endif
#ifdef WIN32
void gettimeofday(struct timeval *tp, struct timezone *tz);
#endif

namespace vzlogging {

unsigned int GetPid();
unsigned int GetPpid();
const   char *GetFileName(const char *filepath);

/*���ͷ*/
int          VzLogPackHead(unsigned int level,
                           const char   *file,
                           int          line,
                           char*        slog,
                           int          nlog);

/*����ʱ�����ӡ*/
int          VzLogPrintFailed(const char  *file,
                              int         line,
                              const char *fmt,
                              ...);
/*�����־��Ϣ*/
void         VzDumpLogging(const char* s_msg, int n_msg);

}  // namespace vzlogging

#ifdef __cplusplus
}
#endif

// �����ӡ
#define VZ_ERROR(VZ_FM, ...)   \
  vzlogging::VzLogPrintFailed(__FILE__, __LINE__, VZ_FM, ##__VA_ARGS__)
#ifndef NDEBUG
#define VZ_PRINT(VZ_FM, ...)   \
  vzlogging::VzLogPrintFailed(__FILE__, __LINE__, VZ_FM, ##__VA_ARGS__)
#else
#define VZ_PRINT(VZ_FM, ...)
#endif

#endif  // LIBVZLOGGING_VZLOGDEF_H_
