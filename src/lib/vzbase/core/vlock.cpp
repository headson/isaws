/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : vlock.cpp
* Author        : Sober.Peng
* Date          : 20:1:2017
* Description   :
*-----------------------------------------------------------------------------
* Modify        : 
*-----------------------------------------------------------------------------
******************************************************************************/
#include "vlock.h"
#include "stdafx.h"

///MUTEX///////////////////////////////////////////////////////////////////////
VMutex::VMutex() 
    : b_lock_(false)
{
#ifdef WIN32
    InitializeCriticalSection(&c_lock_);
#else
    // 支持嵌入
    pthread_mutexattr_t m_cMutexAttr;
    pthread_mutexattr_init(&m_cMutexAttr);
    pthread_mutexattr_settype(&m_cMutexAttr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&c_lock_, &m_cMutexAttr);

    //pthread_mutex_init(&m_cMutex, NULL);
#endif
}

VMutex::~VMutex()
{
#ifdef WIN32
    DeleteCriticalSection(&c_lock_);
#else
    pthread_mutex_destroy(&c_lock_);
#endif

    b_lock_ = false;
}

void VMutex::Lock()
{
#ifdef WIN32
    EnterCriticalSection(&c_lock_);
#else
    if (pthread_mutex_lock(&c_lock_) == 0) {
        b_lock_ = true;
    }
#endif
}

void VMutex::Unlock()
{
#ifdef WIN32
    LeaveCriticalSection(&c_lock_);
#else
    if (pthread_mutex_unlock(&c_lock_) == 0) {
        b_lock_ = false;
    }
#endif
}

bool VMutex::TryLock(long msec)
{
    bool bLock = false;

#ifdef WIN32
    bLock = (TryEnterCriticalSection(&c_lock_) ? true : false);
#else
    bLock = (pthread_mutex_trylock(&c_lock_) == 0) ? true : false;
#endif

    return bLock;
}

bool VMutex::IsLock()
{
    return b_lock_;
}

///SIGNAL//////////////////////////////////////////////////////////////////////
VSignal::VSignal() 
    : b_init_(true)
{
#ifdef WIN32
    c_handle_ = CreateSemaphore(NULL, 0, MAXLONG, NULL);
    if (c_handle_ == NULL) {
        b_init_ = false;
    }
#else
    if (sem_init(&c_handle_, 0, 0) < 0)  {
        b_init_ = false;
    }
#endif
}

VSignal::~VSignal()
{
    b_init_ = false;
#ifdef WIN32
    CloseHandle(c_handle_);
#else
    sem_destroy(&c_handle_);
#endif
}

int32 VSignal::Wait(long msec)
{
    int32 nRet = 0;
    if (b_init_ == false) {
        return -1;
    }

#ifdef WIN32
    if(msec > 0){
        nRet = (WaitForSingleObject(c_handle_, msec) == WAIT_OBJECT_0);
    } else {
        nRet = (WaitForSingleObject(c_handle_, INFINITE) == WAIT_OBJECT_0);
    }
#else
    if(msec > 0)
    {
        struct timeb tb; ftime(&tb);

        struct timespec ts;
        ts.tv_sec = tb.time+(tb.millitm+msec)/1000;
        ts.tv_nsec = ((tb.millitm+msec)%1000)*1000L*1000;
        nRet = sem_timedwait(&c_handle_, &ts);  // 0=接收到POST，-1错误
        if (nRet != 0) 
        {
            if (errno == ETIMEDOUT) 
            {
                nRet = 0;   // 超时
            }
        }
        else
        {
            nRet = 1;       // 接收到POST
        }
    }
    else
    {
        nRet = sem_wait(&c_handle_);
        if (nRet != 0) 
        {
            nRet = 0;   // 超时
        }
        else
        {
            nRet = 1;   // 接收到POST
        }
    }
#endif
    return nRet;
}

void VSignal::Signal()
{
    if (b_init_ == false) {
        return;
    }
#ifdef WIN32
    ReleaseSemaphore(c_handle_, 1, NULL);
#else
    sem_post(&c_handle_);
#endif
}

///LOCK GUARD//////////////////////////////////////////////////////////////////
VLockGuard::VLockGuard(VMutex& c_lock) 
    : c_mutex_(c_lock)
{
    c_mutex_.Lock();
}

VLockGuard::~VLockGuard()
{
    c_mutex_.Unlock();
}
