#include "cdevlistitem.h"
#include "ui_cdevlistitem.h"

CDevListItem::CDevListItem(CDevInfo& cDev, QWidget *parent)
    : QWidget(parent)
    , parent_(parent)
    , dev_info_(cDev)
    , ui(new Ui::CDevListItem())
{
    ui->setupUi(this);
    ui->pImageLabel->installEventFilter(this);

    // style sheet
    QFile file(":/resource/devlistitem.css");
    if (file.open(QFile::ReadOnly)) {
        this->setStyleSheet(QLatin1String(file.readAll()));
        file.close();
    }
    ui->pUuidLabel->setText(dev_info_.id_);
    ui->pNameLabel->setText(dev_info_.name_);
    if (!dev_info_.photo_.isEmpty())
    {
        int nWidth = ui->pImageLabel->width();
        int nHeight = ui->pImageLabel->height();

        ui->pImageLabel->setScaledContents(true);
        ui->pImageLabel->setPixmap(dev_info_.GetPhoto(dev_info_.photo_, nWidth, nHeight));
    }
    if (cDev.is_online_ == 0) {
        ui->pImageLabel->setText(tr("设备不在线"));
    }

    // 名字滚动
    if (ui->pNameLabel->text().size() > 15) {
        connect(&name_roll_timer_, 
            SIGNAL(timeout()), SLOT(OnRollNameSlot()));
        name_roll_pos_ = 0; name_roll_timer_.start(500);
    }
}

CDevListItem::~CDevListItem()
{
    delete ui;
    name_roll_timer_.stop();
}

void CDevListItem::OnRollNameSlot()
{
    QString titleContent = dev_info_.name_;
    // 当截取的位置比字符串长时，从头开始;
    if (name_roll_pos_ > titleContent.length())
        name_roll_pos_ = 0;

    ui->pNameLabel->setText(titleContent.mid(name_roll_pos_));
    name_roll_pos_++;
}
