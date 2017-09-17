/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : ��־���������
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

/* ���ջص� */
typedef int (*CALLBACK_RECEIVE)(SOCKET             sock,
                             const sockaddr_in *addr,
                             const char*        msg,
                             unsigned int       size, 
                             void              *usr_arg);
/* ��ʱ�ص� */
typedef int(*CALLBACK_TIMEOUT)(void *usr_arg);

typedef int(*CALLBACK_FEEDOG)(void *usr_arg);

class CVzLogSrv {
 public:
  CVzLogSrv();
  virtual ~CVzLogSrv();

  // ���ûص�����
  void SetCallback(CALLBACK_RECEIVE receive_cb,
                   void *recv_usr_arg,
                   CALLBACK_TIMEOUT timeout_cb,
                   void *timeout_usr_arg);

  // ��͸����ַ
  int  OpenTransAddr(const char* s_srv_addr);
  int  TransMsgToServer(const char* s_msg, unsigned int n_msg);

  // �򿪼����˿�
  int  OpenListenAddr(const char* ip, unsigned short port);
  void Close();

  // ѭ���ȴ����ݻ�ʱ(����)
  void Loop(int ms = 5*1000);

 private:
  CALLBACK_RECEIVE cb_receive_;       // ���ջص�
  void            *receive_usr_arg_;

  CALLBACK_TIMEOUT cb_timeout_;       // ��ʱ�ص�
  void            *timeout_usr_arg_;

 private:  // ��Ҫ��ʼ��
  fd_set           rfds_;
  SOCKET           rsock_;   // ����SOCKET
  SOCKET           ssock_;   // ת��SOCKET

 private:  // ��������
  int              nlog_;                 //
  char             slog_[A_LOG_SIZE+4];   // ����Զ������,MAX 1024
};

//////////////////////////////////////////////////////////////////////////
class CVzLogFile {
 public:
  CVzLogFile();
  virtual ~CVzLogFile();

  /************************************************************************
  *Description : ����־�ļ�
  *Parameters  : s_path[IN]       ��־�洢·��
  *              s_filename[IN]   ��־�ļ���
  *              n_limit_size[IN] �ܹ���ʹ�õ���־�ļ���С
  *Return      : 0 �ɹ�,����ʧ��
  ************************************************************************/
  virtual int Open(const char*  s_path,
                   const char*  s_filename,
                   unsigned int n_limit_size);
  int WriteSome(const char* s_usr_cmd);

  void Sync();

  // ģ�鶪ʧ,ת����־
  int OnModuleLostHeartbeat(const char *s_info, int n_info);

  // ��־��¼
  virtual int Write(const char* s_msg, unsigned int n_msg);

 protected:
  // �ж�ת��
  virtual int CheckFileReopen(unsigned int n_msg);
  int OpenFileFirst();
  int OpenFileNext(unsigned int n_msg);

  // ��ȡ�ļ����һ���޸�ʱ��
  time_t GetFileMTime(FILE* file);

 protected:
  int     n_file_limit_size_;                // �����ļ���С
  char    s_err_fname_[LEN_FILEPATH + 1];    // �����ļ���
  char    s_filename_[2][LEN_FILEPATH + 1];  // �ļ���
  int     n_filename_idx_;                   // �ļ�������

 protected:
  FILE*   p_file_;                           // ��־�ļ�
  int     n_file_size_;                      // ��־��С
};

/* ֻ��ʹ�õ�һ���ļ� */
class CVzWdgFile : public CVzLogFile {
 public:
  CVzWdgFile();
  virtual ~CVzWdgFile();

  /************************************************************************
  *Description : ����־�ļ�
  *Parameters  : s_path[IN]       ��־�洢·��
  *              s_filename[IN]   ��־�ļ���
  *              n_limit_size[IN] �ܹ���ʹ�õ���־�ļ���С
  *Return      : 0 �ɹ�,����ʧ��
  ************************************************************************/
  virtual int Open(const char*  s_path,
                   const char*  s_filename,
                   unsigned int n_limit_size);

 protected:
  // �ж�ת��
  virtual int CheckFileReopen(unsigned int n_msg);
};

}  // namespace vzlogging

#endif  // LIBVZLOGSERVER_CVZLOGSERVER_H_
