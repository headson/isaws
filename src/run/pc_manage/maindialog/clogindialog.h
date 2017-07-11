#ifndef CLOGINDIALOG_H
#define CLOGINDIALOG_H

#include "appinc.h"

namespace Ui {
class CLoginDialog;
}

class CLoginDialog : public CMyDialog {
  Q_OBJECT

 public:
  explicit CLoginDialog(QWidget *parent = 0);
  ~CLoginDialog();

  void InitWindows();
  int  ShowExec();

 protected slots:
  void OnLoginSlot();

 private:
  Ui::CLoginDialog *ui;
};

#endif // CLOGINDIALOG_H
