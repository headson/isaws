#include "cdevlistwidget.h"
#include "ui_cdevlistwidget.h"

CDevListWidget::CDevListWidget(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::CDevListWidget) {
  ui->setupUi(this);
  ui->dev_list_->setContentsMargins(0, 0, 0, 0);

  // style sheet
  QFile file(":/resource/devlistwidget.css");
  if (file.open(QFile::ReadOnly)) {
    this->setStyleSheet(QLatin1String(file.readAll()));
    file.close();
  }

  // 监听
  connect(ui->find_button_, SIGNAL(clicked()), SLOT(OnDevListSlot()));
  connect(CDeviceDetect::Instance(), SIGNAL(UpdateDevice()), SLOT(OnDevListSlot()));
}

CDevListWidget::~CDevListWidget() {
  delete ui;
}

int CDevListWidget::Initinal() {
  OnDevListSlot();
  return 0;
}

void CDevListWidget::OnDetectSlot() {
  CDeviceDetect::Instance()->Detect();
}

void CDevListWidget::OnDevListSlot() {
  QString sFilter = "";
#if 0
  // 计算显示量
  ui->pDevListWidget->clear();
  m_nPageSize      = ui->pDevListWidget->height() / DEV_ITEM_H;               // 设备ITEM数
  uint32_t DITEM_H = (ui->pDevListWidget->height()-m_nPageSize) / m_nPageSize;// 设备ITEM高

  // 分页
  int32_t nDevCnt = DATABASE()->SelectTableDeviceCount(sFilter);
  if (nDevCnt <= 0) {
    return;
  }
  m_nPageCount = nDevCnt / m_nPageSize + (((nDevCnt % m_nPageSize) > 0) ? 1 : 0);

  // 初始化页CBox
  ui->pCurPageCBox->clear();
  for (int32_t i = 0; i < m_nPageCount; i++) {
    ui->pCurPageCBox->addItem(QString::number(i + 1));
  }
  ui->pCurPageCBox->setCurrentIndex(m_nPageIndex);
  sFilter += " ORDER BY DEVICE_TYPE_ID ASC LIMIT "
             + QString::number(m_nPageIndex*m_nPageSize) + ", " + QString::number(m_nPageSize) + "";

  // 初始化设备Tree
  QVector<CDevInfo> aDev;
  int32_t nRet = DATABASE()->SelectTableDevice(&aDev, sFilter);
  if (nRet == 0) {
    CDevListItem* pDev = NULL;
    QListWidgetItem* pItem = NULL;
    foreach(CDevInfo cDev, aDev) {
      pItem = new QListWidgetItem(ui->pDevListWidget);
      if (pItem) {
        ui->pDevListWidget->addItem(pItem);
        pDev = new CDevListItem(cDev, this);
        if (pDev) {
          pItem->setSizeHint(QSize(DEV_ITEM_W, DITEM_H));
          ui->pDevListWidget->setItemWidget(pItem, pDev);
        }
      }
    }
  }
#endif
}

void CDevListWidget::resizeEvent(QResizeEvent* pEvt) {
  OnDevListSlot();
  QWidget::resizeEvent(pEvt);
}

