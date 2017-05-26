/************************************************************************/
/* Author      : Sober.Peng 17-05-24
/* Description :
/************************************************************************/
#ifndef LIBYHBASE_VTLS_H
#define LIBYHBASE_VTLS_H

#include "vdefine.h"

#ifdef WIN32
// TLS
#include <windows.h>
#include <process.h>
typedef DWORD               TlsKey;
#define TLS_NULL            0
#else
#include <pthread.h>
typedef pthread_key_t       TlsKey;
#define TLS_NULL            -1
#endif

namespace yhbase {

  class VTls {
  public:
    VTls();
    ~VTls();

    // 启动进程时调用一次，分配关键字
    TlsKey KeyAlloc();
    void   KeyFree();

    // true可用,false不可用
    bool   IsReady();

    // 线程中调用，关联关键字和私有数据对应指针
    bool   SetValue(void* value);
    // 线程中调用，获取关键字关联的线程私有数据
    void*  GetValue();

  private:
    TlsKey  tls_key_;
  };

}

#endif  // LIBYHBASE_VTLS_H
