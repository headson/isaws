/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* filename: cusrfaceinfo.h
* describe:
* author  : 
* datetime: 2016-8-23
******************************************************************************/
#pragma once

#include <stdint.h>

#include <QString>

class CUsrFaceInfo
{
public:
    CUsrFaceInfo() {
        usr_no            = "NULL";
        username          = "NULL";
        regtime           = "NULL";
        opendoorstarttime = "NULL";
        opendoorendtime   = "NULL";
    }

    void Update(const CUsrFaceInfo& cUsr) {
        usr_no   = cUsr.usr_no;
        username = cUsr.username;
        regtime  = cUsr.regtime;
        opendoorstarttime = cUsr.opendoorstarttime;
        opendoorendtime   = cUsr.opendoorendtime;
    }

public:
    QString usr_no;              /* ����        */
    QString username;            /* �û���      */
    QString regtime;             /* ��ְʱ��    */
    QString opendoorstarttime;   /* ���ſ�ʼʱ��*/
    QString opendoorendtime;     /* ���Ž���ʱ��*/
};

