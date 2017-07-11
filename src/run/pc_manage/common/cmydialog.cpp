#include "cmydialog.h"

#include <QFile>
#include <QPainter>
#include <QStyleOption>
#include <QApplication>
#include <QDesktopWidget>

CMyDialog::CMyDialog(QWidget *parent)
  : QDialog(parent) {
  // FramelessWindowHint属性设置窗口去除边框;
  // WindowMinimizeButtonHint 属性设置在窗口最小化时，点击任务栏窗口可以显示出原窗口;
  //Qt::WindowFlags flag = this->windowFlags();
  this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinimizeButtonHint/* | Qt::WindowStaysOnTopHint*/);

  // 设置窗口背景透明;
  setAttribute(Qt::WA_TranslucentBackground);

  // 关闭窗口时释放资源;
  //setAttribute(Qt::WA_DeleteOnClose);

  // 初始化标题栏;
  initTitleBar();
}

CMyDialog::~CMyDialog() {
  if (m_pBarTitle) {
    delete m_pBarTitle;
    m_pBarTitle = NULL;
  }
}

void CMyDialog::initTitleBar() {
  m_pBarTitle = new CMyTitleBar(this);
  m_pBarTitle->move(0, 0);

  connect(m_pBarTitle, SIGNAL(signalButtonMinClicked()),     this, SLOT(onButtonMinClicked()));
  connect(m_pBarTitle, SIGNAL(signalButtonRestoreClicked()), this, SLOT(onButtonRestoreClicked()));
  connect(m_pBarTitle, SIGNAL(signalButtonMaxClicked()),     this, SLOT(onButtonMaxClicked()));
  connect(m_pBarTitle, SIGNAL(signalButtonCloseClicked()),   this, SLOT(onButtonCloseClicked()));
}

void CMyDialog::paintEvent(QPaintEvent* event) {
  Q_UNUSED(event);
  QStyleOption cOpt;
  cOpt.init(this);

  QPainter cPaint(this);
  style()->drawPrimitive(QStyle::PE_Widget, &cOpt, &cPaint, this);

  //设置背景色;
  QPainterPath cPathPaint;
  cPathPaint.setFillRule(Qt::WindingFill);
  cPathPaint.addRoundedRect(QRect(0, 0, this->width(), this->height()), 3, 3);
  cPaint.setRenderHint(QPainter::SmoothPixmapTransform, true);
  cPaint.fillPath(cPathPaint, QBrush(QColor(238, 238, 238)));

  //return QWidget::paintEvent(event);
}

void CMyDialog::loadStyleSheet(const QString &sheetName) {
  QFile file(sheetName);
  file.open(QFile::ReadOnly);
  if (file.isOpen()) {
    QString styleSheet = this->styleSheet();
    styleSheet += QLatin1String(file.readAll());
    this->setStyleSheet(styleSheet);
  }
}

void CMyDialog::onButtonMinClicked() {
  if (Qt::Tool == (windowFlags() & Qt::Tool)) {
    hide();    //设置了Qt::Tool 如果调用showMinimized()则窗口就销毁了？？？
  } else {
    showMinimized();
  }
}

void CMyDialog::onButtonRestoreClicked() {
  QPoint windowPos;
  QSize windowSize;
  m_pBarTitle->getRestoreInfo(windowPos, windowSize);
  this->setGeometry(QRect(windowPos, windowSize));
}

void CMyDialog::onButtonMaxClicked() {
  m_pBarTitle->saveRestoreInfo(this->pos(), QSize(this->width(), this->height()));
  QRect desktopRect = QApplication::desktop()->availableGeometry();
  QRect FactRect = QRect(desktopRect.x() - 3, desktopRect.y() - 3, desktopRect.width() + 6, desktopRect.height() + 6);
  setGeometry(FactRect);
}

void CMyDialog::onButtonCloseClicked() {
  // 注意 ！！！;
  // 如果设置了Qt::Tool 且当前窗口为主窗口调用close()方法只是关闭了当前窗口，并没有结束整个程序进程;
  // 如果只有一个窗口情况下,需要调用qApp->quit();来结束进程;
  // 如果设置了Qt::Tool的窗口是子窗口，则只需要调用close()方法关闭即可;
  // if (Qt::Tool == (windowFlags() & Qt::Tool))
  // {
  //      qApp->quit();
  // }
  // else
  //{
  //    close();
  //}
  reject();
}

void CMyDialog::InitMyTitle(E_TBTN_TYPE eType, QString sTitle, int nSize) {
  m_pBarTitle->move(0, 0);
  m_pBarTitle->raise();
  //m_titleBar->setBackgroundColor(0, 0, 0, true);
  m_pBarTitle->setButtonType(eType);
  m_pBarTitle->setTitleWidth(this->width());
  m_pBarTitle->setTitleContent(sTitle, nSize);
}
