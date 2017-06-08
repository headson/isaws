/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* Filename      : vbaseevent.h
* Author        : Sober.Peng
* Date          : 18:1:2017
* Description   :
*-----------------------------------------------------------------------------
* Modify        : 
*-----------------------------------------------------------------------------
******************************************************************************/
#pragma once
#include "inc/vtypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "event2/event-config.h"

#include "event2/util.h"
#include "event2/event.h"
#include "event2/buffer.h"
#include "event2/thread.h"
#include "event2/listener.h"
#include "event2/bufferevent.h"
#include "event2/event_compat.h"
#include "event2/event_struct.h"

#ifdef __cplusplus
}
#endif

#include <string>

typedef int32_t (*EVT_FUNC)(int32_t nEvt, const void* ctx);

#define EVT_READ        EV_READ
#define EVT_WRITE       EV_WRITE
#define EVT_PERSIST     EV_PERSIST
///LOOP////////////////////////////////////////////////////////////////////////
class EVT_LOOP
{
public:
    EVT_LOOP();
    virtual ~EVT_LOOP();

    int32_t         Start();
    void            Stop();

    void            Runing(bool* b_runing);

    struct event_base* get_event() const {
      return m_pEvent;
    }

protected:
    struct event_base* m_pEvent;
};

///TIMER///////////////////////////////////////////////////////////////////////
 class EVT_TIMER
{
public:
    struct event    m_cEvt;      // 
    EVT_LOOP*       m_pLoop;     // 

    EVT_FUNC        m_pCallback; // 消息回调
    void*           m_pUserArgs; // 回调参数

    std::string     m_sName;     // 名称
    bool            m_bStart;

    uint32_t        m_nAfter;    // 
    uint32_t        m_nRepeat;   // 

public:
    EVT_TIMER();    
    void            set_name(const std::string& sName);
    const std::string&   get_name() const;

    void            Init(const EVT_LOOP* loop, EVT_FUNC func, void* pArg);
    int32_t         Start(uint32_t after, uint32_t repeat);
    void            Stop();

    static void     evt_callback(int fd, short event, void *ctx);
};

///IO//////////////////////////////////////////////////////////////////////////
class EVT_IO
{
public:
    struct event    m_cEvt;       // 
    EVT_LOOP*       m_pLoop;      // 

    EVT_FUNC        m_pCallback;  // 消息回调
    void*           m_pUserArgs;  // 回调参数

    std::string     m_sName;      // 名称
    bool            m_bStart;
    int32_t         m_nIdleCall;  // 轮空

public:
    EVT_IO();
    
    bool            is_start();

    void            inc_idlecall() { m_nIdleCall++; }
    int32_t         get_idlecall() { return m_nIdleCall; }
    void            set_idlecall(int32_t n) { m_nIdleCall = n; }

    void            set_name(const std::string& sName);
    const std::string&   get_name() const;

    void            Init(const EVT_LOOP* loop, EVT_FUNC func, void* pArg);
    int32_t         Start(SOCKET vHdl, int32_t nEvt);
    void            Stop();

    static void     evt_callback(evutil_socket_t fd, short events, void *ctx);
};
