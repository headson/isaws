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
#include "vthread.h"

#ifdef WIN32

#else   // LINUX

//#define  _GNU_SOURCE       /* or _BSD_SOURCE or _SVID_SOURCE */
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */

#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <sched.h>
#include <semaphore.h>
#include <sys/timeb.h>
#endif

#include "base/stdafx.h"

///THREAD//////////////////////////////////////////////////////////////////////
VThread::VThread()
  : isInit_(false)
  , callback_(NULL)
  , user_arg_(NULL) {
}

VThread::~VThread() {
  Stop();
}

int32_t VThread::Start(VFTHREAD pFunc, void* pArg, int32_t nPriority) {
  callback_ = pFunc;
  user_arg_ = pArg;
  if (callback_ == NULL) {
    return -1;
  }

#ifdef WIN32
  pid_ = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)callback_, user_arg_, 0, (DWORD*)&pid_);
  if (pid_ == NULL) {
    return -2;
  }
#else
  int32_t nRet = pthread_create(&pid_, NULL, callback_, user_arg_);
  if (nRet != 0) {
    return -2;
  }
#endif

  isInit_ = true;
  return 0;
}

void VThread::Stop() {
  if (!isInit_)
    return;

  if (isInit_) {
#ifdef WIN32
    if (pid_ != NULL) {
      WaitForSingleObject(pid_, INFINITE);
      if(pid_ != NULL)
        TerminateThread(pid_, 0);
      pid_ = NULL;
    }
#else
    pthread_join(pid_, 0);
#endif
  }
  isInit_ = false;
}

void VThread::Detach() {
  if (isInit_) {
#ifdef WIN32
#else
    pthread_detach(pid_);
#endif
  }
}

int32_t VThread::Kill() {
#ifdef WIN32
  return 0;
#else
  return pthread_kill(pid_, 0);
#endif
}

int32_t VThread::get_pid() {
  int32_t id = 0;
#if defined(WIN32)
  id = GetCurrentThreadId();
#else  // WIN32
  //id = pthread_self();
  id = syscall(224);
#endif // WIN32
  return id;
}

void VThread::run_cmd(const char* sCmd) {
#ifndef WIN32
  int pid = fork(); //vfork()
  if(pid == 0) {
    signal(SIGQUIT, SIG_DFL);
    signal(SIGINT,  SIG_DFL);
    signal(SIGCHLD, SIG_DFL);
    execl("/bin/sh", "sh", "-c", sCmd, (char *)0);
    _exit(127);
  } else if(pid == -1) {
    perror("this fork failed.");
  }
  //system(command);
#endif
}

void VThread::set_cpu(int32_t nCpu) {
#ifndef WIN32
  cpu_set_t mask;

  CPU_ZERO(&mask);
  CPU_SET(nCpu, &mask);
  pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask);
#endif
}

void VThread::msleep(long msec) {
#ifdef WIN32
  ::Sleep(msec);
#else
  struct timeval t;
  t.tv_sec  = msec / 1000;
  t.tv_usec = msec * 1000 % 1000000;
  select(0+1, NULL, NULL, NULL, &t);
#endif
}

///EX THREAD///////////////////////////////////////////////////////////////////
VExThread::VExThread()
  : _cThread()
  , _isRuning(false) {
}

VExThread::~VExThread() {
}

int32_t VExThread::Start(int32_t nPriority) {
  if (_isRuning) {
    return 0;
  }

  _isRuning = true;
  return _cThread.Start(thread_func_, this, nPriority);
}

void VExThread::Stop() {
  _isRuning = false;
  _cThread.Stop();
}

bool VExThread::is_runing() {
  return _isRuning;
}

void* VExThread::thread_func_(void *pArg) {
  if (NULL != pArg) {
    ((VExThread*)pArg)->Process();
    ((VExThread*)pArg)->_isRuning = false;
  }
  return NULL;
}
