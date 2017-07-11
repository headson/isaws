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

class CAesInfo
{
public:
    CAesInfo() {
        id_             = "";
        name_           = "NULL";
        islock_         = 0;
        wan_ip_         = "NULL";
        lan_ip_         = "NULL";
        client_port_    = 0;
        dispatch_port_  = 0;
        term_port_      = 0;
        organ_id_       = "NULL";
        install_addr_   = "NULL";
        note_           = "NULL";
    }

    void Update(const CAesInfo& cAes) {
        id_             = cAes.id_           ;
        name_           = cAes.name_         ;
        islock_         = cAes.islock_       ;
        wan_ip_         = cAes.wan_ip_       ;
        lan_ip_         = cAes.lan_ip_       ;
        client_port_    = cAes.client_port_  ;
        dispatch_port_  = cAes.dispatch_port_;
        term_port_      = cAes.term_port_    ;
        organ_id_       = cAes.organ_id_     ;
        install_addr_   = cAes.install_addr_ ;
        note_           = cAes.note_         ;
    }

public:
    QString id_;            /* */
    QString name_;          /* */
    int32_t islock_;        /* */
    QString wan_ip_;        /* */
    QString lan_ip_;        /* */
    int32_t client_port_;   /* */
    int32_t dispatch_port_; /* */
    int32_t term_port_;     /* */
    QString organ_id_;      /* */
    QString install_addr_;  /* */
    QString note_;          /* */
};
