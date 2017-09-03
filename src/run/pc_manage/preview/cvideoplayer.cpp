#include "cvideoplayer.h"

#include "cvideowidget.h"
#include "chttpflvclient.h"
#include "vzbase/helper/stdafx.h"

void __stdcall OnPlayerCallback(int state_code,
                                unsigned int param,
                                CHttpFlvClient *player,
                                void *user_data) {

}

CVideoPlayer::CVideoPlayer(QWidget *parent)
  : QWidget(parent)
  , vz_player_(NULL) {
  ui.setupUi(this);
  setAcceptDrops(true); // мов╖

  vdo_widget_ = new CPlayWidget(this);

  QLayout* pLayout = NULL;
  pLayout = new QHBoxLayout();
  pLayout->addWidget(vdo_widget_);
  pLayout->setContentsMargins(1, 1, 1, 1);
  ui.pVideoGBox->setLayout(pLayout);

  connect(ui.pSharpButton, SIGNAL(clicked()), SLOT(OnSharpButtonSlot()));
  connect(ui.pCloseButton, SIGNAL(clicked()), SLOT(OnCloseButtonSlot()));
}

CVideoPlayer::~CVideoPlayer() {
  Stop();

  if (vdo_widget_) {
    delete vdo_widget_;
    vdo_widget_ = NULL;
  }
}

void CVideoPlayer::HideAllOther() {
  ui.pOtherGBox->setVisible(false);
}

bool CVideoPlayer::PlayUrl(const QString &url) {
  if (vz_player_) {
    delete vz_player_;
    vz_player_ = NULL;
    usleep(200*1000);
  }

  if (NULL == vz_player_) {
    vz_player_ = new CHttpFlvClient(OnPlayerCallback,
                                    this,
                                    1);
    if (vz_player_) {
      vz_player_->SetPlayUrl(url.toLocal8Bit().data());
      vz_player_->StartPlayer(AVMEDIA_TYPE_VIDEO, vdo_widget_);
      return true;
    }
  }
  return false;
}

void CVideoPlayer::Stop() {
  if (vz_player_) {
    delete vz_player_;
    vz_player_ = NULL;
  }
  ui.pSharpButton->setText("HD");
}

void CVideoPlayer::dropEvent(QDropEvent *evt) {
  dev_info_ = evt->mimeData()->data("data1");
  LOG(L_INFO) << dev_info_.toLocal8Bit().data();
  
  sharp_index_ = 1;
  OnSharpButtonSlot();
}

void CVideoPlayer::dragEnterEvent(QDragEnterEvent *evt) {
  QString sData(evt->mimeData()->data("data1"));
  LOG(L_INFO) << sData.toLocal8Bit().data();

  if (!sData.isEmpty()) {
    evt->acceptProposedAction();
    evt->accept();

    this->setMouseTracking(true);
  }
}

void CVideoPlayer::OnSharpButtonSlot() {
  QStringList sl = dev_info_.split(":");
  QString surl = "http://";
  surl += sl.at(2);
  surl += ":";
  surl += sl.at(3);
  surl += "/httpflv?chn=video";
  surl += QString::number(sharp_index_);
  PlayUrl(surl);

  if (sharp_index_ == 0) {
    sharp_index_ = 1;
    ui.pSharpButton->setText("SD");
  } else {
    sharp_index_ = 0;
    ui.pSharpButton->setText("HD");
  }
}

void CVideoPlayer::OnCloseButtonSlot() {
  Stop();
}

//QAXFACTORY_DEFAULT(CVideoPlayer,
//                   "{6251159e-a5c6-49fb-aca6-205ffad9df99}",
//                   "{08e4a961-8212-463f-980b-979ff3208652}",
//                   "{5062265d-dbeb-4872-8cc6-7c8aa9b388b7}",
//                   "{0819081a-27e4-4c00-9019-c9b540ad2cd1}",
//                   "{87c5be4b-5855-4db0-ba9c-68680c77adc2}"
//                  )

