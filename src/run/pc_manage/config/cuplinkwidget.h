/************************************************************************
* Author      : SoberPeng 2017-09-02
* Description :
************************************************************************/
#ifndef PC_MANAGE_CUPLINKWIDGET_H
#define PC_MANAGE_CUPLINKWIDGET_H

#include "appinc.h"

namespace Ui {
class CUplinkWidget;
};

class CPlayFrame;
class CUplinkWidget : public QWidget {
  Q_OBJECT

 public:
  CUplinkWidget(QWidget *parent = NULL);
  ~CUplinkWidget();

  void Initinal(QString id);

 private:
  Ui::CUplinkWidget *ui;
  QString            dev_id_;
};

#endif  // PC_MANAGE_CUPLINKWIDGET_H
