#include "cvdocfgwidget.h"
#include "ui_cvdocfgwidget.h"

CVdoCfgWidget::CVdoCfgWidget(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::CVdoCfgWidget) {
  ui->setupUi(this);
  // ui->dev_list_->setContentsMargins(0, 0, 0, 0);

  // style sheet
  //QFile file(":/resource/devlistwidget.css");
  //if (file.open(QFile::ReadOnly)) {
  //  this->setStyleSheet(QLatin1String(file.readAll()));
  //  file.close();
  //}
}

CVdoCfgWidget::~CVdoCfgWidget() {
  delete ui;
}

void CVdoCfgWidget::Initinal(QString id) {
  dev_id_ = id;

  CDevInfo dev;
  CDeviceDetect::Instance()->GetDev(dev, dev_id_);
}
