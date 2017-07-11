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
#include <QObject>
#include <QString>

#define CHN_0       "监控"
#define CHN_1       "人像"
#define CHN_2       "环境"
#define CHN_DBG     "调试"

class CDevInfo
{
public:
    CDevInfo() {
        id_             = "";
        check_code_     = "NULL";
        device_type_id_ = -1;
        name_           = "NULL";
        module_         = "0";
        islock_         = -1;
        version_        = -1;
        hardware_       = -1;
        is_online_      = -1;
        iva_status_     = "NULL";
        ip_             = "NULL";
        msg_port_       = -1;
        vdo_port_       = -1;
        ado_port_       = -1;
        web_port_       = -1;
        update_port_    = -1;
        access_ip_      = "NULL";
        store_id_       = "NULL";
        dispatch_id_    = "NULL";
        organ_          = "NULL";
        heart_cycle_    = -1;
        install_addr_   = "NULL";
        note_           = "NULL";
        photo_          = "NULL";
    
        mac_            = "";
        reg_            = false;
        link_time_      = 0;
    }

    CDevInfo(const CDevInfo& cDev) {
        id_             = cDev.id_            ;
        check_code_     = cDev.check_code_    ;
        device_type_id_ = cDev.device_type_id_;
        name_           = cDev.name_          ;
        module_         = cDev.module_        ;
        islock_         = cDev.islock_        ;
        version_        = cDev.version_       ;
        hardware_       = cDev.hardware_      ;
        is_online_      = cDev.is_online_     ;
        iva_status_     = cDev.iva_status_    ;
        ip_             = cDev.ip_            ;
        msg_port_       = cDev.msg_port_      ;
        vdo_port_       = cDev.vdo_port_      ;
        ado_port_       = cDev.ado_port_      ;
        web_port_       = cDev.web_port_      ;
        update_port_    = cDev.update_port_   ;
        access_ip_      = cDev.access_ip_     ;
        store_id_       = cDev.store_id_      ;
        dispatch_id_    = cDev.dispatch_id_   ;
        organ_          = cDev.organ_         ;
        heart_cycle_    = cDev.heart_cycle_   ;
        install_addr_   = cDev.install_addr_  ;
        note_           = cDev.note_          ;
        photo_          = cDev.photo_         ;

        mac_            = cDev.mac_;
        reg_            = cDev.reg_;
        link_time_      = cDev.link_time_;
    }

    CDevInfo operator=(const CDevInfo& cDev) {
        id_             = cDev.id_            ;
        check_code_     = cDev.check_code_    ;
        device_type_id_ = cDev.device_type_id_;
        name_           = cDev.name_          ;
        module_         = cDev.module_        ;
        islock_         = cDev.islock_        ;
        version_        = cDev.version_       ;
        hardware_       = cDev.hardware_      ;
        is_online_      = cDev.is_online_     ;
        iva_status_     = cDev.iva_status_    ;
        ip_             = cDev.ip_            ;
        msg_port_       = cDev.msg_port_      ;
        vdo_port_       = cDev.vdo_port_      ;
        ado_port_       = cDev.ado_port_      ;
        web_port_       = cDev.web_port_      ;
        update_port_    = cDev.update_port_   ;
        access_ip_      = cDev.access_ip_     ;
        store_id_       = cDev.store_id_      ;
        dispatch_id_    = cDev.dispatch_id_   ;
        organ_          = cDev.organ_         ;
        heart_cycle_    = cDev.heart_cycle_   ;
        install_addr_   = cDev.install_addr_  ;
        note_           = cDev.note_          ;
        photo_          = cDev.photo_         ;

        mac_            = cDev.mac_;
        reg_            = cDev.reg_;
        link_time_      = cDev.link_time_;
        return *this;
    }

    int32_t GetType() {
        return device_type_id_ / 100;
    }
    QString GetDefVdoChn();
    QPixmap GetPhoto(QString& photo_, uint32_t nWidth, uint32_t nHeight);

public:
    QString id_;                // 设备ID
    QString check_code_;        // 网络获取-设备验证码
    int32_t device_type_id_;    // 设备类型
    QString name_;              // 设备名
    QString module_;            // 
    int32_t islock_;            // 
    int32_t version_;           // 软件版本
    int32_t hardware_;          // 
    int32_t timezone_;          // 
    int32_t is_online_;         //
    QString iva_status_;        // 
    QString ip_;                //
    int32_t msg_port_;          // 
    int32_t vdo_port_;          // 
    int32_t ado_port_;          // 
    int32_t web_port_;          // 
    int32_t update_port_;       //
    QString access_ip_;         // 
    QString store_id_;          // 
    QString dispatch_id_;       //
    QString organ_;             //    
    int32_t heart_cycle_;       //
    QString install_addr_;      // 
    QString note_;              //
    QString photo_;             // 

    QString mac_;               // 
    bool    reg_;               // 
    int32_t link_time_;         // 
};
