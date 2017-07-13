/*****************************************************************************
* Copyright (c) 2011,ChengDu ThinkWatch Company
* All rights reserved.
*-----------------------------------------------------------------------------
* filename: cdevinfo.h
* describe:
* author  : Sober.Peng
* datetime: 2016-7-26
******************************************************************************/
#include "cdevinfo.h"

QPixmap CDevInfo::GetPhoto(QString& photo_, int nWidth, int nHeight) {
  QPixmap cPixmap;  // ’’∆¨
  if (!photo_.isEmpty()) {
    QString sPhoto = photo_;
    sPhoto.replace('.', '+');
    sPhoto.replace('_', '/');
    cPixmap.loadFromData(QByteArray::fromBase64(sPhoto.toLocal8Bit()));

    if (!cPixmap.isNull()) {
      QMatrix matrix;
      if (false) {
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

QString CDevInfo::GetDefVdoChn() {
  return "video_0";
}
