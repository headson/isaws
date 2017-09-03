#ifndef CLIVEWIDGET_H
#define CLIVEWIDGET_H

#include <QWidget>
#include "cvideoplayer.h"
#include "ui_clivewidget.h"

class CLiveWidget : public QWidget {
  Q_OBJECT

public:
  typedef QList<CVideoPlayer*>    VIDEO_SHOW;
  typedef VIDEO_SHOW::iterator    VIDEO_SHOW_ITER;

 public:
  CLiveWidget(QWidget *parent = 0);
  ~CLiveWidget();

 public slots:
  void OnMultShowSlot(int nCount=4);
  void OnWinDClickSlot(bool bOne, QObject* pObj);

 protected:
  void Clear();

 private:
  Ui::CLiveWidget     ui;

 private:
  VIDEO_SHOW          m_aVideoShows;
  QGridLayout*        m_pVideoShowLayout;
};

#endif // CLIVEWIDGET_H
