#include "maindialog/cmaindialog.h"
#include "maindialog/clogindialog.h"

#pragma execution_character_set("utf-8")

CMainDialog::CMainDialog(QDialog *parent)
  : CMyDialog(parent) {
  ui.setupUi(this);

  this->setWindowTitle("iSaw����ϵͳ");
  InitMyTitle(E_MIN_MAX_BTN, "iSaw����ϵͳ", 12);
  this->loadStyleSheet(":/resource/mytitle.css");
}

CMainDialog::~CMainDialog() {

}

bool CMainDialog::CheckLogin()
{
  CLoginDialog cDlg;
  cDlg.InitWindows();
  if (!cDlg.ShowExec()) {
    return -1;
  }
  return 0;
}
