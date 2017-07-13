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

  QString FindFilter();
  void    OnFindSlot();                       // 查找设备

  void    OnDevListSlot();                    // 下载设备列表
  void    OnDevPrePageSlot();                 // 显示上一页
  void    OnDevNextPageSlot();                // 显示下一页

 private:
  Ui::CDevListWidget* ui;

 private:
  int   page_size_;        // 一页设备显示数
  int   page_count_;       // 设备总数
  int   page_index_;       // 当前显示页索引
};

#endif  // PC_MANAGE_CDEVLISTWIDGET_H
