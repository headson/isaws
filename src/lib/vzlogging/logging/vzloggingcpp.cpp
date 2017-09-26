/************************************************************************
*Author      : Sober.Peng 17-06-27
*Description :
************************************************************************/
#include "vzlogging/logging/vzlogging.h"
#include "vzlogging/logging/vzloggingcpp.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "vzlogging/base/vzbases.h"
#include "vzlogging/base/vzcommon.h"

extern int Write(vzlogging::CVzLogSock *ptls,
                 bool local_print, int level,
                 const char *slog, int nlog);

namespace vzlogging {

/**日志流格式化**********************************************************/
CVzLogStream::CVzLogStream(int          n_level,
                           const char*  p_file,
                           int          n_line,
                           unsigned int b_print) {
  local_print_ = b_print;
  nlevel_      = n_level;

  ptls_        = GetVzLogSock();
  if (!k_log_print) {
    if (local_print_ == 1) {
#ifdef NDEBUG
      ptls_ = NULL;
#endif
    }
  }

  if (ptls_) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    psock->nlog = VzLogHead(n_level, p_file, n_line,
                            psock->slog, psock->max_nlog);
  }
}

vzlogging::CVzLogStream::~CVzLogStream() {
  if (ptls_) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return;
    }

    if (psock->nlog <= psock->max_nlog) {  // 分配日志buffer多了4字节,可存'\n\0'
      psock->slog[psock->nlog++] = '\n';
    }

    if (nlevel_ != L_HEARTBEAT) {
      if (local_print_ == 1 || k_log_print) {
        psock->slog[psock->nlog] = '\0';
        VzLogPrint(psock->slog, psock->nlog);
      }
    }

    /*if ((b_local_print_ == 0) &&
        (n_level_ >= k_shm_arg.GetLevel())) {
        psock->Write(k_shm_arg.GetSockAddr(), psock->slog, psock->nlog);
        }*/
    Write(psock, local_print_, nlevel_, psock->slog, psock->nlog);
  }
}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(char val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
  if (ptls_) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }

    psock->nlog += snprintf(psock->slog + psock->nlog,
                            psock->max_nlog - psock->nlog,
                            "%c", val);
  }
  return *this;
}


vzlogging::CVzLogStream&
CVzLogStream::operator<<(unsigned char val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
  if (ptls_) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }

    psock->nlog += snprintf(psock->slog + psock->nlog,
                            psock->max_nlog - psock->nlog,
                            "%u", (int)val);
  }
  return *this;
}

vzlogging::CVzLogStream&
CVzLogStream::operator<<(bool val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
  if (ptls_) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }

    psock->nlog += snprintf(psock->slog + psock->nlog,
                            psock->max_nlog - psock->nlog,
                            "%s", (val ? "true" : "false"));
  }
  return *this;
}

vzlogging::CVzLogStream&
CVzLogStream::operator<<(short val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
  if (ptls_) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }

    psock->nlog += snprintf(psock->slog + psock->nlog,
                            psock->max_nlog - psock->nlog,
                            "%d", (int)val);
  }
  return *this;
}

vzlogging::CVzLogStream&
CVzLogStream::operator<<(unsigned short val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
  if (ptls_) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }

    psock->nlog += snprintf(psock->slog + psock->nlog,
                            psock->max_nlog - psock->nlog,
                            "%u", (int)val);
  }
  return *this;
}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(int t) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
  if (ptls_) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }

    psock->nlog += snprintf(psock->slog + psock->nlog,
                            psock->max_nlog - psock->nlog,
                            "%d", t);
  }
  return *this;
}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(unsigned int t) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
  if (ptls_) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }

    psock->nlog += snprintf(psock->slog + psock->nlog,
                            psock->max_nlog - psock->nlog,
                            "%u", t);
  }
  return *this;
}

vzlogging::CVzLogStream&
CVzLogStream::operator<<(long val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
  if (ptls_) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }

    psock->nlog += snprintf(psock->slog + psock->nlog,
                            psock->max_nlog - psock->nlog,
                            "%ld", val);
  }
  return *this;
}

vzlogging::CVzLogStream&
CVzLogStream::operator<<(unsigned long val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
  if (ptls_) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }

    psock->nlog += snprintf(psock->slog + psock->nlog,
                            psock->max_nlog - psock->nlog,
                            "%lu", val);
  }
  return *this;
}

vzlogging::CVzLogStream&
CVzLogStream::operator<<(long long val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
  if (ptls_) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }

    psock->nlog += snprintf(psock->slog + psock->nlog,
                            psock->max_nlog - psock->nlog,
                            "%lld", val);
  }
  return *this;
}

vzlogging::CVzLogStream&
CVzLogStream::operator<<(unsigned long long val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
  if (ptls_) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }

    psock->nlog += snprintf(psock->slog + psock->nlog,
                            psock->max_nlog - psock->nlog,
                            "%llu", val);
  }
  return *this;
}

vzlogging::CVzLogStream&
CVzLogStream::operator<<(float val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
  if (ptls_) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }

    psock->nlog += snprintf(psock->slog + psock->nlog,
                            psock->max_nlog - psock->nlog,
                            "%f", val);
  }
  return *this;
}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(double val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
  if (ptls_) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }

    psock->nlog += snprintf(psock->slog + psock->nlog,
                            psock->max_nlog - psock->nlog,
                            "%f", val);
  }
  return *this;
}


vzlogging::CVzLogStream&
CVzLogStream::operator<<(long double val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
  if (ptls_) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }

    psock->nlog += snprintf(psock->slog + psock->nlog,
                            psock->max_nlog - psock->nlog,
                            "%Lf", val);
  }
  return *this;
}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(const char* val) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
  if (ptls_ && val) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }
    //if (strlen(val) > (psock->max_nlog - psock->nlog)) {
    //  return *this;
    //}

    psock->nlog += snprintf(psock->slog + psock->nlog,
                            psock->max_nlog - psock->nlog,
                            "%s", val);
  }
  return *this;
}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(std::string str) {
//printf("%s[%d]\n", __FUNCTION__, __LINE__);
  if (ptls_ && str.size()>0) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }
    //if (str.size() >(psock->max_nlog - psock->nlog)) {
    //  return *this;
    //}

    psock->nlog += snprintf(psock->slog + psock->nlog,
                            psock->max_nlog - psock->nlog,
                            "%s", str.c_str());
  }
  return *this;
}

//vzlogging::CVzLogStream&
//vzlogging::CVzLogStream::operator<<(const time_t tt) {
//  //printf("%s[%d]\n", __FUNCTION__, __LINE__);
//  if (p_tls_) {
//    CTlsLog* psock = (CTlsLog*)p_tls_;
//    if (psock->nlog_ < 0) {
//      return *this;
//    }
//
//    psock->nlog_ += snprintf(psock->slog_ + psock->nlog_,
//                               psock->nlog_max_ - psock->nlog_,
//                               "%d", tt);
//  }
//  return *this;
//}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::write(const char* s_msg, int n_msg) {
  if (ptls_ && s_msg && n_msg > 0) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }

    int n_left = psock->max_nlog - psock->nlog; // 剩余空间
    int n_less = (n_left > n_msg) ? n_msg : n_left;     // 消息与剩余字符串那个小

    memcpy(psock->slog + psock->nlog, s_msg, n_less);
    psock->nlog += n_less;
  }
  return *this;
}

}  // namespace vzlogging
