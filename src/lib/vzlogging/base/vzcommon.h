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
#ifndef INVALID_SOCKET
#define INVALID_SOCKET  -1
#endif  // INVALID_SOCKET

typedef int       HANDLE;
#define HDL_NULL  -1
#endif  // LINUX

namespace vzlogging {

/**�����ڴ�-����*********************************************************/
typedef struct _TAG_WATCHDOG {
  unsigned int  mark;

  char          app_name[LEN_APP_NAME];    // ������
  char          descrebe[LEN_DESCREBE];    // �û�����
  unsigned int  join_time;                 // ������ʱ��

  unsigned int  status;                    // 

  unsigned int  timeout;                   // ��ʱ
  
  unsigned int  last_heartbeat;            // ���һ������ʱ��

  //////////////////////////////////////////////////////////////////////////
  bool Init(const char *name, const char* desc,
            unsigned int ms_timeout);
  bool UpdateTimeout(unsigned int ms_timeout);
  bool Heartbeat();
  bool isSame(const char *name, const char* desc);
  bool isEmpty();
  bool isTimeout(unsigned int cur_time);
} TAG_MODULE_STATE;

typedef struct {
  unsigned int        snd_level;    // ��־��ӡ����
  struct sockaddr_in  sock_addr;    //

  unsigned int        checksum;     // У��;У�鹲���ڴ�Ϊ������

  TAG_MODULE_STATE    mod_state[MAX_WATCHDOG_A_DEVICE];
} TAG_SHM_ARG;

class VShm {
public:
  VShm() {
    shm_id_ = HDL_NULL;
    shm_ptr_ = NULL;
    shm_size_ = 0;
  }
  virtual ~VShm() {
    if (shm_ptr_) {
      vzShmDt(shm_ptr_);
      shm_ptr_ = NULL;
    }
  }

#ifdef _WIN32
  bool Open(const char* name, int size);

protected:
  void *vzShmAt();
  void vzShmDt(void *p_ptr);

#else
  int create_file(const char *filename);
  bool Open(const char* name, int size);

protected:
  void *vzShmAt();
  void vzShmDt(void *p_ptr);
#endif
public:
  void* GetData() const {
    return shm_ptr_;
  }
  int GetSize() const {
    return shm_size_;
  }

  int GetData(void *pdata, int ndata);
  int SetData(const void *pdata, int ndata);

private:
  HANDLE  shm_id_;
  void *  shm_ptr_;     //
  int     shm_size_;    //
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
extern vzlogging::VShm        k_shm_arg;
extern vzlogging::CVzLogSock *GetVzLogSock();   // ��ȡ��־SOCKET���(tls)

#endif  // LIBVZLOGGING_VZCOMMON_H
