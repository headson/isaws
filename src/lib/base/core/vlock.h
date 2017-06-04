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
#include "inc/vtypes.h"

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
    void    lock();
    void    unlock();

    bool    try_lock(long msec);

    bool    is_locked();

private:
    volatile bool       isLock_;

#ifdef WIN32
    CRITICAL_SECTION    cMutex_;
#else
    pthread_mutex_t     cMutex_;
#endif
};

class VSignal
{
public:
    VSignal();
    ~VSignal();

    // 1=接收到POST，0=超时
    int32_t waits(long msec=0);
    void    notify();

private:
    bool    isInit_;

#ifdef WIN32
    HANDLE  cHandle_;
#else
    sem_t   cHandle_;
#endif
};

class VLockGuard
{
public:
    VLockGuard(VMutex& cMutex);
    ~VLockGuard();

private:
    VMutex&     cMutex_;
};


