#include "cvdocfgwidget.h"
#include "ui_cvdocfgwidget.h"
#include "vzbase/helper/stdafx.h"
#include "vzbase/base/vmessage.h"
#include "curlclient/chttpclient.h"

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
  connect(ui->pGetOsdBtn, SIGNAL(clicked()), SLOT(OnGetOsdSlot()));
  connect(ui->pSetOsdBtn, SIGNAL(clicked()), SLOT(OnSetOsdSlot()));
}

CVdoCfgWidget::~CVdoCfgWidget() {
  delete ui;
}

void CVdoCfgWidget::Initinal(QString id) {
  dev_id_ = id;
}

void CVdoCfgWidget::OnGetOsdSlot() {
  CDevInfo dev;
  CDeviceDetect::Instance()->GetDev(dev, dev_id_);

  Json::Value jreq;
  jreq[MSG_CMD] = MSG_GET_ENC_OSD;
  jreq[MSG_ID]  = 1;
  jreq[MSG_BODY] = "";

  Json::FastWriter jw;
  std::string sreq = jw.write(jreq);
  
  osd_cfg_ = "";

  CHttpClient cli;
  cli.Post(dev.GetUrl(), 80, sreq, osd_cfg_);
  LOG(L_INFO) << osd_cfg_;

  Json::Value jresp;
  Json::Reader jread;
  if (!jread.parse(osd_cfg_, jresp)) {
    LOG(L_ERROR) << "parse json failed.";
    osd_cfg_ = "";
    return;
  }
  Json::Value josd = jresp[MSG_BODY];

  int chn = 0, en, x, y;
  // 0
  chn = 0;
  if (josd[chn]["handle"].asInt() == chn) {
    en = josd[chn]["enable"].asInt();
    ui->pDateTimeCBox->setChecked((en > 0) ? true : false);

    x = josd[chn]["x"].asInt();
    ui->pDateTimeX->setText(QString::number(x));

    y = josd[chn]["y"].asInt();
    ui->pDateTimeY->setText(QString::number(y));
  }

  // 1
  chn = 1;
  if (josd[chn]["handle"].asInt() == chn) {
    en = josd[chn]["enable"].asInt();
    ui->pPCountCBox->setChecked((en > 0) ? true : false);

    x = josd[chn]["x"].asInt();
    ui->pPCountX->setText(QString::number(x));

    y = josd[chn]["y"].asInt();
    ui->pPCountY->setText(QString::number(y));
  }

  // 2
  chn = 2;
  if (josd[chn]["handle"].asInt() == chn) {
    en = josd[chn]["enable"].asInt();
    ui->pIRCoverCBox->setChecked((en > 0) ? true : false);

    x = josd[chn]["x"].asInt();
    ui->pIRCoverX->setText(QString::number(x));

    y = josd[chn]["y"].asInt();
    ui->pIRCoverY->setText(QString::number(y));
  }
}

void CVdoCfgWidget::OnSetOsdSlot() {
  Json::Value jreq;
  Json::Reader jread;
  if (!jread.parse(osd_cfg_, jreq)) {
    LOG(L_ERROR) << "parse json failed.";
    osd_cfg_ = "";
    return;
  }
  jreq[MSG_CMD] = MSG_SET_ENC_OSD;
  Json::Value josd = jreq[MSG_BODY];

  int chn = 0;

  chn = 0;
  josd[chn]["enable"] = (ui->pDateTimeCBox->checkState() == Qt::Checked) ? 1 : 0;
  josd[chn]["x"] = ui->pDateTimeX->text().toInt();
  josd[chn]["y"] = ui->pDateTimeY->text().toInt();

  chn = 1;
  josd[chn]["enable"] = (ui->pPCountCBox->checkState() == Qt::Checked) ? 1 : 0;
  josd[chn]["x"] = ui->pPCountX->text().toInt();
  josd[chn]["y"] = ui->pPCountY->text().toInt();

  chn = 2;
  josd[chn]["enable"] = (ui->pIRCoverCBox->checkState() == Qt::Checked) ? 1 : 0;
  josd[chn]["x"] = ui->pIRCoverX->text().toInt();
  josd[chn]["y"] = ui->pIRCoverY->text().toInt();

  jreq[MSG_BODY] = josd;

  Json::FastWriter jw;
  std::string sreq = jw.write(jreq);

  CDevInfo dev;
  CDeviceDetect::Instance()->GetDev(dev, dev_id_);

  std::string sresp = "";
  CHttpClient cli;
  cli.Post(dev.GetUrl(), 80, sreq, sresp);
  LOG(L_INFO) << sresp;
}
