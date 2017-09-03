#include "cdevcfgwidget.h"
#include "ui_cdevcfgwidget.h"

#include "vzbase/base/vmessage.h"

CDevCfgWidget::CDevCfgWidget(QWidget *parent)
  : QDialog(parent)
  , ui(new Ui::CDevCfgWidget)
  , iva_cfg_(NULL)
  , vdo_cfg_(NULL)
  , uplink_cfg_(NULL) {
  ui->setupUi(this);
  // ui->dev_list_->setContentsMargins(0, 0, 0, 0);

  // style sheet
  //QFile file(":/resource/devlistwidget.css");
  //if (file.open(QFile::ReadOnly)) {
  //  this->setStyleSheet(QLatin1String(file.readAll()));
  //  file.close();
  //}

  ui->pCfgTreeWidget->setDragEnabled(true);
  ui->pCfgTreeWidget->setDropIndicatorShown(true);
  ui->pCfgTreeWidget->viewport()->installEventFilter(this);
  ui->pCfgTreeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->pCfgTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  //ui->pDevTreeWidget->header()->setResizeMode(QHeaderView::ResizeToContents);

  // 监听
  connect(ui->pCfgTreeWidget, SIGNAL(itemPressed(QTreeWidgetItem*, int)), SLOT(OnItemClickSlot(QTreeWidgetItem*, int)));

  connect(ui->pGetCfgButton, SIGNAL(clicked()), SLOT(OnGetDevCfgSlot()));
  connect(ui->pSetCfgButton, SIGNAL(clicked()), SLOT(OnSetDevCfgSlot()));
}

CDevCfgWidget::~CDevCfgWidget() {
  delete ui;
}

void CDevCfgWidget::Initinal(QString id) {
  dev_id_ = id;
  OnGetDevCfgSlot();
  ui->pNameLabel->setText(dev_id_);

  iva_cfg_ = new CIvaCfgWidget(ui->pConfigWidget);
  ui->pConfigWidget->addWidget(iva_cfg_);

  vdo_cfg_ = new CVdoCfgWidget(ui->pConfigWidget);
  ui->pConfigWidget->addWidget(vdo_cfg_);

  uplink_cfg_ = new CUplinkWidget(ui->pConfigWidget);
  ui->pConfigWidget->addWidget(uplink_cfg_);

  ui->pConfigWidget->setCurrentWidget(ui->pDevInfoPage);

  show();
  exec();
}

void CDevCfgWidget::OnItemClickSlot(QTreeWidgetItem *item, int index) {
  if (item->text(0) == tr("设备信息")) {
    ui->pConfigWidget->setCurrentWidget(ui->pDevInfoPage);
  } else if (item->text(0) == tr("算法参数")) {
    ui->pConfigWidget->setCurrentWidget(iva_cfg_);
  } else if (item->text(0) == tr("级联配置")) {
    ui->pConfigWidget->setCurrentWidget(uplink_cfg_);
  } else if (item->text(0) == tr("视频参数")) {
    ui->pConfigWidget->setCurrentWidget(vdo_cfg_);
  }
}

/*
{
"body" : {
"alg_version" : "",
"dev_uuid" : "adsasdas"
"dev_name" : "PC_001",
"dev_type" : 100100,
"hw_version" : "1.0.0.1001707310",
"ins_addr" : "",
"net" : {
"dhcp_en" : 0,
"dns_addr" : "192.168.1.1",
"gateway" : "192.168.1.1",
"http_port" : 80,
"ip_addr" : "192.168.1.100",
"netmask" : "255.255.255.0",
"phy_mac" : "",
"rtsp_port" : 554,
"wifi_en" : 0
},
"sw_version" : ""
},
"cmd" : "get_devinfo",
"state" : 0
}
*/
void CDevCfgWidget::OnGetDevCfgSlot() {
  CDevInfo dev;
  CDeviceDetect::Instance()->GetDev(dev, dev_id_);

  ui->pDevUUIDEdit->setText(dev.json_[MSG_BODY]["dev_uuid"].asString().c_str());
  ui->pDevNameEdit->setText(dev.json_[MSG_BODY]["dev_name"].asString().c_str());
  ui->pDevTypeEdit->setText(QString::number(dev.json_[MSG_BODY]["dev_type"].asInt()));
  ui->pSoftVerEdit->setText(dev.json_[MSG_BODY]["sw_version"].asString().c_str());
  ui->pHardVerEdit->setText(dev.json_[MSG_BODY]["hw_version"].asString().c_str());
  ui->pAlgVerEdit->setText(dev.json_[MSG_BODY]["alg_version"].asString().c_str());

  ui->pDHCPCBox->setChecked(false);
  if (dev.json_[MSG_BODY]["net"]["dhcp_en"].asInt() == 1) {
    ui->pDHCPCBox->setChecked(true);
  }
  ui->pIpAddrEdit->setText(dev.json_[MSG_BODY]["net"]["ip_addr"].asString().c_str());
  ui->pNetmaskEdit->setText(dev.json_[MSG_BODY]["net"]["netmask"].asString().c_str());
  ui->pGatewayEdit->setText(dev.json_[MSG_BODY]["net"]["gateway"].asString().c_str());
  ui->pDnsEdit->setText(dev.json_[MSG_BODY]["net"]["dns_addr"].asString().c_str());
  ui->pHttpPortEdit->setText(QString::number(dev.json_[MSG_BODY]["net"]["http_port"].asInt()));
}

void CDevCfgWidget::OnSetDevCfgSlot() {
  CDevInfo dev;
  CDeviceDetect::Instance()->GetDev(dev, dev_id_);

  dev.json_[MSG_CMD] = MSG_SET_DEVINFO;
  dev.json_[MSG_BODY]["dev_name"] = ui->pDevNameEdit->text().toLocal8Bit().data();
  dev.json_[MSG_BODY]["net"]["dhcp_en"] = ui->pDHCPCBox->isChecked() ? 1 : 0;
  dev.json_[MSG_BODY]["net"]["ip_addr"] = ui->pIpAddrEdit->text().toLocal8Bit().data();
  dev.json_[MSG_BODY]["net"]["netmask"] = ui->pNetmaskEdit->text().toLocal8Bit().data();
  dev.json_[MSG_BODY]["net"]["gateway"] = ui->pGatewayEdit->text().toLocal8Bit().data();
  dev.json_[MSG_BODY]["net"]["dns_addr"] = ui->pDnsEdit->text().toLocal8Bit().data();
  dev.json_[MSG_BODY]["net"]["http_port"] = ui->pHttpPortEdit->text().toInt();

  CDeviceDetect::Instance()->SetDev(dev.json_);
}
