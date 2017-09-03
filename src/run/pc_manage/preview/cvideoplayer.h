#pragma once

#include <QtWidgets/QWidget>
#include <ActiveQt/QAxBindable>

#include "ui_cvideoplayer.h"

#ifdef _AVPLAYER_DEF_
#define AVPLAYER_API _declspec(dllexport)
#else
#define AVPLAYER_API _declspec(dllimport)
#endif

class CPlayWidget;
class CHttpFlvClient;
class CVideoPlayer : public QWidget {
  Q_OBJECT

 public:
  CVideoPlayer(QWidget *parent = Q_NULLPTR);
  ~CVideoPlayer();

  void HideAllOther();

 public slots:
  bool PlayUrl(const QString &url);
  void Stop();

 protected slots:
  void dropEvent(QDropEvent *evt);            // мов╖
  void dragEnterEvent(QDragEnterEvent *evt);

  void OnSharpButtonSlot();
  void OnCloseButtonSlot();

 private:
  Ui::CVideoPlayerClass ui;

 private:
  CHttpFlvClient *vz_player_;
  CPlayWidget    *vdo_widget_;

  QString         dev_info_;
  unsigned int    sharp_index_;
};
