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

//#define DBG_PRINT printf

extern int Write(vzlogging::CVzLogSock *ptls,
                 bool local_print, int level,
                 const char *slog, int nlog);

namespace vzlogging {

/**日志流格式化**********************************************************/
CVzLogStream::CVzLogStream(int          nlevel,
                           const char*  pfile,
                           int          nline,
                           unsigned int bprint) {
  local_print_ = bprint;
  nlevel_      = nlevel;

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
    psock->nlog = VzLogHead(nlevel, pfile, nline,
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
  //DBG_PRINT("%s[%d] (char val)\n", __FUNCTION__, __LINE__);
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
  //DBG_PRINT("%s[%d] (unsigned char val)\n", __FUNCTION__, __LINE__);
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
  //DBG_PRINT("%s[%d] (bool val)\n", __FUNCTION__, __LINE__);
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
  //DBG_PRINT("%s[%d] (short val)\n", __FUNCTION__, __LINE__);
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
  //DBG_PRINT("%s[%d] (unsigned short val)\n", __FUNCTION__, __LINE__);
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
  //DBG_PRINT("%s[%d] (int t)\n", __FUNCTION__, __LINE__);
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
  //DBG_PRINT("%s[%d] (unsigned int t)\n", __FUNCTION__, __LINE__);
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
  //DBG_PRINT("%s[%d] (long val)\n", __FUNCTION__, __LINE__);
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
  //DBG_PRINT("%s[%d] (unsigned long val)\n", __FUNCTION__, __LINE__);
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
  //DBG_PRINT("%s[%d] (long long val)\n", __FUNCTION__, __LINE__);
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
  //DBG_PRINT("%s[%d] (unsigned long long val)\n", __FUNCTION__, __LINE__);
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
  //DBG_PRINT("%s[%d] (float val)\n", __FUNCTION__, __LINE__);
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
  //DBG_PRINT("%s[%d] (double val)\n", __FUNCTION__, __LINE__);
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
  //DBG_PRINT("%s[%d] (long double val)\n", __FUNCTION__, __LINE__);
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
  //DBG_PRINT("%s[%d] (const char* val)\n", __FUNCTION__, __LINE__);
  if (ptls_ && val) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }
    //if (strlen(val) > (psock->max_nlog - psock->nlog)) {
    //  return *this;
    //}

#ifndef _WIN32
    psock->nlog += snprintf(psock->slog + psock->nlog,
                            psock->max_nlog - psock->nlog,
                            "%s", val);
#else
    int left_size = psock->max_nlog - psock->nlog;
    if (strlen(val) < left_size) {
      left_size = strlen(val);
    }
    memcpy(psock->slog + psock->nlog, val, left_size);
    psock->nlog += left_size;
#endif
  }
  return *this;
}

vzlogging::CVzLogStream&
vzlogging::CVzLogStream::operator<<(std::string str) {
  //DBG_PRINT("%s[%d] (std::string str)\n", __FUNCTION__, __LINE__);
  if (ptls_ && str.size()>0) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }
    //if (str.size() >(psock->max_nlog - psock->nlog)) {
    //  return *this;
    //}
#ifndef _WIN32
    psock->nlog += snprintf(psock->slog + psock->nlog,
                            psock->max_nlog - psock->nlog,
                            "%s", str.c_str());
#else
    int left_size = psock->max_nlog - psock->nlog;
    if (str.size() < left_size) {
      left_size = str.size();
    }
    memcpy(psock->slog + psock->nlog, str.c_str(), left_size);
    psock->nlog += left_size;
#endif
  }
  return *this;
}

//vzlogging::CVzLogStream&
//vzlogging::CVzLogStream::operator<<(const time_t tt) {
//  //DBG_PRINT("%s[%d]\n", __FUNCTION__, __LINE__);
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
vzlogging::CVzLogStream::write(const char* smsg, int nmsg) {
  if (ptls_ && smsg && nmsg > 0) {
    CVzLogSock* psock = (CVzLogSock*)ptls_;
    if (psock->nlog < 0) {
      return *this;
    }

    int n_left = psock->max_nlog - psock->nlog;       // 剩余空间
    int n_less = (n_left > nmsg) ? nmsg : n_left;     // 消息与剩余字符串那个小

    memcpy(psock->slog + psock->nlog, smsg, n_less);
    psock->nlog += n_less;
  }
  return *this;
}

}  // namespace vzlogging
