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

    // ��������ʱ����һ�Σ�����ؼ���
    TlsKey KeyAlloc();
    void   KeyFree();

    // true����,false������
    bool   IsReady();

    // �߳��е��ã������ؼ��ֺ�˽�����ݶ�Ӧָ��
    bool   SetValue(void* value);
    // �߳��е��ã���ȡ�ؼ��ֹ������߳�˽������
    void*  GetValue();

  private:
    TlsKey  tls_key_;
  };

}

#endif  // LIBYHBASE_VTLS_H
