/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#ifndef LIBVZLOGGING_VZLOGDEF_H_
#define LIBVZLOGGING_VZLOGDEF_H_

#include <stdio.h>
#define DEF_TAG_MARK              0x55AAEEFF

#define MAX_THREAD_A_PROCESS      16    // ������������߳���
#define MAX_WATCHDOG_A_PROCESS    4     // ����������࿴�Ź���
#define MAX_WATCHDOG_A_DEVICE     32    // ���豸����������

#define LEN_APP_NAME              32
#define LEN_DESCREBE              16    // �߳�����,��������һ�����̶��ι������

#define LEN_FILEPATH              256   // PATH+FILENAME
#define MAX_FILE_CNT              2     // ��¼�����ļ�����+1;
// �ļ���СΪһ��Ĭ����־���洢�ռ�

#define DEF_SERVER_HOST           "127.0.0.1"     // Ĭ�Ϸ�������ַ
#define DEF_SERVER_PORT           5760            // Ĭ�Ϸ������˿�

#define A_LOG_SIZE                1024            // ������־�ַ����ߴ�

#define MAX_LOG_FILE_SIZE         2 * 1024 * 1024  // Ĭ����־�ļ����洢�ռ�
#define MAX_WDG_FILE_SIZE         64 * 1024        // Ĭ�Ͽ��Ź��ļ����洢�ռ�

#ifdef WIN32
#ifndef snprintf
#define snprintf    _snprintf
#endif

#define DEF_RECORD_PATH     "d:/log/"         // Ĭ����־��¼·��
// ��־�ļ���һ��Ϊ��,�洢��Ϊ runing_0.log, runing_1.log
#define DEF_LOG_REC_FILE    "logfile"         // Ĭ����־�洢��
// ���Ź��ļ�,�洢��Ϊ watchdog.log
#define DEF_WDG_REC_FILE    "watchdog"        // Ĭ�Ͽ��Ź��洢��

#define DEF_WDG_MODULE_FILE "c:\\tools\\module.cfg"    // ���Ź����ģ�������ļ�

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

/* ��ȡ����ID */
unsigned int GetPid();
/* ��ȡ�߳�ID */
unsigned int GetPpid();
/* ��ȡϵͳ����ʱ�� */
unsigned int GetSysSec();
/* ���ļ�·���зָ���ļ��� */
const char  *GetFileName(const char *filepath);

/**/
/************************************************************************
* Description : ����־��ͷ
* Parameters  :
* Return      : ��ʹ��slog��ƫ����
************************************************************************/
int  VzLogHead(unsigned int level,
               const char *file, int line,
               char *slog, int nlog);

/*�����־��Ϣ*/
void VzLogPrint(const char* smsg, int nmsg);

/*����ʱ�����ӡ*/
int  VzLogDebug(const char *file, int line,
                const char *fmt, ...);

}  // namespace vzlogging

#ifdef __cplusplus
}
#endif

// �����ӡ
#define VZ_ERROR(VZ_FM, ...)   \
  vzlogging::VzLogDebug(__FILE__, __LINE__, VZ_FM, ##__VA_ARGS__)
#ifndef NDEBUG  // cmake Debugģʽ�¶���
#define VZ_PRINT(VZ_FM, ...)   \
  vzlogging::VzLogDebug(__FILE__, __LINE__, VZ_FM, ##__VA_ARGS__)
#else
#define VZ_PRINT(VZ_FM, ...)
#endif

#endif  // LIBVZLOGGING_VZLOGDEF_H_