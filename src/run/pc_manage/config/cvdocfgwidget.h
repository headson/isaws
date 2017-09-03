/************************************************************************
* Author      : SoberPeng 2017-09-02
* Description :
************************************************************************/
#ifndef PC_MANAGE_CVDOCFGWIDGET_H
#define PC_MANAGE_CVDOCFGWIDGET_H

#include "appinc.h"

namespace Ui {
class CVdoCfgWidget;
};

class CPlayFrame;
class CVdoCfgWidget : public QWidget {
  Q_OBJECT

 public:
  CVdoCfgWidget(QWidget *parent = NULL);
  ~CVdoCfgWidget();

  void Initinal(QString id);
  
 private:
  Ui::CVdoCfgWidget *ui;
  QString            dev_id_;
};

#endif  // PC_MANAGE_CVDOCFGWIDGET_H
