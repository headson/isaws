#include "maindialog/cmaindialog.h"
#include "maindialog/clogindialog.h"

#include "vzbase/helper/stdafx.h"

#pragma execution_character_set("utf-8")

CMainDialog::CMainDialog(QDialog *parent)
  : CMyDialog(parent) {
  ui.setupUi(this);

  this->setWindowTitle("iSaw管理系统");
  InitMyTitle(E_MIN_MAX_BTN, "iSaw管理系统", 12);
  this->loadStyleSheet(":/resource/mytitle.css");

  InitWindow();
}

CMainDialog::~CMainDialog() {

}

bool CMainDialog::InitWindow() {
  resize(1000, 700);

  dev_list_ = new CDevListWidget(ui.dev_list_grp_);
  if (NULL == dev_list_) {
    LOG(L_ERROR) << "create device list widget failed.";
    return false;
  } else {
    QGridLayout *layout_ = new QGridLayout();
    layout_->addWidget(dev_list_);
    layout_->setSpacing(3);
    layout_->setContentsMargins(0,0,0,0);
    ui.dev_list_grp_->setLayout(layout_);
    ui.dev_list_grp_->setMaximumWidth(196);
  }

  live_widget_ = new CLiveWidget();
  ui.center_widget_->addWidget(live_widget_);
  ui.center_widget_->setCurrentWidget(live_widget_);
  return true;
}

bool CMainDialog::CheckLogin() {
  CLoginDialog cDlg;
  cDlg.InitWindows();
  if (!cDlg.ShowExec()) {
    return -1;
  }
  return 0;
}
