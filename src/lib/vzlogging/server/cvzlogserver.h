/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : 日志网络服务器
************************************************************************/
#ifndef LIBVZLOGSERVER_CVZLOGSERVER_H_
#define LIBVZLOGSERVER_CVZLOGSERVER_H_

#include <stdio.h>

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>

#include <io.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#include <netdb.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef int             SOCKET;
#define INVALID_SOCKET  -1
#endif

#include "vzlogging/base/vzlogdef.h"

namespace vzlogging {

/* 接收回调 */
typedef int (*RECV_CALLBACK)(SOCKET             sock,
                             const sockaddr_in* addr,
                             const char*        msg,
                             unsigned int       size);
/* 超时回调 */
typedef int(*TIMEOUT_CALLBACK)();

typedef int(*FEEDOG_CALLBACK)();

class CVzSockDgram {
 public:
  CVzSockDgram();
  virtual ~CVzSockDgram();

  // 设置回调函数
  void SetCallback(RECV_CALLBACK recv_cb,
                   TIMEOUT_CALLBACK timeout_cb,
                   FEEDOG_CALLBACK feeddog_cb);

  // 打开透传地址
  int  OpenTransAddr(const char* s_srv_addr);
  int  TransMsgToServer(const char* s_msg, unsigned int n_msg);

  // 打开监听端口
  int  OpenListenAddr(const char* ip, unsigned short port);
  void Close();

  // 循环等待数据或超时
  void Loop();

 private:
  RECV_CALLBACK    recv_cb_;                // 接收回调
  TIMEOUT_CALLBACK timeout_cb_;             // 超时回调
  FEEDOG_CALLBACK  feeddog_cb_;             // 喂狗回调

 private:  // 需要初始化
  fd_set           rfds_;
  SOCKET           sock_recv_;              // 接收SOCKET
  SOCKET           sock_send_;              // 转发SOCKET

  // 运行时初始化;不在循环中创建,减少可能的内存碎片
 private:
  struct timeval   tv_;                           // select超时参数

 private:  // 接收数据
  sockaddr_in      recv_addr_;                    // 接收远端地址
#ifdef WIN32
  int              recv_addr_len_;                //
#else
  socklen_t        recv_addr_len_;                //
#endif

  int              recv_size_;                    //
  char             recv_data_[DEF_LOG_MAX_SIZE+1];  // 接收远端数据,MAX 1024
};

//////////////////////////////////////////////////////////////////////////
class CVzLoggingFile {
 public:
  CVzLoggingFile();
  virtual ~CVzLoggingFile();

  /************************************************************************
  *Description : 打开日志文件
  *Parameters  : s_path[IN]       日志存储路径
  *              s_filename[IN]   日志文件名
  *              n_limit_size[IN] 总共能使用的日志文件大小
  *Return      : 0 成功,其他失败
  ************************************************************************/
  virtual int Open(const char*  s_path,
                   const char*  s_filename,
                   unsigned int n_limit_size);
  int WriteSome(const char* s_usr_cmd);

  void Sync();

  // 模块丢失,转存日志
  int OnModuleLostHeartbeat(const char *s_info, int n_info);

  // 日志记录
  virtual int Write(const char* s_msg, unsigned int n_msg);

 protected:
  // 判断转档
  virtual int CheckFileReopen(unsigned int n_msg);
  int OpenFileFirst();
  int OpenFileNext(unsigned int n_msg);

  // 获取文件最后一次修改时间
  time_t GetFileMTime(FILE* file);

 protected:
  int     n_file_limit_size_;                   // 单个文件大小
  char    s_err_fname_[DEF_LOG_FILE_NAME + 1];    // 错误文件名
  char    s_filename_[2][DEF_LOG_FILE_NAME + 1];  // 文件名
  int     n_filename_idx_;                      // 文件名索引

 protected:
  FILE*   p_file_;                              // 日志文件
  int     n_file_size_;                         // 日志大小
};

/* 只能使用第一个文件 */
class CVzWatchdogFile : public CVzLoggingFile {
 public:
  CVzWatchdogFile();
  virtual ~CVzWatchdogFile();

  /************************************************************************
  *Description : 打开日志文件
  *Parameters  : s_path[IN]       日志存储路径
  *              s_filename[IN]   日志文件名
  *              n_limit_size[IN] 总共能使用的日志文件大小
  *Return      : 0 成功,其他失败
  ************************************************************************/
  virtual int Open(const char*  s_path,
                   const char*  s_filename,
                   unsigned int n_limit_size);

 protected:
  // 判断转档
  virtual int CheckFileReopen(unsigned int n_msg);
};

}  // namespace vzlogging

#endif  // LIBVZLOGSERVER_CVZLOGSERVER_H_
