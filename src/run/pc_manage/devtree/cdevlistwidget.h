/************************************************************************
*Author      : Sober.Peng 17-07-13
*Description :
************************************************************************/
#ifndef PC_MANAGE_CDEVLISTWIDGET_H
#define PC_MANAGE_CDEVLISTWIDGET_H

#include "appinc.h"
#include <QTreeWidgetItem>

namespace Ui {
class CDevListWidget;
};

class CPlayFrame;
class CDevListWidget : public QWidget {
  Q_OBJECT

 public:
  CDevListWidget(QWidget *parent = NULL);
  ~CDevListWidget();

  int Initinal();

 protected slots:
  virtual bool eventFilter(QObject *pObj, QEvent *pEvt);

  void OnItemClickSlot(QTreeWidgetItem*, int);
  // 右键菜单
  virtual void contextMenuEvent(QContextMenuEvent *pEvt);

 public slots:
  void    OnDetectSlot();                     // 发送探测信息
  void    OnDevListSlot();                    // 下载设备列表

 protected slots:
  void    OnDevCfgSlot();
  void    OnDbgVdoSlot();
  void    OnClrPcntSlot();

 private:
  Ui::CDevListWidget* ui;
  QString             dev_info_;

 private:
  QMenu*              item_menu_;            // 菜单项
};

#endif  // PC_MANAGE_CDEVLISTWIDGET_H
