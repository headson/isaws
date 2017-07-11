/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* filename: cevttype.h
* describe:
* author  : Sober.Peng
* datetime: 2016-11-24
******************************************************************************/
#pragma once

#include <stdint.h>

#include <QMap>
#include <QMutex>
#include <QString>

class CEvtType
{
public:
    CEvtType() {
        nId_    = -1;
        sName_  = "";
        bView_  = 0;
        bSend_  = 0;
        bPopup_ = 0;
    }

    CEvtType(const CEvtType& cet) {
        nId_    = cet.nId_   ;
        sName_  = cet.sName_ ;
        bView_  = cet.bView_ ;
        bSend_  = cet.bSend_ ;
        bPopup_ = cet.bPopup_;
    }

    CEvtType operator=(const CEvtType& cet) {
        nId_    = cet.nId_   ;
        sName_  = cet.sName_ ;
        bView_  = cet.bView_ ;
        bSend_  = cet.bSend_ ;
        bPopup_ = cet.bPopup_;
        return *this;
    }

public:
    int32_t nId_;       // 事件ID
    QString sName_;     // 事件名

    int32_t bView_;     // 查看
    int32_t bSend_;     // 发送
    int32_t bPopup_;    // 弹窗
};

class CEvtTypeM 
{
public:
    typedef QMap<int32_t, CEvtType> EVTT_MAP;
    typedef EVTT_MAP::Iterator      EVTT_ITER;

public:
    CEvtTypeM();
    ~CEvtTypeM();

    int32_t Initinal();

    bool    isEvent(int32_t nEvt);

    QString GetEvtName(int32_t nEvt);
    CEvtType GetEvtType(int32_t nEvt);
    
    void Insert(const CEvtType& cet);

    EVTT_MAP GetEvtMap() {
        return cEvtMap_;
    }

private:
    QMutex                  vMutex_;
    QMap<int32_t, CEvtType> cEvtMap_;
};

