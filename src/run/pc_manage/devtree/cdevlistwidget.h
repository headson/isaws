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
  void    OnFindSlot();                       // �����豸

  void    OnDevListSlot();                    // �����豸�б�
  void    OnDevPrePageSlot();                 // ��ʾ��һҳ
  void    OnDevNextPageSlot();                // ��ʾ��һҳ

 private:
  Ui::CDevListWidget* ui;

 private:
  int   page_size_;        // һҳ�豸��ʾ��
  int   page_count_;       // �豸����
  int   page_index_;       // ��ǰ��ʾҳ����
};

#endif  // PC_MANAGE_CDEVLISTWIDGET_H
