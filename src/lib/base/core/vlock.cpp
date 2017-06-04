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
#include "inc/vdefine.h"

///MUTEX///////////////////////////////////////////////////////////////////////
VMutex::VMutex() 
    : isLock_(false)
{
#ifdef WIN32
    InitializeCriticalSection(&cMutex_);
#else
    // 支持嵌入
    pthread_mutexattr_t m_cMutexAttr;
    pthread_mutexattr_init(&m_cMutexAttr);
    pthread_mutexattr_settype(&m_cMutexAttr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&cMutex_, &m_cMutexAttr);

    //pthread_mutex_init(&m_cMutex, NULL);
#endif
}

VMutex::~VMutex()
{
#ifdef WIN32
    DeleteCriticalSection(&cMutex_);
#else
    pthread_mutex_destroy(&cMutex_);
#endif

    isLock_ = false;
}

void VMutex::lock()
{
#ifdef WIN32
    EnterCriticalSection(&cMutex_);
#else
    if (pthread_mutex_lock(&cMutex_) == 0) {
        isLock_ = true;
    }
#endif
}

void VMutex::unlock()
{
#ifdef WIN32
    LeaveCriticalSection(&cMutex_);
#else
    if (pthread_mutex_unlock(&cMutex_) == 0) {
        isLock_ = false;
    }
#endif
}

bool VMutex::try_lock(long msec)
{
    bool bLock = false;

#ifdef WIN32
    bLock = (TryEnterCriticalSection(&cMutex_) ? true : false);
#else
    bLock = (pthread_mutex_trylock(&cMutex_) == 0) ? true : false;
#endif

    return bLock;
}

bool VMutex::is_locked()
{
    return isLock_;
}

///SIGNAL//////////////////////////////////////////////////////////////////////
VSignal::VSignal() 
    : isInit_(true)
{
#ifdef WIN32
    cHandle_ = CreateSemaphore(NULL, 0, MAXLONG, NULL);
    if (cHandle_ == NULL) {
        isInit_ = false;
    }
#else
    if (sem_init(&cHandle_, 0, 0) < 0)  {
        isInit_ = false;
    }
#endif
}

VSignal::~VSignal()
{
    isInit_ = false;
#ifdef WIN32
    CloseHandle(cHandle_);
#else
    sem_destroy(&cHandle_);
#endif
}

int32_t VSignal::waits(long msec)
{
    int32_t nRet = 0;
    if (isInit_ == false) {
        return -1;
    }

#ifdef WIN32
    if(msec > 0){
        nRet = (WaitForSingleObject(cHandle_, msec) == WAIT_OBJECT_0);
    } else {
        nRet = (WaitForSingleObject(cHandle_, INFINITE) == WAIT_OBJECT_0);
    }
#else
    if(msec > 0)
    {
        struct timeb tb; ftime(&tb);

        struct timespec ts;
        ts.tv_sec = tb.time+(tb.millitm+msec)/1000;
        ts.tv_nsec = ((tb.millitm+msec)%1000)*1000L*1000;
        nRet = sem_timedwait(&cHandle_, &ts);  // 0=接收到POST，-1错误
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
        nRet = sem_wait(&cHandle_);
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

void VSignal::notify()
{
    if (isInit_ == false) {
        return;
    }
#ifdef WIN32
    ReleaseSemaphore(cHandle_, 1, NULL);
#else
    sem_post(&cHandle_);
#endif
}

///LOCK GUARD//////////////////////////////////////////////////////////////////
VLockGuard::VLockGuard(VMutex& cMutex) 
    : cMutex_(cMutex)
{
    cMutex_.lock();
}

VLockGuard::~VLockGuard()
{
    cMutex_.unlock();
}
