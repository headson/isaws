/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : vthread.h
* Author        : Sober.Peng
* Date          : 2016-3-10
* Description   :
*-----------------------------------------------------------------------------
* Modify        :
*-----------------------------------------------------------------------------
******************************************************************************/
#pragma once
#include "base/vtypes.h"

#ifdef WIN32
#include <windows.h>
#else   // LINUX
#include <pthread.h>
#endif

typedef void* (*VFTHREAD)(void*);
class VThread {
 public:
  VThread();
  ~VThread();

 public:
  int32_t         Start(VFTHREAD pFunc, void* pArg, int32_t nPriority=0); // �����߳�
  void            Stop();

  void            Detach();
  int32_t         Kill();

  static int32_t  get_pid();
  static void     run_cmd(const char* sCmd);
  static void     set_cpu(int32_t nCpu);
  // ˯�ߺ���
  // nMSec[IN] ����
  static void     msleep(long msec);

 private:
  bool            isInit_;    // ��ʼ���ɹ�
#ifdef WIN32
  HANDLE          pid_;
#else
  pthread_t       pid_;
#endif
  VFTHREAD        callback_;  // �̻߳ص�FUNCTION
  void*           user_arg_;  // �̲߳���
};

class VExThread {
 public:
  VExThread();
  virtual ~VExThread();

 public:
  virtual int32_t Start(int32_t nPriority=0);
  virtual void    Stop();
  virtual void    Process() = 0;

  bool            is_runing();

 protected:
  static void*    thread_func_(void *pArg);

 protected:
  VThread         _cThread;
  volatile bool   _isRuning;
};
