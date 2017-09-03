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

  bool        bDebug;             // 调试

  QString     sPath;              // 应用地址
  QString     sUsername;          // 登陆用户
  QString     sPassword;          // 用户密码

  bool        b_conn_remote;      // 连接远程数据库
  QString     s_remote_addr;      // 远程数据库地址
  quint32     n_remote_port;      // 远程数据库端口
} TAG_APPLET;
extern TAG_APPLET   g_app;

#endif  // _APPINC_H
