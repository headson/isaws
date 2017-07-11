#ifndef CMAINDIALOG_H
#define CMAINDIALOG_H

#include <QDialog>
#include "common/cmydialog.h"

#include "ui_cmaindialog.h"

class CMainDialog : public CMyDialog
{
    Q_OBJECT

public:
    CMainDialog(QDialog *parent = 0);
    ~CMainDialog();

    bool CheckLogin();

private:
    Ui::CMainDialogClass ui;
};

#endif // CMAINDIALOG_H
