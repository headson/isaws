#ifndef CMAINDIALOG_H
#define CMAINDIALOG_H

#include "appinc.h"

#include "ui_cmaindialog.h"
#include "preview/clivewidget.h"
#include "devtree/cdevlistwidget.h"

class CMainDialog : public CMyDialog {
  Q_OBJECT

 public:
  CMainDialog(QDialog *parent = 0);
  ~CMainDialog();

  bool InitWindow();

  bool CheckLogin();

 private:
  Ui::CMainDialog ui;

 private:
   CDevListWidget *dev_list_;
   CLiveWidget    *live_widget_;
};

#endif // CMAINDIALOG_H
