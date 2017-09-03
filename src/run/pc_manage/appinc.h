/************************************************************************/
/* Author      : SoberPeng 2017-07-12
/* Description :
/************************************************************************/
#ifndef _APPINC_H
#define _APPINC_H

#include <QFile>
#include <QDebug>
#include <QDateTime>

#include <QPixmap>
#include <QTreeWidgetItem>

#include <QtWidgets/QDialog>
#include <QtWidgets/QWidget>
#include <QtWidgets/QApplication>

#include "vzbase/base/basictypes.h"

#include "common/cmydialog.h"
#include "database/cdevinfo.h"

#include "devtree/cdevicedetect.h"

#pragma execution_character_set("utf-8")

typedef struct _TAG_APPLET {
  _TAG_APPLET() {
  }

  bool        bDebug;             // ����

  QString     sPath;              // Ӧ�õ�ַ
  QString     sUsername;          // ��½�û�
  QString     sPassword;          // �û�����

  bool        b_conn_remote;      // ����Զ�����ݿ�
  QString     s_remote_addr;      // Զ�����ݿ��ַ
  quint32     n_remote_port;      // Զ�����ݿ�˿�
} TAG_APPLET;
extern TAG_APPLET   g_app;

#endif  // _APPINC_H
