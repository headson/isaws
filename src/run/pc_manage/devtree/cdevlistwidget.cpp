#include "cdevlistwidget.h"
#include "ui_cdevlistwidget.h"

CDevListWidget::CDevListWidget(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::CDevListWidget)
  , page_size_(5)
  , page_count_(0)
  , page_index_(0) {
  ui->setupUi(this);
  ui->dev_list_->setContentsMargins(0, 0, 0, 0);

  // style sheet
  QFile file(":/resource/devlistwidget.css");
  if (file.open(QFile::ReadOnly)) {
    this->setStyleSheet(QLatin1String(file.readAll()));
    file.close();
  }

  // 监听
  connect(ui->find_button_, SIGNAL(clicked()), SLOT(OnFindSlot()));
  connect(ui->prev_button_, SIGNAL(clicked()), SLOT(OnDevPrePageSlot()));
  connect(ui->next_button_, SIGNAL(clicked()), SLOT(OnDevNextPageSlot()));
}

CDevListWidget::~CDevListWidget() {
  delete ui;
}

int CDevListWidget::Initinal() {
  OnDevListSlot();
  return 0;
}

QString CDevListWidget::FindFilter() {
  QString sFindText = ui->find_edit_->text();
#if 0
  bool bOnlineShow = ui->pIsOnlineCBox->checkState();
  int32_t nFindType = ui->pFindTypeCBox->currentIndex();

  QString sFilter = "";
  if (!sFindText.isEmpty()) {
    if (0 == nFindType) {         // 设备ID
      sFilter += " WHERE d.id like '%" + sFindText + "%'";
    } else if (1 == nFindType) {  // 设备名
      sFilter += " WHERE d.name like '%" + sFindText + "%'";
    } else if (2 == nFindType) {  // 用户名
      sFilter += " left join user_device ud on ud.device_id = d.ID and ud.USER_RIGHT_ID=1000 "
                 "left join user u on u.id = ud.USER_ID "
                 "left join device_type dt on dt.id = d.DEVICE_TYPE_ID "
                 "where u.USERNAME LIKE  '%" + sFindText + "%'";
    }

    if (bOnlineShow) {
      sFilter += " AND d.IS_ONLINE=1";
    }
  } else {
    if (bOnlineShow) {
      sFilter = " WHERE d.IS_ONLINE=1";
    }
  }
#endif
  return sFindText;
}

void CDevListWidget::OnFindSlot() {
  page_index_ = 0;

  page_size_ = ui->dev_list_->height() / DEV_ITEM_H;

  OnDevListSlot();
}

void CDevListWidget::OnDevListSlot() {
  QString sFilter = FindFilter();
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

void CDevListWidget::OnDevPrePageSlot() {
  page_index_--;
  if (page_index_ < 0) {
    page_index_ = 0;
  }
  //ui->pCurPageCBox->setCurrentIndex(m_nPageIndex);

  OnDevListSlot();
}

void CDevListWidget::OnDevNextPageSlot() {
  page_index_++;
  if (page_index_ >= page_count_) {
    page_index_ = page_count_ - 1;
  }
  //ui->pCurPageCBox->setCurrentIndex(m_nPageIndex);

  OnDevListSlot();
}

void CDevListWidget::resizeEvent(QResizeEvent* pEvt) {
  OnDevListSlot();
  QWidget::resizeEvent(pEvt);
}
