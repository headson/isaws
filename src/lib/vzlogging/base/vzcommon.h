/************************************************************************
* Author      : SoberPeng 2017-09-08
* Description : ��־������
************************************************************************/
#ifndef LIBVZLOGGING_VZCOMMON_H
#define LIBVZLOGGING_VZCOMMON_H

#include "vzbases.h"
#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
#define HDL_NULL  NULL
#else   // LINUX
#include <fcntl.h>
#include <unistd.h>

#include <sys/time.h>

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define INVALID_SOCKET -1

typedef int       HANDLE;
#define HDL_NULL  -1
#endif  // LINUX

namespace vzlogging {

/**�����ڴ�-����*********************************************************/
typedef struct _TAG_WATCHDOG {
  unsigned int  mark;

  char          app_name[LEN_APP_NAME];    // ������
  char          descrebe[LEN_DESCREBE];    // �û�����

  unsigned int  status;                    // 

  unsigned int  timeout;                   // ��ʱ
  unsigned int  last_heartbeat;            // ���һ������ʱ��
} TAG_MODULE_STATE;

typedef struct {
  unsigned int        snd_level;    // ��־��ӡ����
  struct sockaddr_in  sock_addr;    //

  unsigned int        checksum;     // У��;У�鹲���ڴ�Ϊ������

  TAG_MODULE_STATE    mod_state[MAX_WATCHDOG_A_DEVICE];
} TAG_SHM_ARG;

class CVzLogShm {
 public:
  CVzLogShm();
  virtual ~CVzLogShm();

  // ��
  bool  Open();
  void  Close();

  int Share(const void* data, unsigned int size);

  // ��ȡ����
  unsigned int        GetLevel() const;
  struct sockaddr_in* GetSockAddr() const;

  int GetShmArg(TAG_SHM_ARG *arg);

 private:
  HANDLE      vshm_;
  bool        valid_;   // ������Ĵ���
};

/**��־�߳�˽������*******************************************************/
class CVzLogSock {
public:
  CVzLogSock();
  ~CVzLogSock();

  int InitSocket();

  void SetRemoteAddr(const char* ip, unsigned short port);
  int SetRemoteAddr(struct sockaddr_in* addr);              // ��ַ���Ϸ���������־

  int Write(struct sockaddr_in* addr, const char* msg, unsigned int size);

private:
  SOCKET              s_;
  struct sockaddr_in  addr_;

public:
  const int           max_nlog;
  int                 nlog;
  char                slog[A_LOG_SIZE + 4];  // ��־ʹ��buffer
};

}  // namespace vzlogging

extern bool k_log_print;                        // ���ʹ��
extern char k_app_name[LEN_APP_NAME];           // ������
extern vzlogging::CVzLogShm   k_shm_arg;        // �����ڴ�
extern vzlogging::CVzLogSock *GetVzLogSock();   // ��ȡ��־SOCKET���(tls)

#endif  // LIBVZLOGGING_VZCOMMON_H