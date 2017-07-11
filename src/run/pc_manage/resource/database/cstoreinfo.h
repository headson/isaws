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

class CStoreInfo
{
public:
public:
    CStoreInfo() {
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

    void Update(const CStoreInfo& cStore) {
        id_                 = cStore.id_              ;
        name_               = cStore.name_            ;
        wan_ip_             = cStore.wan_ip_          ;
        lan_ip_             = cStore.lan_ip_          ;
        vdo_port_           = cStore.vdo_port_        ;
        msg_port_           = cStore.msg_port_        ;
        owner_ship_         = cStore.owner_ship_      ;
        link_type_          = cStore.link_type_       ;
        heart_cycle_        = cStore.heart_cycle_     ;
        max_input_cnt_      = cStore.max_input_cnt_   ;
        max_output_cnt_     = cStore.max_output_cnt_  ;
        resvd_output_cnt_   = cStore.resvd_output_cnt_;
        organ_id_           = cStore.organ_id_        ;
        install_addr_       = cStore.install_addr_    ;
        note_               = cStore.note_            ;
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
