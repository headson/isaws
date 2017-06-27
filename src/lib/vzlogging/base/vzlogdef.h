/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : 
************************************************************************/
#ifndef LIBVZLOGGING_VZLOGDEF_H_
#define LIBVZLOGGING_VZLOGDEF_H_

#include <stdio.h>
#define DEF_TAG_MARK             0x55AAEEFF

#define DEF_PER_PRO_THREAD_MAX    16    // 单进程中最多线程数
#define DEF_PER_PRO_WATCHDOG_MAX  4     // 单进程中最多看门狗数
#define DEF_PER_DEV_PROCESS_MAX   32    // 单设备中最多进程数

#define DEF_PROCESS_NAME_MAX      32
#define DEF_USER_DESCREBE_MAX     16

#define DEF_LOG_FILE_NAME         256   // PATH+FILENAME
#define DEF_ERR_FILE_COUNT        2     // 记录错误文件个数+1;
// 文件大小为一半默认日志最大存储空间

#define DEF_SERVER_HOST          "127.0.0.1"     // 默认服务器地址
#define DEF_SERVER_PORT          5760            // 默认服务器端口

#define DEF_LOG_MAX_SIZE         1024            // 单条日志字符最大尺寸

#define DEF_LOG_FILE_SIZE        2 * 1024 * 1024  // 默认日志文件最大存储空间
#define DEF_WDG_FILE_SIZE        64 * 1024        // 默认看门狗文件最大存储空间

#ifdef WIN32
#ifndef snprintf
#define snprintf    _snprintf
#endif

#define DEF_RECORD_PATH     "d:/log/"         // 默认日志记录路径
// 日志文件会一分为二,存储名为 runing_0.log, runing_1.log
#define DEF_LOG_REC_FILE    "logfile"         // 默认日志存储名
// 看门狗文件会一分为二,存储名为 watchdog_0.log, watchdog_1.log
#define DEF_WDG_REC_FILE    "watchdog"        // 默认看门狗存储名

#define DEF_WDG_MODULE_FILE "E:/vz/git_work/src/run/vzlogserver/module.cfg"    // 看门狗监控模块配置文件

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

/*组包头*/
int          VzLogPackHead(unsigned int level,
                           const char   *file,
                           int          line,
                           char*        slog,
                           int          nlog);

/*运行时错误打印*/
int          VzLogPrintFailed(const char  *file,
                              int         line,
                              const char *fmt,
                              ...);
/*输出日志信息*/
void         VzDumpLogging(const char* s_msg, int n_msg);

}  // namespace vzlogging

#ifdef __cplusplus
}
#endif

// 错误打印
#define VZ_ERROR(VZ_FM, ...)   \
  vzlogging::VzLogPrintFailed(__FILE__, __LINE__, VZ_FM, ##__VA_ARGS__)
#ifndef NDEBUG
#define VZ_PRINT(VZ_FM, ...)   \
  vzlogging::VzLogPrintFailed(__FILE__, __LINE__, VZ_FM, ##__VA_ARGS__)
#else
#define VZ_PRINT(VZ_FM, ...)
#endif

#endif  // LIBVZLOGGING_VZLOGDEF_H_
