#include "cdevlistwidget.h"
#include "ui_cdevlistwidget.h"

#include <QDrag>
#include <QMenu>
#include <QMimeData>
#include <QMouseEvent>

#include "config/cdevcfgwidget.h"
#include "config/cdbgvdowidget.h"

CDevListWidget::CDevListWidget(QWidget *parent)
  : QWidget(parent)
  , ui(new Ui::CDevListWidget) {
  item_menu_ = NULL;
  ui->setupUi(this);
  // ui->dev_list_->setContentsMargins(0, 0, 0, 0);

  // style sheet
  //QFile file(":/resource/devlistwidget.css");
  //if (file.open(QFile::ReadOnly)) {
  //  this->setStyleSheet(QLatin1String(file.readAll()));
  //  file.close();
  //}

  ui->pDevTreeWidget->setDragEnabled(true);
  ui->pDevTreeWidget->setDropIndicatorShown(true);
  ui->pDevTreeWidget->viewport()->installEventFilter(this);
  ui->pDevTreeWidget->setSelectionMode(QAbstractItemView::SingleSelection);
  ui->pDevTreeWidget->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
  //ui->pDevTreeWidget->header()->setResizeMode(QHeaderView::ResizeToContents);

  // 监听
  connect(CDeviceDetect::Instance(), SIGNAL(UpdateDevice()), SLOT(OnDevListSlot()));
  connect(ui->pDevTreeWidget, SIGNAL(itemPressed(QTreeWidgetItem*, int)), SLOT(OnItemClickSlot(QTreeWidgetItem*, int)));
}

CDevListWidget::~CDevListWidget() {
  delete ui;
}

int CDevListWidget::Initinal() {
  OnDevListSlot();
  return 0;
}

bool CDevListWidget::eventFilter(QObject *pObj, QEvent *pEvt) {
  if ((pObj == ui->pDevTreeWidget->viewport()) &&
      (pEvt->type() == QEvent::MouseButtonPress)) {
    if (((QMouseEvent*)pEvt)->button() == Qt::RightButton) {
    }
    else if (dev_info_ != "") {
      QDrag* pDrag = new QDrag(this);
      QMimeData* pMimeData = new QMimeData();
      if ((NULL != pDrag) && (NULL != pMimeData)) {
        pMimeData->setData("data1", dev_info_.toLocal8Bit());

        pDrag->setMimeData(pMimeData);
        pDrag->exec(Qt::CopyAction | Qt::MoveAction);
      }
    }
  }

  return QWidget::eventFilter(pObj, pEvt);
}

void CDevListWidget::OnItemClickSlot(QTreeWidgetItem *item, int index) {
  dev_info_ = item->text(0);        // name
  dev_info_ += ":";
  dev_info_ += item->toolTip(0);    // id
  dev_info_ += ":";
  dev_info_ += item->text(1);       // ip
  dev_info_ += ":";
  dev_info_ += item->toolTip(1);    // port
}

void CDevListWidget::contextMenuEvent(QContextMenuEvent *pEvt) {
  if (dev_info_.isEmpty())
    return;

  if (!item_menu_) {
    item_menu_ = new QMenu(ui->pDevTreeWidget);

    QAction* pAct = NULL;

    pAct = new QAction(tr("配置参数"), item_menu_);
    if (pAct) {
      item_menu_->addAction(pAct);
      connect(pAct, SIGNAL(triggered()), this, SLOT(OnDevCfgSlot()));
    }

    pAct = new QAction(tr("清空计数"), item_menu_);
    if (pAct) {
      item_menu_->addAction(pAct);
      connect(pAct, SIGNAL(triggered()), this, SLOT(OnClrPcntSlot()));
    }

    pAct = new QAction(tr("调试图像"), item_menu_);
    if (pAct) {
      item_menu_->addAction(pAct);
      connect(pAct, SIGNAL(triggered()), this, SLOT(OnDbgVdoSlot()));
    }
  }

  item_menu_->exec(QCursor::pos());
}

void CDevListWidget::OnDetectSlot() {
  CDeviceDetect::Instance()->Detect();
}

void CDevListWidget::OnDevListSlot() {
  ui->pDevTreeWidget->clear();

  QVector<CDevInfo> devs;
  CDeviceDetect::Instance()->GetDevs(devs);

  int dev_size = devs.size();
  for (int i = 0; i < dev_size; i++) {
    QTreeWidgetItem *items = new QTreeWidgetItem(ui->pDevTreeWidget);
    items->setText(0, devs[i].name_.c_str());
    items->setText(1, devs[i].ip_.c_str());
    items->setToolTip(0, devs[i].id_.c_str());
    items->setToolTip(1, QString::number(devs[i].port_));
    ui->pDevTreeWidget->insertTopLevelItem(0, items);
  }
}

void CDevListWidget::OnDevCfgSlot() {
  if (dev_info_.isEmpty()) {
    return;
  }
  QStringList sl = dev_info_.split(":");

  CDevCfgWidget dev_cfg;
  dev_cfg.Initinal(sl.at(1));
}

void CDevListWidget::OnDbgVdoSlot() {
  if (dev_info_.isEmpty()) {
    return;
  }
  QStringList sl = dev_info_.split(":");

  CDbgVdoWidget dbg_vdo;
  dbg_vdo.Initinal(sl.at(1));
}

void CDevListWidget::OnClrPcntSlot() {

}
