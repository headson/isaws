/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : vlock.h
* Author        : Sober.Peng
* Date          : 20:1:2017
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
#include <errno.h>
//#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <sched.h>
#include <semaphore.h>
#include <sys/timeb.h>
#endif

class VMutex
{
public:
    VMutex();
    ~VMutex();

public:
    void    Lock();
    void    Unlock();

    bool    TryLock(long msec);

    bool    IsLock();

private:
    volatile bool       b_lock_;

#ifdef WIN32
    CRITICAL_SECTION    c_lock_;
#else
    pthread_mutex_t     c_lock_;
#endif
};

class VSignal
{
public:
    VSignal();
    ~VSignal();

    // 1=接收到POST，0=超时
    int32_t Wait(long msec=0);
    void    Signal();

private:
    bool    b_init_;

#ifdef WIN32
    HANDLE  c_handle_;
#else
    sem_t   c_handle_;
#endif
};

class VLockGuard
{
public:
    VLockGuard(VMutex& c_mutex);
    ~VLockGuard();

private:
    VMutex&     c_mutex_;
};


