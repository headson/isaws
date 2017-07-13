/************************************************************************
*Author      : Sober.Peng 17-07-13
*Description : 
************************************************************************/
#ifndef PC_MANAGE_CDEVLISTWIDGET_H
#define PC_MANAGE_CDEVLISTWIDGET_H

#include "appinc.h"
#include "cdevlistitem.h"

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

 public slots:
  void    resizeEvent(QResizeEvent* pEvt);

  void    OnDetectSlot();                     // 发送探测信息
  void    OnDevListSlot();                    // 下载设备列表

 private:
  Ui::CDevListWidget* ui;
};

#endif  // PC_MANAGE_CDEVLISTWIDGET_H
