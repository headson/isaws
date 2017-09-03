/************************************************************************
* Author      : SoberPeng 2017-09-02
* Description :
************************************************************************/
#ifndef PC_MANAGE_CDEVCFGWIDGET_H
#define PC_MANAGE_CDEVCFGWIDGET_H

#include "appinc.h"

#include "civacfgwidget.h"
#include "cvdocfgwidget.h"
#include "cuplinkwidget.h"

namespace Ui {
class CDevCfgWidget;
};

class CPlayFrame;
class CDevCfgWidget : public QDialog {
  Q_OBJECT

 public:
  CDevCfgWidget(QWidget *parent = NULL);
  ~CDevCfgWidget();

  void Initinal(QString id);

 protected slots:
  void OnItemClickSlot(QTreeWidgetItem*, int);

  void OnGetDevCfgSlot();
  void OnSetDevCfgSlot();

 private:
  Ui::CDevCfgWidget *ui;
  QString            dev_id_;

 private:
   CIvaCfgWidget    *iva_cfg_;
   CVdoCfgWidget    *vdo_cfg_;
   CUplinkWidget    *uplink_cfg_;
};

#endif  // PC_MANAGE_CDEVCFGWIDGET_H
