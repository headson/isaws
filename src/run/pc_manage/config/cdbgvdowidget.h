/************************************************************************
* Author      : SoberPeng 2017-09-02
* Description :
************************************************************************/
#ifndef PC_MANAGE_CDBGVDOWIDGET_H
#define PC_MANAGE_CDBGVDOWIDGET_H

#include "appinc.h"

#include "preview/cvideoplayer.h"

namespace Ui {
class CDbgVdoWidget;
};

class CPlayFrame;
class CDbgVdoWidget : public QDialog {
  Q_OBJECT

 public:
  CDbgVdoWidget(QWidget *parent = NULL);
  ~CDbgVdoWidget();

  void Initinal(QString id);

 private:
  Ui::CDbgVdoWidget *ui;

  CVideoPlayer      *vdo_play_;
};

#endif  // PC_MANAGE_CDBGVDOWIDGET_H

