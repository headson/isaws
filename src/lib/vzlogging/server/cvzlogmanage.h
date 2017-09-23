/************************************************************************
*Author      : Sober.Peng 17-07-20
*Description :
************************************************************************/
#ifndef LIBVZLOGSERVER_CVZLOGMANAGE_H_
#define LIBVZLOGSERVER_CVZLOGMANAGE_H_

#include "vzlogging/base/vzbases.h"
#include "vzlogging/base/vzcommon.h"
#include "vzlogging/logging/vzlogging.h"
#include "vzlogging/server/cvzlogserver.h"

namespace vzlogging {
class CVzLogManage {
 protected:
  CVzLogManage();
  virtual ~CVzLogManage();

 public:
  static CVzLogManage *Instance();

  int  Start(const char* log_path,
             unsigned short log_port,
             const char* tran_addr);

  void RunLoop(int ms);

  // 设置端口和等级
  void SetLogAddrAndLevel(const char *lhost, unsigned short port, unsigned int level);

  // 开启STDOUT输出
  void SetEnableStdout();
  // 关闭看门狗
  void SetDisableWatchdog();

 protected:
  /*初始化日志记录*/
  int InitLogRecord(const char* path,
                    const char* log_fname,
                    const char* wdg_fname);

  /*初始化看门狗监控模块*/
  int InitMonitorModule(const char* s_path);

  /*初始化网络服务*/
  int InitSrvSocket(const char* ip, unsigned short port,
                    const char* trans_addr);

  /*看门狗处理*/
  int WatchdogProcess(const char* smsg, unsigned int nmsg);

  int OnModuleLostHeartbeat(time_t nnow);

 protected:
  static int callback_receive(SOCKET sock, const sockaddr_in *sa_remote,
                              const char *smsg, unsigned int nmsg,
                              void *usr_arg) {
    if (usr_arg) {
      return ((CVzLogManage*)usr_arg)->OnReceive(sock, sa_remote, smsg, nmsg);
    }
    return -1;
  }
  int OnReceive(SOCKET sock, const sockaddr_in *raddr,
                const char *smsg, unsigned int nmsg);

  static int callback_timeout(void *usr_arg) {
    if (usr_arg) {
      return ((CVzLogManage*)usr_arg)->OnTimeout();
    }
    return -1;
  }
  int OnTimeout();

 protected:
  TAG_SHM_ARG    *shm_mod_;

  unsigned int    is_stdout_;     // 使能输出
  unsigned int    is_reboot_;     // 使能看门狗

 private:
  CVzLogSrv       log_srv_;       // 网络服务
  CVzLogFile      log_file_;      // 普通日志
  CVzWdgFile      wdg_file_;      // 看门狗日志

 private:
  unsigned int    stride_watch_time_;  // 比GetSysSec大,跳过记录日志和心跳检查;
};
}  // namespace vzlogging
#endif  // LIBVZLOGSERVER_CVZLOGMANAGE_H_
