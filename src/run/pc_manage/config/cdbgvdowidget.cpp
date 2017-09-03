#include "cdbgvdowidget.h"
#include "ui_cdbgvdowidget.h"

CDbgVdoWidget::CDbgVdoWidget(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::CDbgVdoWidget) {
  ui->setupUi(this);
  // ui->dev_list_->setContentsMargins(0, 0, 0, 0);

  // style sheet
  //QFile file(":/resource/devlistwidget.css");
  //if (file.open(QFile::ReadOnly)) {
  //  this->setStyleSheet(QLatin1String(file.readAll()));
  //  file.close();
  //}

  vdo_play_ = new CVideoPlayer();
  if (vdo_play_) {
    vdo_play_->HideAllOther();

    QLayout* pLayout = new QHBoxLayout();
    pLayout->addWidget(vdo_play_);
    pLayout->setContentsMargins(1, 1, 1, 1);
    ui->pVdoGBox->setLayout(pLayout);
  }
}

CDbgVdoWidget::~CDbgVdoWidget() {
  if (vdo_play_) {
    delete vdo_play_;
    vdo_play_ = NULL;
  }

  delete ui;
}

void CDbgVdoWidget::Initinal(QString id) {
  CDevInfo dev;
  CDeviceDetect::Instance()->GetDev(dev, id);

  QString surl = "http://";
  surl += dev.ip_.c_str();
  surl += ":";
  surl += QString::number(dev.port_);
  surl += "/httpflv?chn=video";
  surl += QString::number(2);
  //surl += QString::number(0);
  if (vdo_play_) {
    vdo_play_->PlayUrl(surl);
  }

  show();
  exec();
}
