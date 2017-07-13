/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* filename: cadminfo.h
* describe:
* author  : Sober.Peng
* datetime: 2016-7-26
******************************************************************************/
#pragma once

#include <stdint.h>

#include <QString>

class CUsrInfo
{
public:
    CUsrInfo(){
        username_ = "";
        password_ = "";
        right_id_ = 0;
        note_     = "";
    }

public:
    QString       username_;
    QString       password_;
    unsigned int  right_id_;
    QString       note_;
};

