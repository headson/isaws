
#include "clogindialog.h"
#include "ui_clogindialog.h"

CLoginDialog::CLoginDialog(QWidget *parent)
  : CMyDialog(parent)
  , ui(new Ui::CLoginDialog) {
  ui->setupUi(this);

  ui->pFgtPwdBtn->setVisible(false);
  ui->pRegUserBtn->setVisible(false);

  ui->pRmbPwdCBox->setVisible(false);
  ui->pAutoLgnCBox->setVisible(false);

  ui->pPwdEdit->setEchoMode(QLineEdit::Password);

  connect(ui->pLoginButton, SIGNAL(clicked()), SLOT(OnLoginSlot()));
}

CLoginDialog::~CLoginDialog() {
  delete ui;
}

void CLoginDialog::InitWindows() {
  QLabel* pBack = new QLabel(this);
  QPixmap cPixmap;
  cPixmap.load(":/resource/logindialog/backgroud.png");
  pBack->setPixmap(cPixmap);
  pBack->move(0, 0);

  // 暗注释
  ui->pNameCBox->setEditable(true);
  QLineEdit* pNameEdit = ui->pNameCBox->lineEdit();
  pNameEdit->setPlaceholderText(tr("用户名"));
  ui->pPwdEdit->setPlaceholderText(tr("用户密码"));

  pNameEdit->setText(g_app.sUsername);
  if (g_app.bDebug) {
    ui->pPwdEdit->setText("12345678");
  }

  InitMyTitle(E_MIN_BTN, "");
  this->loadStyleSheet(":/resource/logindialog.css");
}

void CLoginDialog::OnLoginSlot() {
  g_app.sUsername  = ui->pNameCBox->currentText();
  g_app.sPassword  = ui->pPwdEdit->text();

  bool bRet = true;
  if(bRet) {
    accept();
  } else {
  }
}

int CLoginDialog::ShowExec() {
  show();
  return exec();
}

