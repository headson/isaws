/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* filename: cdevinfo.h
* describe:
* author  : Sober.Peng
* datetime: 2016-7-26
******************************************************************************/
#pragma once
#include <stdint.h>
#include <QString>

class CEvtInfo
{
public:
    CEvtInfo() {
        id_         = -1;   // 
        time_point_ = "";   // 
        time_end_    = "";   // 
        event_      = -1;   // 
        device_id_  = "";   // 
        ident_user_ = "";   // 
        number_     = 0;    // 
        level_      = 0;    // 
        user_id_    = "";   // 
        process_    = 0;    // 
        photo_      = "";   // 
        note_       = "";   // 

        event_name_     = "";
        device_name_    = "";
        device_type_    = 0;
        ident_user_gbk_ = "";
        process_name_   = "";

        nInRelVal  = nInAbsVal  = 0;
        nOutRelVal = nOutAbsVal = 0;
    }
    CEvtInfo(const CEvtInfo& cEvt) {
        id_         = cEvt.id_        ;
        time_point_ = cEvt.time_point_;
        time_end_    = cEvt.time_end_   ;
        event_      = cEvt.event_     ;
        device_id_  = cEvt.device_id_ ;
        ident_user_ = cEvt.ident_user_;
        number_     = cEvt.number_    ;
        level_      = cEvt.level_     ;
        user_id_    = cEvt.user_id_   ;
        process_    = cEvt.process_   ;
        photo_      = cEvt.photo_     ;
        note_       = cEvt.note_      ;

        event_name_     = cEvt.event_name_;
        device_name_    = cEvt.device_name_;
        device_type_    = cEvt.device_type_;
        ident_user_gbk_ = cEvt.ident_user_gbk_;
        process_name_   = cEvt.process_name_;

        nInRelVal   = cEvt.nInRelVal;
        nInAbsVal   = cEvt.nInAbsVal;
        nOutRelVal  = cEvt.nOutRelVal;
        nOutAbsVal  = cEvt.nOutAbsVal;
    }

    CEvtInfo(const VCmd2004Req& cReq) {
        id_         = -1;   // 
        time_point_ = cReq.sTmBng.c_str();   // 
        time_end_   = cReq.sTmEnd.c_str();   // 
        event_      = cReq.nType;   // 
        device_id_  = cReq.sDevId.c_str();   // 
        ident_user_ = cReq.sTitle.c_str();   // 
        number_     = cReq.nNumber;    // 
        level_      = cReq.nLevel;    // 
        user_id_    = "";   // 
        process_    = 0;    // 
        photo_      = cReq.sPhoto.c_str();   // 
        note_       = "";   // 

        event_name_     = "";
        device_name_    = "";
        device_type_    = 0;
        ident_user_gbk_ = "";
        process_name_   = "";

        nInRelVal   = cReq.nInRelVal;
        nInAbsVal   = cReq.nInAbsVal;
        nOutRelVal  = cReq.nOutRelVal;
        nOutAbsVal  = cReq.nOutAbsVal;
    }

public:
    int32_t  id_;        // 
    QString  time_point_;// 
    QString  time_end_;   // 
    int32_t  event_;     // 
    QString  device_id_; // 
    QString  ident_user_;// 
    uint32_t number_;    // 
    int32_t  level_;     // 
    QString  user_id_;   // 
    int32_t  process_;   // 
    QString  photo_;     // 
    QString  note_;      // 

    QString  event_name_;
    QString  device_name_;
    int32_t  device_type_;
    QString  ident_user_gbk_;
    QString  process_name_;

    int32_t     nInRelVal;  // 入绝对值
    int32_t     nInAbsVal;  // 入相对值
    int32_t     nOutRelVal; // 出绝对值
    int32_t     nOutAbsVal; // 出相对值
};
