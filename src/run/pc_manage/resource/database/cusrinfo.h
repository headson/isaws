/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* filename: cusrinfo.h
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
        id_       = "";
        username_ = "";
        password_ = "";

        user_right_id_ = 2000;
    }

    CUsrInfo(const CUsrInfo& cUsr) {
        id_             = cUsr.id_;
        username_       = cUsr.username_;
        password_       = cUsr.password_;
        user_right_id_  = cUsr.user_right_id_;
    }

public:
    QString     id_;
    QString     username_;
    QString     password_;

    int32_t     user_right_id_;
};

