#include "cuplinkwidget.h"
#include "ui_cuplinkwidget.h"

CUplinkWidget::CUplinkWidget(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::CUplinkWidget) {
  ui->setupUi(this);
  // ui->dev_list_->setContentsMargins(0, 0, 0, 0);

  // style sheet
  //QFile file(":/resource/devlistwidget.css");
  //if (file.open(QFile::ReadOnly)) {
  //  this->setStyleSheet(QLatin1String(file.readAll()));
  //  file.close();
  //}
}

CUplinkWidget::~CUplinkWidget() {
  delete ui;
}

void CUplinkWidget::Initinal(QString id) {
  dev_id_ = id;

  CDevInfo dev;
  CDeviceDetect::Instance()->GetDev(dev, dev_id_);
}
