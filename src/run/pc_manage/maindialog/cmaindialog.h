#ifndef CMAINDIALOG_H
#define CMAINDIALOG_H

#include "appinc.h"

#include "ui_cmaindialog.h"

class CMainDialog : public CMyDialog
{
    Q_OBJECT

public:
    CMainDialog(QDialog *parent = 0);
    ~CMainDialog();

    bool CheckLogin();

private:
    Ui::CMainDialog ui;
};

#endif // CMAINDIALOG_H
