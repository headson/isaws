#include "clivewidget.h"

CLiveWidget::CLiveWidget(QWidget *parent)
  : QWidget(parent)
  , m_pVideoShowLayout(NULL) {
  setWindowFlags(Qt::Window);
  ui.setupUi(this);

  OnMultShowSlot(4);
}

CLiveWidget::~CLiveWidget() {
  Clear();
}

void CLiveWidget::Clear() {
  for (size_t i = 0; i < m_aVideoShows.size();  i++) {
    QWidget *pShow = m_aVideoShows.at(i);

    if (pShow) {
      delete pShow;
      pShow = NULL;
    }
  }
  m_aVideoShows.clear();
}

void CLiveWidget::OnMultShowSlot(int nCount) {
  Clear();

  if (m_pVideoShowLayout) {
    delete m_pVideoShowLayout;
    m_pVideoShowLayout = NULL;
  }
  int nDiv =  (int)sqrt((double)nCount);
  m_pVideoShowLayout = new QGridLayout(ui.m_pLiveGroupBox);
  m_pVideoShowLayout->setVerticalSpacing(nDiv);
  m_pVideoShowLayout->setHorizontalSpacing(nDiv);
  m_pVideoShowLayout->setContentsMargins(0, 0, 0, 0);

  for (int i = 0; i < nDiv; i++) {
    for (int j = 0; j < nDiv; j++) {
      CVideoPlayer* pShow = new CVideoPlayer(ui.m_pLiveGroupBox);

      if (pShow) {
        m_aVideoShows.append(pShow);
        m_pVideoShowLayout->addWidget(pShow, i, j, 1, 1);

        connect(pShow, SIGNAL(DoubleClicked(bool, QObject*)), SLOT(OnWinDClickSlot(bool, QObject*)));
      }
    }
  }
}

void CLiveWidget::OnWinDClickSlot(bool bOne, QObject* pObj) {
  for (size_t i = 0; i < m_aVideoShows.size();  i++) {
    if (m_aVideoShows.at(i) != pObj)
      m_aVideoShows.at(i)->setVisible(!bOne);
  }
}
