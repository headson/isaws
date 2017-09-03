#include "civacfgwidget.h"
#include "ui_civacfgwidget.h"

CIvaCfgWidget::CIvaCfgWidget(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::CIvaCfgWidget) {
  ui->setupUi(this);
  // ui->dev_list_->setContentsMargins(0, 0, 0, 0);

  // style sheet
  //QFile file(":/resource/devlistwidget.css");
  //if (file.open(QFile::ReadOnly)) {
  //  this->setStyleSheet(QLatin1String(file.readAll()));
  //  file.close();
  //}
}

CIvaCfgWidget::~CIvaCfgWidget() {
  delete ui;
}

void CIvaCfgWidget::Initinal(QString id) {
  dev_id_ = id;

  CDevInfo dev;
  CDeviceDetect::Instance()->GetDev(dev, dev_id_);
}
