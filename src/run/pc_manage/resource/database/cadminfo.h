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

class CAdmInfo
{
public:
    CAdmInfo(){
        username_  = "";
        password1_ = "";
        password2_ = "";

        country_   = "";
        phone_     = "";

        login_ip_  = "";
        login_mac_ = "";

        note_      = "";
    }

public:
    QString     username_;
    QString     password1_;
    QString     password2_;

    QString     country_;
    QString     phone_;

    QString     login_ip_;
    QString     login_mac_;

    int32_t     right_id_;

    QString     note_;
};

