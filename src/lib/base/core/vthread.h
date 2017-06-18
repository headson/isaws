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
#include "basictypes.h"

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
  int32         Start(VFTHREAD pFunc, void* pArg, int32 nPriority=0); // 启动线程
  void            Stop();

  void            Detach();
  int32         Kill();

  static int32  get_pid();
  static void     run_cmd(const char* sCmd);
  static void     set_cpu(int32 nCpu);
  // 睡眠毫秒
  // nMSec[IN] 毫秒
  static void     msleep(long msec);

 private:
  bool            isInit_;    // 初始化成功
#ifdef WIN32
  HANDLE          pid_;
#else
  pthread_t       pid_;
#endif
  VFTHREAD        callback_;  // 线程回调FUNCTION
  void*           user_arg_;  // 线程参数
};

class VExThread {
 public:
  VExThread();
  virtual ~VExThread();

 public:
  virtual int32 Start(int32 nPriority=0);
  virtual void    Stop();
  virtual void    Process() = 0;

  bool            is_runing();

 protected:
  static void*    thread_func_(void *pArg);

 protected:
  VThread         _cThread;
  volatile bool   _isRuning;
};
