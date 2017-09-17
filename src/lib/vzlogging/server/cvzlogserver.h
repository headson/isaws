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
#ifndef INVALID_SOCKET
#define INVALID_SOCKET  -1
#endif  // INVALID_SOCKET
#endif

#include "vzlogging/base/vzbases.h"

namespace vzlogging {

/* 接收回调 */
typedef int (*CALLBACK_RECEIVE)(SOCKET             sock,
                             const sockaddr_in *addr,
                             const char*        msg,
                             unsigned int       size, 
                             void              *usr_arg);
/* 超时回调 */
typedef int(*CALLBACK_TIMEOUT)(void *usr_arg);

typedef int(*CALLBACK_FEEDOG)(void *usr_arg);

class CVzLogSrv {
 public:
  CVzLogSrv();
  virtual ~CVzLogSrv();

  // 设置回调函数
  void SetCallback(CALLBACK_RECEIVE receive_cb,
                   void *recv_usr_arg,
                   CALLBACK_TIMEOUT timeout_cb,
                   void *timeout_usr_arg);

  // 打开透传地址
  int  OpenTransAddr(const char* s_srv_addr);
  int  TransMsgToServer(const char* s_msg, unsigned int n_msg);

  // 打开监听端口
  int  OpenListenAddr(const char* ip, unsigned short port);
  void Close();

  // 循环等待数据或超时(毫秒)
  void Loop(int ms = 5*1000);

 private:
  CALLBACK_RECEIVE cb_receive_;       // 接收回调
  void            *receive_usr_arg_;

  CALLBACK_TIMEOUT cb_timeout_;       // 超时回调
  void            *timeout_usr_arg_;

 private:  // 需要初始化
  fd_set           rfds_;
  SOCKET           rsock_;   // 接收SOCKET
  SOCKET           ssock_;   // 转发SOCKET

 private:  // 接收数据
  int              nlog_;                 //
  char             slog_[A_LOG_SIZE+4];   // 接收远端数据,MAX 1024
};

//////////////////////////////////////////////////////////////////////////
class CVzLogFile {
 public:
  CVzLogFile();
  virtual ~CVzLogFile();

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
  int     n_file_limit_size_;                // 单个文件大小
  char    s_err_fname_[LEN_FILEPATH + 1];    // 错误文件名
  char    s_filename_[2][LEN_FILEPATH + 1];  // 文件名
  int     n_filename_idx_;                   // 文件名索引

 protected:
  FILE*   p_file_;                           // 日志文件
  int     n_file_size_;                      // 日志大小
};

/* 只能使用第一个文件 */
class CVzWdgFile : public CVzLogFile {
 public:
  CVzWdgFile();
  virtual ~CVzWdgFile();

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
