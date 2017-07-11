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

class CDispInfo
{
public:
    CDispInfo() {
        id_                 = "";     
        name_               = "NULL"; 
        wan_ip_             = "NULL"; 
        lan_ip_             = "NULL"; 
        vdo_port_           = -1;
        msg_port_           = -1;
        owner_ship_         = "NULL";
        link_type_          = "NULL";
        heart_cycle_        = -1;
        max_input_cnt_      = -1;
        max_output_cnt_     = -1;
        resvd_output_cnt_   = -1;
        organ_id_           = "NULL";
        install_addr_       = "NULL";
        note_               = "NULL";
    }

    void Update(const CDispInfo& cDisp) {
        id_                 = cDisp.id_              ;
        name_               = cDisp.name_            ;
        wan_ip_             = cDisp.wan_ip_          ;
        lan_ip_             = cDisp.lan_ip_          ;
        vdo_port_           = cDisp.vdo_port_        ;
        msg_port_           = cDisp.msg_port_        ;
        owner_ship_         = cDisp.owner_ship_      ;
        link_type_          = cDisp.link_type_       ;
        heart_cycle_        = cDisp.heart_cycle_     ;
        max_input_cnt_      = cDisp.max_input_cnt_   ;
        max_output_cnt_     = cDisp.max_output_cnt_  ;
        resvd_output_cnt_   = cDisp.resvd_output_cnt_;
        organ_id_           = cDisp.organ_id_        ;
        install_addr_       = cDisp.install_addr_    ;
        note_               = cDisp.note_            ;
    }

public:
    QString id_;                /* */
    QString name_;              /* */
    QString wan_ip_;            /* */
    QString lan_ip_;            /* */
    int32_t vdo_port_;          /* */
    int32_t msg_port_;          /* */
    QString owner_ship_;        /* */
    QString link_type_;         /* */
    int32_t heart_cycle_;       /* */
    int32_t max_input_cnt_;     /* */
    int32_t max_output_cnt_;    /* */
    int32_t resvd_output_cnt_;  /* */
    QString organ_id_;          /* */
    QString install_addr_;      /* */
    QString note_;              /* */
};
