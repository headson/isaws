/************************************************************************/
/* Copyright@ 2008 vzenith.com
/* All rights reserved.
/* ----------------------------------------------------------------------
/* Author      : Sober.Peng
/* Date        : 19:5:2017
/* Description : ��־���������
/************************************************************************/
#ifndef SRC_LIB_VZLOGGING_SERVER_CVZLOGSERVER_H_
#define SRC_LIB_VZLOGGING_SERVER_CVZLOGSERVER_H_

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

/* ���ջص� */
typedef int (*RECV_CALLBACK)(SOCKET             sock,
                             const sockaddr_in* addr,
                             const char*        msg,
                             unsigned int       size);
/* ��ʱ�ص� */
typedef int(*TIMEOUT_CALLBACK)();

class CVzSockDgram {
 public:
  CVzSockDgram();
  virtual ~CVzSockDgram();

  // ���ûص�����
  void SetCallback(RECV_CALLBACK recv_cb, TIMEOUT_CALLBACK timeout_cb);

  // ��͸����ַ
  int  OpenTransAddr(const char* s_srv_addr);
  int  TransMsgToServer(const char* s_msg, unsigned int n_msg);

  // �򿪼����˿�
  int  OpenListenAddr(const char* ip, unsigned short port);
  void Close();

  // ѭ���ȴ����ݻ�ʱ
  void Loop();

 private:
  RECV_CALLBACK    recv_cb_;                // ���ջص�
  TIMEOUT_CALLBACK timeout_cb_;             // ��ʱ�ص�

 private:  // ��Ҫ��ʼ��
  fd_set           rfds_;
  SOCKET           sock_recv_;              // ����SOCKET
  SOCKET           sock_send_;              // ת��SOCKET

  // ����ʱ��ʼ��;����ѭ���д���,���ٿ��ܵ��ڴ���Ƭ
 private:
  struct timeval   tv_;                           // select��ʱ����

 private:  // ��������
  sockaddr_in      recv_addr_;                    // ����Զ�˵�ַ
#ifdef WIN32
  int              recv_addr_len_;                //
#else
  socklen_t        recv_addr_len_;                //
#endif

  int              recv_size_;                    //
  char             recv_data_[DEF_LOG_MAX_SIZE+1];  // ����Զ������,MAX 1024
};

//////////////////////////////////////////////////////////////////////////
class CVzLoggingFile {
 public:
  CVzLoggingFile();
  virtual ~CVzLoggingFile();

  /************************************************************************/
  /* Description : ����־�ļ�
  /* Parameters  : s_path[IN]       ��־�洢·��
                   s_filename[IN]   ��־�ļ���
                   n_limit_size[IN] �ܹ���ʹ�õ���־�ļ���С
  /* Return      : 0 �ɹ�,����ʧ��
  /************************************************************************/
  virtual int Open(const char*  s_path,
                   const char*  s_filename,
                   unsigned int n_limit_size);
  int StartRecord(const char* s_usr_cmd);

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
  int     n_file_limit_size_;                   // �����ļ���С
  char    s_err_fname_[DEF_LOG_FILE_NAME + 1];    // �����ļ���
  char    s_filename_[2][DEF_LOG_FILE_NAME + 1];  // �ļ���
  int     n_filename_idx_;                      // �ļ�������

 protected:
  FILE*   p_file_;                              // ��־�ļ�
  int     n_file_size_;                         // ��־��С
};

/* ֻ��ʹ�õ�һ���ļ� */
class CVzWatchdogFile : public CVzLoggingFile {
 public:
  CVzWatchdogFile();
  virtual ~CVzWatchdogFile();

  /************************************************************************/
  /* Description : ����־�ļ�
  /* Parameters  : s_path[IN]       ��־�洢·��
                   s_filename[IN]   ��־�ļ���
                   n_limit_size[IN] �ܹ���ʹ�õ���־�ļ���С
  /* Return      : 0 �ɹ�,����ʧ��
  /************************************************************************/
  virtual int Open(const char*  s_path,
                   const char*  s_filename,
                   unsigned int n_limit_size);

 protected:
  // �ж�ת��
  virtual int CheckFileReopen(unsigned int n_msg);
};

}  // namespace vzlogging

#endif  // SRC_LIB_VZLOGGING_SERVER_CVZLOGSERVER_H_