/************************************************************************/
/* Copyright@ 2008 vzenith.com                                          */
/* All rights reserved.                                                 */
/* -------------------------------------------------------------------- */
/* Author      : Sober.Peng                                             */
/* Date        : 19:5:2017                                              */
/* Description : ͨ�������ڴ�ʵ�ֽ��̼���־��������                       */
/************************************************************************/
#ifndef SRC_LIB_VZLOGGING_BASE_VZSHMARG_H_
#define SRC_LIB_VZLOGGING_BASE_VZSHMARG_H_

#include "vzlogging/base/vzlogdef.h"

#ifdef WIN32
#include <windows.h>
#include <winsock2.h>
typedef HANDLE              ShmHdl;
typedef DWORD               ShmSize;
typedef LPVOID              ShmPtr;
#define SHM_NULL            NULL
#else
#include <netdb.h>
typedef int                 ShmHdl;
typedef unsigned int        ShmSize;
typedef void*               ShmPtr;
#define SHM_NULL            -1
#endif

typedef const char*         ShmKey;

namespace vzlogging {

/**VShm******************************************************************/
class VShm {
 public:
  VShm();
  virtual ~VShm();

  int           Open(ShmKey key, ShmSize size);
  void          Close();

  void*         GetData() const {
    return shm_ptr_;
  }
  unsigned int  GetSize() const {
    return shm_size_;
  }

 private:
  char      shm_key_[32];

 private:
  ShmHdl    shm_hdl_;     //
  ShmPtr    shm_ptr_;     //
  ShmSize   shm_size_;    //
};

/**�����ڴ�-����*********************************************************/
typedef struct _TAG_WATCHDOG {
  unsigned int  mark;

  char          app_name[DEF_PROCESS_NAME_MAX];     // ������
  char          descrebe[DEF_USER_DESCREBE_MAX];    // �û�����

  unsigned int  status;                             // 

  unsigned int  timeout;                            // ��ʱ
  unsigned int  last_heartbeat;                     // ���һ������ʱ��
} TAG_MODULE_STATE;

typedef struct {
  unsigned int        snd_level;    // ��־��ӡ����
  struct sockaddr_in  sock_addr;    //

  unsigned int        checksum;     // У��;У�鹲���ڴ�Ϊ������

  TAG_MODULE_STATE    mod_state[DEF_PER_DEV_PROCESS_MAX];
} TAG_SHM_ARG;

class CVzShmArg {
 public:
  CVzShmArg();
  virtual ~CVzShmArg();

  // ��
  int   Open();
  void  Close();

  // ����
  int   Share(const void* data, unsigned int size);

  // ��ȡ����
  unsigned int        GetSendLevel() const;
  struct sockaddr_in* GetSockAddr() const;

  int GetShmArg(TAG_SHM_ARG *arg);

 private:
  VShm        vshm_;
  bool        valid_;   // ������Ĵ���
};

}  // namespace vzlogging

#endif  // SRC_LIB_VZLOGGING_BASE_VZSHMARG_H_
