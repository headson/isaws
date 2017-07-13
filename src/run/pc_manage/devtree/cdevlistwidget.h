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

  void    OnDetectSlot();                     // ����̽����Ϣ
  void    OnDevListSlot();                    // �����豸�б�

 private:
  Ui::CDevListWidget* ui;
};

#endif  // PC_MANAGE_CDEVLISTWIDGET_H
