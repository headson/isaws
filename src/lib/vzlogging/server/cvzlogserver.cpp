/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description : ��־���������
************************************************************************/
#include "vzlogging/server/cvzlogserver.h"

#include <time.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys/stat.h>

#include "vzlogging/base/vzbases.h"

namespace vzlogging {
const char MONTH_TABLE[12][4] = {
  "Jan", "Feb", "Mar", "Apr", "May", "Jun",
  "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

CVzLogSrv::CVzLogSrv()
  : cb_receive_(NULL)
  , receive_usr_arg_(NULL)
  , cb_timeout_(NULL)
  , timeout_usr_arg_(NULL) {
  //VZ_PRINT("%s[%d].\n", __FUNCTION__, __LINE__);

  FD_ZERO(&rfds_);
  rsock_ = INVALID_SOCKET;
  ssock_ = INVALID_SOCKET;
}

CVzLogSrv::~CVzLogSrv() {
  Close();
  //VZ_PRINT("%s[%d].\n", __FUNCTION__, __LINE__);
}

void CVzLogSrv::SetCallback(CALLBACK_RECEIVE receive_cb,
                            void *recv_usr_arg,
                            CALLBACK_TIMEOUT timeout_cb,
                            void *timeout_usr_arg) {
  cb_receive_ = receive_cb;
  receive_usr_arg_ = recv_usr_arg;

  cb_timeout_ = timeout_cb;
  timeout_usr_arg_ = timeout_usr_arg;
}

//
int CVzLogSrv::OpenTransAddr(const char* s_srv_addr) {
  char s_ip[20] = {0};
  unsigned int n_port = 0;
  char* token = strrchr((char*)s_srv_addr, ':');
  if (token != NULL) {
    memcpy(s_ip, s_srv_addr, token-s_srv_addr);
    n_port = atoi(token+1);
  }
  if (strlen(s_ip) > 0 && n_port > 0) {
    // SOCKETֻ��һ��
    if (ssock_ == INVALID_SOCKET) {
      ssock_ = ::socket(AF_INET, SOCK_DGRAM, 0);
      if (ssock_ == INVALID_SOCKET) {
        VZ_ERROR("socket create failed.\n");
        return -1;
      }

      // REUSE address
      int val = 1;
      setsockopt(ssock_,
                 SOL_SOCKET,
                 SO_REUSEADDR,
                 (char*)&val,
                 sizeof(int));
    }

    // ����ת����ַ���ܶ��
    struct sockaddr_in  sa_snd_srv;          // ת����ַ
    sa_snd_srv.sin_family       = AF_INET;
    sa_snd_srv.sin_port         = htons(n_port);
    sa_snd_srv.sin_addr.s_addr  = inet_addr(s_ip);

    // �󶨷��͵�ַ,�ڷ���ʱ�ɲ���ָ��
    connect(ssock_, (sockaddr*)&sa_snd_srv, sizeof(sa_snd_srv));
    return 0;
  }
  return -1;
}

int CVzLogSrv::TransMsgToServer(const char* s_msg, unsigned int n_msg) {
  if (ssock_ == INVALID_SOCKET) {
    return -1;
  }
  return send(ssock_, s_msg, n_msg, 0);
}

int CVzLogSrv::OpenListenAddr(const char* ip, unsigned short port) {
  Close();

  rsock_ = ::socket(AF_INET, SOCK_DGRAM, 0);
  if (rsock_ == INVALID_SOCKET) {
    VZ_ERROR("socket create failed.\n");
    return -1;
  }

  // REUSE address
  int val = 1;
  setsockopt(rsock_, SOL_SOCKET, SO_REUSEADDR, (char*)&val, sizeof(int));

  // bind local address
  struct sockaddr_in local_addr;
  local_addr.sin_family       = AF_INET;
  local_addr.sin_port         = htons(port);
  local_addr.sin_addr.s_addr  = inet_addr(ip);

  int ret = ::bind(rsock_,
                   (struct sockaddr*)&local_addr, sizeof(struct sockaddr));
  if (ret < 0) {
    VZ_ERROR("socket bind address failed.\n");

    Close();
    return -2;
  }

  // ���ý��ջ�����
  int opt = 64 * 1024;
  ret = setsockopt(rsock_, SOL_SOCKET, SO_RCVBUF, (char*)&opt, sizeof(int));
  if (ret < 0) {
    VZ_ERROR("setsockopt SO_RCVBUF failed.\n");

    Close();
    return -3;
  }

  // nonblock
#ifdef WIN32
  int mode = 1;
  ::ioctlsocket(rsock_, FIONBIO, (u_long FAR*)&mode);
#else
  int mode = fcntl(rsock_, F_GETFL, 0);
  fcntl(rsock_, F_SETFL, mode | O_NONBLOCK);
#endif

  return 0;
}

void CVzLogSrv::Close() {
  if (INVALID_SOCKET != rsock_) {
#ifdef WIN32
    closesocket(rsock_);
#else
    close(rsock_);
#endif
    rsock_ = INVALID_SOCKET;
  }
}

void CVzLogSrv::Loop(int ms) {
  FD_ZERO(&rfds_);
  FD_SET(rsock_, &rfds_);

  // ������
  struct timeval tv;
  tv.tv_sec = ms / 1000;
  tv.tv_usec = (ms % 1000) * 1000;
  if (select(rsock_+1, &rfds_, NULL, NULL, &tv) > 0) {
    if (INVALID_SOCKET != rsock_) {
      if (FD_ISSET(rsock_, &rfds_)) {
        do {
          sockaddr_in      raddr;                    // ����Զ�˵�ַ
#ifdef WIN32
          int raddr_len = 0;
#else
          socklen_t raddr_len = 0;
#endif

          raddr_len = sizeof(sockaddr);
          nlog_ = recvfrom(rsock_, slog_, A_LOG_SIZE,
                           0,
                           (sockaddr*)&raddr,
                           &raddr_len);
          if (nlog_ > 0) {
            if (cb_receive_) {
              if (nlog_ < A_LOG_SIZE) {
                slog_[nlog_] = '\0';  // ��ֹ���
              }
              cb_receive_(rsock_, &raddr, slog_, nlog_, receive_usr_arg_);
            }
          } else {
            break;
          }
        } while (true);
      }
    }
  }

  if (cb_timeout_) {
    cb_timeout_(timeout_usr_arg_);
  }
}

//////////////////////////////////////////////////////////////////////////
CVzLogFile::CVzLogFile() {
  //VZ_PRINT("%s[%d].\n", __FUNCTION__, __LINE__);

  n_file_limit_size_  = 1024*1024;  // 1MB

  s_filename_[0][0]   = '\0';
  s_filename_[1][0]   = '\0';
  n_filename_idx_     = 0;

  p_file_             = NULL;
  n_file_size_        = 0;
}

CVzLogFile::~CVzLogFile() {
  if (p_file_) {
    fclose(p_file_);
    p_file_ = NULL;
  }
  //VZ_PRINT("%s[%d].\n", __FUNCTION__, __LINE__);
}

int CVzLogFile::Open(const char*  spath,
                     const char*  sfilename,
                     unsigned int limit_size) {
  // ���ļ���
  for (int i = 0; i < 2; i++) {
    memset(s_filename_[i], 0, LEN_FILEPATH+1);
    snprintf(s_filename_[i], LEN_FILEPATH,
             "%s%s_%d.log", spath, sfilename, i+1);
  }
  // �����ļ�������
  memset(s_err_fname_, 0, LEN_FILEPATH+1);
  snprintf(s_err_fname_, LEN_FILEPATH,
           "%s%s_err_%d.log", spath, sfilename, MAX_FILE_CNT);
  // �˴����ļ��Ѵ���,�ɵ�֮ǰ�Ĵ����ļ�
#ifdef WIN32
  if (_access(s_err_fname_, 0) == 0) {
#else
  if (access(s_err_fname_, 0) == 0) {
#endif
    char s_temp_fname[LEN_FILEPATH+1];
    for (int i = 0; i < MAX_FILE_CNT; i++) {
      // i�ļ�
      memset(s_temp_fname, 0, LEN_FILEPATH + 1);
      snprintf(s_temp_fname, LEN_FILEPATH,
               "%s%s_err_%d.log", spath, sfilename, i);
      // i+1�ļ�
      memset(s_err_fname_, 0, LEN_FILEPATH + 1);
      snprintf(s_err_fname_, LEN_FILEPATH,
               "%s%s_err_%d.log", spath, sfilename, i + 1);
      // ɾ����һ���ļ�,����ڶ�������rename�ɹ�
      if (i == 0 && (remove(s_temp_fname) == 0)) {
        VZ_PRINT("remove file %s success.\n", s_temp_fname);
      }
      if (rename(s_err_fname_, s_temp_fname) == 0) {
        VZ_PRINT("rename file %s success.\n", s_err_fname_);
      } else {
        VZ_PRINT("rename file %s failed.\n", s_err_fname_);
      }
    }
  }

  n_filename_idx_     = 0;                  // �ӵ�һ���ļ�д
  n_file_limit_size_  = limit_size / 2;   // �ļ����ƴ�С
  if (n_file_limit_size_ == 0) {
    n_file_limit_size_ = MAX_LOG_FILE_SIZE / 2;
  }

  return CheckFileReopen(0);
}

void CVzLogFile::Sync() {
  if (p_file_) {
    fflush(p_file_);
  }
}

int CVzLogFile::WriteSome(const char* s_usr_cmd) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  time_t tt = tv.tv_sec;
  struct tm *wtm = localtime(&tt);
  if (wtm == NULL) {
    return -1;
  }

  char s_log[64] = { 0 };
  int n_log = snprintf(s_log, 63,
                       "%s at [%02d/%s/%04d %02d:%02d:%02d %04d]\n",
                       s_usr_cmd,
                       wtm->tm_mday, MONTH_TABLE[wtm->tm_mon], wtm->tm_year + 1900,
                       wtm->tm_hour, wtm->tm_min, wtm->tm_sec,
                       (int)(tv.tv_usec / 1000));
  return Write(s_log, n_log);
}

int CVzLogFile::Write(const char* msg, unsigned int size) {
  if (p_file_ == NULL ||
      msg == NULL || size > A_LOG_SIZE) {
    return -1;
  }

  // ����ļ�,�����´�
  CheckFileReopen(size);

  // д���ļ�
  if (p_file_ != NULL) {
    unsigned int pos = 0;
    do {
      pos += fwrite(msg + pos, 1, size - pos, p_file_);
    } while (pos > 0 && pos < size && p_file_ != NULL);
    // VZ_ERROR("write file pos %d, %p.\n", pos, p_file_);
    n_file_size_ += size;
    return size;
  }
  return -2;
}

int CVzLogFile::CheckFileReopen(unsigned int size) {
  if (p_file_ != NULL) {  // �����д�,��wt+��
    return OpenFileNext(size);
  } else {              // ��һ����at+��
    return OpenFileFirst();
  }
  return -1;
}

// ��һ����at+��
int CVzLogFile::OpenFileFirst() {
  n_filename_idx_ = 0;
  p_file_ = fopen(s_filename_[n_filename_idx_], "at+");
  if (p_file_) {
    VZ_ERROR("open file %s success.\n", s_filename_[n_filename_idx_]);

    fseek(p_file_, 0, SEEK_END);
    n_file_size_ = ftell(p_file_);

    // ���ļ��ѱ�д��,����һ���ļ�
    if ((n_file_size_ + 2 * A_LOG_SIZE) >= n_file_limit_size_) {
      time_t f1_wtime = 0, f2_wtime = 0;

      // ��ȡ��һ���ļ�ʱ��
      f1_wtime = GetFileMTime(p_file_);
      fclose(p_file_);

      // �򿪵ڶ����ļ�
      n_filename_idx_ = 1;
      p_file_ = fopen(s_filename_[n_filename_idx_], "at+");
      if (p_file_) {
        fseek(p_file_, 0, SEEK_END);
        n_file_size_ = ftell(p_file_);

        // �ڶ����ļ�Ҳд����,�ж�����ļ�����ʱ��
        if ((n_file_size_ + 2 * A_LOG_SIZE) >= n_file_limit_size_) {
          f2_wtime = GetFileMTime(p_file_);

          // ��һ���ļ�����޸�ʱ����ڵڶ����ļ�,�رյڶ����ļ�,����һ���ļ�
          // �������Ⱥ�д�����ļ�,���Բ����������ļ��޸�ʱ��һ��
          if (f1_wtime > f2_wtime) {
            fclose(p_file_);

            // �ԽضϷ�ʽ�򿪵�һ���ļ�
            n_file_size_ = 0;
            n_filename_idx_ = 0;
            p_file_ = fopen(s_filename_[n_filename_idx_], "wt+");
            if (p_file_) {
              VZ_PRINT("use %s.\n", s_filename_[n_filename_idx_]);
              return 0;
            }
          }
        }

        VZ_PRINT("use %s.\n", s_filename_[n_filename_idx_]);
        return 0;
      } else {
        VZ_ERROR("open file %s failed.\n", s_filename_[n_filename_idx_]);
        return -1;
      }
    }
    VZ_PRINT("use %s.\n", s_filename_[n_filename_idx_]);
    return 0;
  }

  VZ_ERROR("open file %s failed.\n", s_filename_[n_filename_idx_]);
  return -1;
}

// �����д�,��wt+��
int CVzLogFile::OpenFileNext(unsigned int size) {
  // �ر�ԭ�ļ�
  if ((int)(n_file_size_ + size) >= n_file_limit_size_) {
    fclose(p_file_);
    p_file_ = NULL;
  } else {
    return 0;
  }

  // �����ļ���
  if (n_filename_idx_ == 0) {
    n_filename_idx_ = 1;
  } else {
    n_filename_idx_ = 0;
  }

  // �����ļ�
  n_file_size_ = 0;
  p_file_ = fopen(s_filename_[n_filename_idx_], "wt+");
  if (p_file_) {
    VZ_PRINT("use %s.\n", s_filename_[n_filename_idx_]);
    return 0;
  }
  return -1;
}

time_t CVzLogFile::GetFileMTime(FILE* file) {
  struct stat buf;
#ifdef WIN32
  int fd = _fileno(file);
#else
  int fd = fileno(file);
#endif
  fstat(fd, &buf);
  return buf.st_mtime;
}

int CVzLogFile::OnModuleLostHeartbeat(const char *s_info, int n_info) {
  int  n_log = 0;
  char s_log[A_LOG_SIZE + 1] = {0};
  FILE* fd_err = fopen(s_err_fname_, "wt+");
  if (fd_err) {
    int n_other_filename_idx = n_filename_idx_ ? 0 : 1;
    FILE* file = fopen(s_filename_[n_other_filename_idx], "rt");
    if (file) {
      fseek(file, n_file_size_, SEEK_SET);

      // ��һ��
      n_log = fread(s_log, 1, A_LOG_SIZE, file);
      char* p_log = strrchr(s_log, '\n');
      if (n_log > 0 && strlen(p_log) < A_LOG_SIZE) {
        fwrite(p_log + 1, 1, strlen(p_log) - 1, fd_err);
      }
      do {
        n_log = fread(s_log, 1, A_LOG_SIZE, file);
        if (n_log > 0 && n_log < A_LOG_SIZE) {
          fwrite(s_log, 1, n_log, fd_err);
        }
      } while (!feof(file) && n_log > 0);
      fclose(file);
      file = NULL;
    }

    if (p_file_) {
      fclose(p_file_);
      p_file_ = NULL;

      p_file_ = fopen(s_filename_[n_filename_idx_], "rt+");
      if (p_file_) {
        do {
          n_log = fread(s_log, 1, A_LOG_SIZE, p_file_);
          if (n_log > 0 && n_log < A_LOG_SIZE) {
            fwrite(s_log, 1, n_log, fd_err);
          }
        } while (!feof(p_file_) && n_log > 0);
      }
    }
    // д
    if (s_info != NULL && n_info > 0) {
      fwrite(s_info, 1, n_info, fd_err);
    }
    fclose(fd_err);
    fd_err = NULL;
    return 0;
  }
  return -1;
}

/*���Ź���־�ļ�**********************************************************/
CVzWdgFile::CVzWdgFile()
  : CVzLogFile() {
  //VZ_PRINT("%s[%d].\n", __FUNCTION__, __LINE__);
}

CVzWdgFile::~CVzWdgFile() {
  //VZ_PRINT("%s[%d].\n", __FUNCTION__, __LINE__);
}

int CVzWdgFile::Open(const char* s_path,
                     const char* s_filename,
                     unsigned int n_limit_size) {
  // ���ļ���
  snprintf(s_filename_[0], LEN_FILEPATH,
           "%s%s.log", s_path, s_filename);
  snprintf(s_filename_[1], LEN_FILEPATH,
           "%s%s_temp.log", s_path, s_filename);

  n_filename_idx_ = 0;
  n_file_limit_size_ = n_limit_size;

  return CheckFileReopen(0);
}

int CVzWdgFile::CheckFileReopen(unsigned int n_msg) {
  if (p_file_ == NULL) {  // �����д�,��wt+��
    p_file_ = fopen(s_filename_[0], "at+");
    if (p_file_) {
      fseek(p_file_, 0, SEEK_END);
      n_file_size_ = ftell(p_file_);

      // �������ļ����ƴ�С,��ȡǰ���
      if (n_file_size_ > n_file_limit_size_) {
        FILE* file_temp = fopen(s_filename_[1], "wt+");
        if (file_temp) {
          int n_less_size = n_file_limit_size_ / 2;
          fseek(p_file_,
                n_file_size_-n_less_size, SEEK_SET);  // ��ȡһ����־

          int  n_log = 0;
          char s_log[A_LOG_SIZE+1] = {0};
          // ��һ��
          n_log = fread(s_log, 1, A_LOG_SIZE, p_file_);
          char* p_log = strrchr(s_log, '\n');
          if (p_log && strlen(p_log) < A_LOG_SIZE) {
            fwrite(p_log + 1, 1, strlen(p_log) - 1, file_temp);
          }

          do {
            n_log = fread(s_log, 1, A_LOG_SIZE, p_file_);
            if (n_log > 0 && n_log < A_LOG_SIZE) {
              fwrite(s_log, 1, n_log, file_temp);
            }
          } while (!feof(p_file_) && n_log > 0);
          fclose(file_temp);
          file_temp = NULL;

          fclose(p_file_);

          if (remove(s_filename_[0]) == 0) {
            VZ_PRINT("watchdog file remove %s success.\n", s_filename_[0]);
          }
          if (rename(s_filename_[1], s_filename_[0]) == 0) {
            VZ_PRINT("watchdog file rename success.\n");
          } else {
            VZ_PRINT("watchdog file rename failed.\n");
          }

          // reopen watchdog file
          p_file_ = fopen(s_filename_[0], "at+");
          if (p_file_) {
            fseek(p_file_, 0, SEEK_END);
            n_file_size_ = ftell(p_file_);
          }
        }
      }
      return 0;
    }
  }
  return -1;
}

}  // namespace vzlogging
