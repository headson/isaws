/************************************************************************
*Author      : Sober.Peng 17-07-13
*Description : 
************************************************************************/
#ifndef PC_MANAGE_CDEVLISTITEM_H
#define PC_MANAGE_CDEVLISTITEM_H

#include "appinc.h"

namespace Ui { class CDevListItem; };

#define DEV_ITEM_W  160
#define DEV_ITEM_H  120
class CDevListItem : public QWidget
{
    Q_OBJECT

public:
    CDevListItem(CDevInfo& cDev, QWidget *parent = NULL);
    ~CDevListItem();

    CDevInfo&   Device() { return dev_info_; }

protected slots:
    // �÷�����Ҫ���ñ������еı�����ʾΪ������Ч��;
    void        OnRollNameSlot();

private:
    Ui::CDevListItem*   ui;
    QWidget*            parent_;
    CDevInfo            dev_info_;

    // �����������Ч��ʱ��;
    int                 name_roll_pos_;
    QTimer              name_roll_timer_;
};

#endif  // PC_MANAGE_CDEVLISTITEM_H
