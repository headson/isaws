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
  // �Ҽ��˵�
  virtual void contextMenuEvent(QContextMenuEvent *pEvt);

 public slots:
  void    OnDetectSlot();                     // ����̽����Ϣ
  void    OnDevListSlot();                    // �����豸�б�

 protected slots:
  void    OnDevCfgSlot();
  void    OnDbgVdoSlot();
  void    OnClrPcntSlot();

 private:
  Ui::CDevListWidget* ui;
  QString             dev_info_;

 private:
  QMenu*              item_menu_;            // �˵���
};

#endif  // PC_MANAGE_CDEVLISTWIDGET_H
