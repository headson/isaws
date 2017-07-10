#ifndef CMAINDIALOG_H
#define CMAINDIALOG_H

#include <QtWidgets/QMainWindow>
#include "ui_cmaindialog.h"

class CMainDialog : public QMainWindow
{
    Q_OBJECT

public:
    CMainDialog(QWidget *parent = 0);
    ~CMainDialog();

private:
    Ui::CMainDialogClass ui;
};

#endif // CMAINDIALOG_H
