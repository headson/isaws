/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#ifndef LIBVZLOGGING_VZLOGDEF_H_
#define LIBVZLOGGING_VZLOGDEF_H_

#include <stdio.h>
#define DEF_TAG_MARK              0x55AAEEFF

#define MAX_THREAD_A_PROCESS      16    // 单进程中最多线程数
#define MAX_WATCHDOG_A_PROCESS    4     // 单进程中最多看门狗数
#define MAX_WATCHDOG_A_DEVICE     32    // 单设备中最多进程数
#define MAX_WATCHDOG_TIMEOUT      180  /* 3minute */

#define LEN_APP_NAME              32
#define LEN_DESCREBE              12    // 线程描述,用于区分一个进程多个喂狗调用

#define LEN_FILEPATH              256   // PATH+FILENAME
#define MAX_FILE_CNT              2     // 记录错误文件个数+1;
// 文件大小为一半默认日志最大存储空间

#define DEF_SERVER_HOST           "127.0.0.1"     // 默认服务器地址
#define DEF_SERVER_PORT           5760            // 默认服务器端口

#define A_LOG_SIZE                1024            // 单条日志字符最大尺寸

#define MAX_LOG_FILE_SIZE         2 * 1024 * 1024  // 默认日志文件最大存储空间
#define MAX_WDG_FILE_SIZE         64 * 1024        // 默认看门狗文件最大存储空间

#ifdef WIN32
#ifndef snprintf
#define snprintf    _snprintf
#endif

#define DEF_RECORD_PATH     "d:/log/"         // 默认日志记录路径
// 日志文件会一分为二,存储名为 runing_0.log, runing_1.log
#define DEF_LOG_REC_FILE    "logfile"         // 默认日志存储名
// 看门狗文件,存储名为 watchdog.log
#define DEF_WDG_REC_FILE    "watchdog"        // 默认看门狗存储名

#define DEF_WDG_MODULE_FILE "c:\\tools\\module.cfg"    // 看门狗监控模块配置文件
#define DEF_WDG_SHM_ARG     "c:\\tools\\wdg_shm_arg"
#else
typedef int         SOCKET;

#define DEF_RECORD_PATH     "/mnt/log/system_server_files/"
#define DEF_LOG_REC_FILE    "logfile"
#define DEF_WDG_REC_FILE    "watchdog"

#define DEF_WDG_MODULE_FILE "/mnt/usr/module.cfg"
#define DEF_WDG_SHM_ARG     "/dev/shm/wdg_shm_arg"
#endif

#ifdef __cplusplus
extern "C" {
#endif
#ifdef WIN32
void gettimeofday(struct timeval *tp, struct timezone *tz);
#endif

namespace vzlogging {

/* 获取进程ID */
unsigned int GetPid();
/* 获取线程ID */
unsigned int GetPpid();
/* 获取系统启动时间 */
unsigned int GetSysSec();
/* 从文件路径中分割出文件名 */
const char  *GetFileName(const char *filepath);

/**/
/************************************************************************
* Description : 组日志包头
* Parameters  :
* Return      : 已使用slog的偏移量
************************************************************************/
int  VzLogHead(unsigned int level,
               const char *file, int line,
               char *slog, int nlog);

/*输出日志信息*/
void VzLogPrint(const char* smsg, int nmsg);

/*运行时错误打印*/
int  VzLogDebug(const char *file, int line,
                const char *fmt, ...);

}  // namespace vzlogging

#ifdef __cplusplus
}
#endif

// 错误打印
#define VZ_ERROR(VZ_FM, ...)   \
  vzlogging::VzLogDebug(__FILE__, __LINE__, VZ_FM, ##__VA_ARGS__)
#ifndef NDEBUG  // cmake Debug模式下定义
#define VZ_PRINT(VZ_FM, ...)   \
  vzlogging::VzLogDebug(__FILE__, __LINE__, VZ_FM, ##__VA_ARGS__)
#else
#define VZ_PRINT(VZ_FM, ...)
#endif

#endif  // LIBVZLOGGING_VZLOGDEF_H_
