#ifndef BASEWINDOW_H
#define BASEWINDOW_H

#include <QDialog>
#include "cmytitlebar.h"

class CMyDialog : public QDialog {
  Q_OBJECT

 public:
  CMyDialog(QWidget *parent = 0);
  ~CMyDialog();

 protected:
  void initTitleBar();
  void paintEvent(QPaintEvent *event);
  void loadStyleSheet(const QString &sSheetName);

  void InitMyTitle(E_TBTN_TYPE eType, QString sTitle, int nSize=12);

 protected slots:
  virtual void onButtonMinClicked();
  virtual void onButtonRestoreClicked();
  virtual void onButtonMaxClicked();
  virtual void onButtonCloseClicked();

 protected:
  CMyTitleBar* m_pBarTitle;
};

#endif // BASEWINDOW_H
