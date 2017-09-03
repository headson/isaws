/************************************************************************
* Author      : SoberPeng 2017-09-02
* Description :
************************************************************************/
#ifndef PC_MANAGE_CIVACFGWIDGET_H
#define PC_MANAGE_CIVACFGWIDGET_H

#include "appinc.h"

namespace Ui {
class CIvaCfgWidget;
};

class CPlayFrame;
class CIvaCfgWidget : public QWidget {
  Q_OBJECT

 public:
  CIvaCfgWidget(QWidget *parent = NULL);
  ~CIvaCfgWidget();

  void Initinal(QString id);

 private:
  Ui::CIvaCfgWidget *ui;
  QString            dev_id_;
};

#endif  // PC_MANAGE_CIVACFGWIDGET_H
