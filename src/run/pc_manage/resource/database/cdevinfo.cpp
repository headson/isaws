/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* filename: cdevinfo.h
* describe:
* author  : Sober.Peng
* datetime: 2016-7-26
******************************************************************************/
#include "stdafx.h"
#include "cdevinfo.h"

QPixmap CDevInfo::GetPhoto(QString& photo_, uint32_t nWidth, uint32_t nHeight)
{
    QPixmap cPixmap;  // ’’∆¨
    if (!photo_.isEmpty())
    {
        QString sPhoto = photo_;
        sPhoto.replace('.', '+'); sPhoto.replace('_', '/');
        cPixmap.loadFromData(QByteArray::fromBase64(sPhoto.toAscii()));

        if (!cPixmap.isNull())
        {
            QMatrix matrix; 
            if (GetType() == TYPE_SW1 || GetType() == TYPE_SW2) {
                matrix.rotate(270);
                cPixmap = cPixmap.scaled(nHeight, nWidth,
                    Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            } else {
                matrix.rotate(0);
                cPixmap = cPixmap.scaled(nWidth, nHeight, 
                    Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
            }
            cPixmap = cPixmap.transformed(matrix);
        }
    }
    return cPixmap;
}

QString CDevInfo::GetDefVdoChn()
{
    int64_t nIva = VString::get(iva_status_.toStdString()).to_num<int64_t>();
    if (VBit::btst((uint8_t*)&nIva, 26)) {
        return "video_2";
    }
    return "video_0";
}
