/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* filename: cevttype.h
* describe:
* author  : Sober.Peng
* datetime: 2016-11-24
******************************************************************************/
#include "stdafx.h"
#include "cevttype.h"
#include "cdatabase.h"

CEvtTypeM::CEvtTypeM() {
    cEvtMap_.clear();
}
CEvtTypeM::~CEvtTypeM() {
}

int32_t CEvtTypeM::Initinal()
{
    QVector<CEvtType> cVET;
    DATABASE()->SelectTableEventType(cVET);
    for (int32_t i = 0; i < cVET.size(); i++) {
        Insert(cVET.at(i));
    }

    return cEvtMap_.size();
}

bool CEvtTypeM::isEvent( int32_t nEvt )
{
    QMutexLocker al(&vMutex_);

    CEvtType cEt;
    EVTT_ITER it = cEvtMap_.find(nEvt);
    if (it != cEvtMap_.end()) {
        return true;
    }
    return false;
}

CEvtType CEvtTypeM::GetEvtType(int32_t nEvt)
{
    QMutexLocker al(&vMutex_);

    CEvtType cEt;
    EVTT_ITER it = cEvtMap_.find(nEvt);
    if (it != cEvtMap_.end()) {
        cEt = it.value();
    }
    return cEt;
}

QString CEvtTypeM::GetEvtName(int32_t nEvt)
{
    QMutexLocker al(&vMutex_);
    EVTT_ITER it = cEvtMap_.find(nEvt);
    if (it != cEvtMap_.end()) {
        return it->sName_;
    }
    return "";
}

void CEvtTypeM::Insert(const CEvtType& cet)
{
    QMutexLocker al(&vMutex_);

    EVTT_ITER it = cEvtMap_.find(cet.nId_);
    if (it != cEvtMap_.end()) {
        return;
    }

    cEvtMap_.insert(cet.nId_, cet);
}

