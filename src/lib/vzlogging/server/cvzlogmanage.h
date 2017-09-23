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

  // ���ö˿ں͵ȼ�
  void SetLogAddrAndLevel(const char *lhost, unsigned short port, unsigned int level);

  // ����STDOUT���
  void SetEnableStdout();
  // �رտ��Ź�
  void SetDisableWatchdog();

 protected:
  /*��ʼ����־��¼*/
  int InitLogRecord(const char* path,
                    const char* log_fname,
                    const char* wdg_fname);

  /*��ʼ�����Ź����ģ��*/
  int InitMonitorModule(const char* s_path);

  /*��ʼ���������*/
  int InitSrvSocket(const char* ip, unsigned short port,
                    const char* trans_addr);

  /*���Ź�����*/
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

  unsigned int    is_stdout_;     // ʹ�����
  unsigned int    is_reboot_;     // ʹ�ܿ��Ź�

 private:
  CVzLogSrv       log_srv_;       // �������
  CVzLogFile      log_file_;      // ��ͨ��־
  CVzWdgFile      wdg_file_;      // ���Ź���־

 private:
  unsigned int    stride_watch_time_;  // ��GetSysSec��,������¼��־���������;
};
}  // namespace vzlogging
#endif  // LIBVZLOGSERVER_CVZLOGMANAGE_H_
