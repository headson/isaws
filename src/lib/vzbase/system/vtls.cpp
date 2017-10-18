/************************************************************************/
/* Author      : Sober.Peng 17-05-24
/* Description : 
/************************************************************************/
#include "vtls.h"
#include "vzbase/helper/stdafx.h"

namespace vzbase {

  VTls::VTls() 
    : tls_key_(TLS_NULL)
  {
    LOG_INFO("%s[%d].\n", __FUNCTION__, __LINE__);
  }


  VTls::~VTls()
  {
    KeyFree();
    LOG_INFO("%s[%d].\n", __FUNCTION__, __LINE__);
  }

  TlsKey VTls::KeyAlloc()
  {
    if (!IsReady()) {
#ifdef WIN32
      tls_key_ = TlsAlloc();
#else
      pthread_key_create(&tls_key_, NULL);
#endif
    }
    return tls_key_;
  }

  void VTls::KeyFree()
  {
    if (tls_key_ != TLS_NULL) {
#ifdef WIN32
      TlsFree(tls_key_);
#else
      pthread_key_delete(tls_key_);
#endif
      tls_key_ = TLS_NULL;
    }
  }

  bool VTls::IsReady()
  {
    if (TLS_NULL != tls_key_) {
      return true;
    }
    return false;
  }

  bool VTls::SetValue(void* value)
  {
    if (!IsReady()) {
      return false;
    }
#ifdef WIN32
    return TlsSetValue(tls_key_, value) == TRUE;
#else
    return pthread_setspecific(tls_key_, value);
#endif
  }

  void* VTls::GetValue()
  {
    if (!IsReady()) {
      return NULL;
    }
#ifdef WIN32
    return TlsGetValue(tls_key_);
#else
    return pthread_getspecific(tls_key_);
#endif
  }

}  // namespace yhbase
