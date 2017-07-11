#include "cmytitlebar.h"
#include <QHBoxLayout>
#include <QPainter>
#include <QFile>
#include <QMouseEvent>
#include <QDebug>

CMyTitleBar::CMyTitleBar(QWidget *parent)
  : QWidget(parent)
  , m_colorR(20)
  , m_colorG(108)
  , m_colorB(171)
  , m_isPressed(false)
  , m_buttonType(E_MIN_BTN)
  , m_windowBorderWidth(0)
  , m_isTransparent(false) {
  // ��ʼ��;
  initControl();
  initConnections();
  loadStyleSheet(":/resource/mytitle.css");
}

CMyTitleBar::~CMyTitleBar() {

}

// ��ʼ���ؼ�;
void CMyTitleBar::initControl() {
  m_pIcon = new QLabel;
  m_pTitleContent = new QLabel;

  m_pButtonMin      = new QPushButton;
  m_pButtonRestore  = new QPushButton;
  m_pButtonMax      = new QPushButton;
  m_pButtonClose    = new QPushButton;

  m_pButtonMin->setFixedSize(QSize(DEF_TBTN_WIDTH, DEF_TBTN_HEIGHT));
  m_pButtonRestore->setFixedSize(QSize(DEF_TBTN_WIDTH, DEF_TBTN_HEIGHT));
  m_pButtonMax->setFixedSize(QSize(DEF_TBTN_WIDTH, DEF_TBTN_HEIGHT));
  m_pButtonClose->setFixedSize(QSize(DEF_TBTN_WIDTH, DEF_TBTN_HEIGHT));

  m_pTitleContent->setObjectName("TitleContent");
  m_pButtonMin->setObjectName("ButtonMin");
  m_pButtonRestore->setObjectName("ButtonRestore");
  m_pButtonMax->setObjectName("ButtonMax");
  m_pButtonClose->setObjectName("ButtonClose");

  QHBoxLayout* mylayout = new QHBoxLayout(this);
  mylayout->addWidget(m_pIcon);
  mylayout->addWidget(m_pTitleContent);

  mylayout->addWidget(m_pButtonMin);
  mylayout->addWidget(m_pButtonRestore);
  mylayout->addWidget(m_pButtonMax);
  mylayout->addWidget(m_pButtonClose);

  mylayout->setContentsMargins(0, 0, 0, 0);
  mylayout->setSpacing(0);

  m_pTitleContent->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  this->setFixedHeight(DEF_TITLE_HEIGHT);
  this->setWindowFlags(Qt::FramelessWindowHint);
}

// �źŲ۵İ�;
void CMyTitleBar::initConnections() {
  connect(m_pButtonMin, SIGNAL(clicked()), this, SLOT(onButtonMinClicked()));
  connect(m_pButtonRestore, SIGNAL(clicked()), this, SLOT(onButtonRestoreClicked()));
  connect(m_pButtonMax, SIGNAL(clicked()), this, SLOT(onButtonMaxClicked()));
  connect(m_pButtonClose, SIGNAL(clicked()), this, SLOT(onButtonCloseClicked()));
}

// ���ñ���������ɫ,��paintEvent�¼��н��л��Ʊ���������ɫ;
//�ڹ��캯���и���Ĭ��ֵ�������ⲿ������ɫֵ�ı����������ɫ;
void CMyTitleBar::setBackgroundColor(int r, int g, int b, bool isTransparent) {
  m_colorR = r;
  m_colorG = g;
  m_colorB = b;
  m_isTransparent = isTransparent;

  // ���»��ƣ�����paintEvent�¼���;
  update();
}

// ���ñ�����ͼ��;
void CMyTitleBar::setTitleIcon(QString filePath) {
  QPixmap titleIcon(filePath);
  m_pIcon->setPixmap(titleIcon.scaled(DEF_TITLE_HEIGHT*1.5, DEF_TITLE_HEIGHT));
}

// ���ñ�������;
void CMyTitleBar::setTitleContent(QString titleContent, int titleFontSize) {
  // ���ñ��������С;
  QFont font = m_pTitleContent->font();
  font.setPointSize(titleFontSize);
  m_pTitleContent->setFont(font);
  // ���ñ�������;
  m_pTitleContent->setText(titleContent);
  m_titleContent = titleContent;
}

// ���ñ���������;
void CMyTitleBar::setTitleWidth(int width) {
  this->setFixedWidth(width);
}

// ���ñ������ϰ�ť����;
// ���ڲ�ͬ���ڱ������ϵİ�ť����һ�������Կ����Զ���������еİ�ť;
// �����ṩ���ĸ���ť���ֱ�Ϊ��С������ԭ����󻯡��رհ�ť�������Ҫ������ť�������������;
void CMyTitleBar::setButtonType(E_TBTN_TYPE eBtnType) {
  m_buttonType = eBtnType;

  switch (eBtnType) {
  case E_MIN_BTN: {
    m_pButtonRestore->setVisible(false);
    m_pButtonMax->setVisible(false);
  }
  break;

  case E_MIN_MAX_BTN: {
    m_pButtonRestore->setVisible(false);
  }
  break;

  case E_CLOSE_BTN: {
    m_pButtonMin->setVisible(false);
    m_pButtonRestore->setVisible(false);
    m_pButtonMax->setVisible(false);
  }
  break;

  default:
    break;
  }
}

// ���ñ������еı����Ƿ���Զ�����������Ƶ�Ч��;
// һ������±������еı��������ǲ������ģ����Ǽ�Ȼ�Զ���Ϳ��Լ���Ҫ�����ô��ƾ���ô��O(��_��)O��
void CMyTitleBar::setTitleRoll() {
  connect(&m_titleRollTimer, SIGNAL(timeout()), this, SLOT(onRollTitle()));

  m_titleRollPos = 0;
  m_titleRollTimer.start(200);
}

void CMyTitleBar::setWindowBorderWidth(int borderWidth) {
  m_windowBorderWidth = borderWidth;
}

// ���洰�����ǰ���ڵ�λ���Լ���С;
void CMyTitleBar::saveRestoreInfo(const QPoint point, const QSize size) {
  m_restorePos = point;
  m_restoreSize = size;
}

// ��ȡ�������ǰ���ڵ�λ���Լ���С;
void CMyTitleBar::getRestoreInfo(QPoint& point, QSize& size) {
  point = m_restorePos;
  size = m_restoreSize;
}

// ���Ʊ���������ɫ;
void CMyTitleBar::paintEvent(QPaintEvent *event) {
  if (!m_isTransparent) {
    //���ñ���ɫ;
    QPainter painter(this);
    QPainterPath pathBack;
    pathBack.setFillRule(Qt::WindingFill);
    pathBack.addRoundedRect(QRect(0, 0, this->width(), this->height()), 3, 3);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.fillPath(pathBack, QBrush(QColor(m_colorR, m_colorG, m_colorB)));
  }

  // ��������󻯻��߻�ԭ�󣬴��ڳ��ȱ��ˣ��������ĳ���Ӧ��һ��ı�;
  if (this->width() != (this->parentWidget()->width() - m_windowBorderWidth)) {
    this->setFixedWidth(this->parentWidget()->width() - m_windowBorderWidth);
  }
  QWidget::paintEvent(event);
}

// ˫����Ӧ�¼�����Ҫ��ʵ��˫��������������󻯺���С������;
void CMyTitleBar::mouseDoubleClickEvent(QMouseEvent *event) {
  // ֻ�д�����󻯡���ԭ��ťʱ˫������Ч;
  if (m_buttonType == E_MIN_MAX_BTN) {
    // ͨ����󻯰�ť��״̬�жϵ�ǰ�����Ǵ�����󻯻���ԭʼ��С״̬;
    // ����ͨ���������ñ�������ʾ��ǰ����״̬;
    if (m_pButtonMax->isVisible()) {
      onButtonMaxClicked();
    } else {
      onButtonRestoreClicked();
    }
  }

  return QWidget::mouseDoubleClickEvent(event);
}

// ����ͨ��mousePressEvent��mouseMoveEvent��mouseReleaseEvent�����¼�ʵ��������϶��������ƶ����ڵ�Ч��;
void CMyTitleBar::mousePressEvent(QMouseEvent *event) {
  if (m_buttonType == E_MIN_MAX_BTN) {
    // �ڴ������ʱ��ֹ�϶�����;
    if (m_pButtonMax->isVisible()) {
      m_isPressed = true;
      m_startMovePos = event->globalPos();
    }
  } else {
    m_isPressed = true;
    m_startMovePos = event->globalPos();
  }

  return QWidget::mousePressEvent(event);
}

void CMyTitleBar::mouseMoveEvent(QMouseEvent *event) {
  if (m_isPressed) {
    QPoint movePoint = event->globalPos() - m_startMovePos;
    QPoint widgetPos = this->parentWidget()->pos();
    m_startMovePos = event->globalPos();
    this->parentWidget()->move(widgetPos.x() + movePoint.x(), widgetPos.y() + movePoint.y());
  }
  return QWidget::mouseMoveEvent(event);
}

void CMyTitleBar::mouseReleaseEvent(QMouseEvent *event) {
  m_isPressed = false;
  return QWidget::mouseReleaseEvent(event);
}

// ���ر�����ʽ�ļ�;
// ���Խ���ʽֱ��д���ļ��У���������ʱֱ�Ӽ��ؽ���;
void CMyTitleBar::loadStyleSheet(const QString &sheetName) {
  QFile file(sheetName);
  file.open(QFile::ReadOnly);
  if (file.isOpen()) {
    QString styleSheet = this->styleSheet();
    styleSheet += QLatin1String(file.readAll());
    this->setStyleSheet(styleSheet);
  }
}

// ����Ϊ��ť������Ӧ�Ĳ�;
void CMyTitleBar::onButtonMinClicked() {
  emit signalButtonMinClicked();
}

void CMyTitleBar::onButtonRestoreClicked() {
  m_pButtonRestore->setVisible(false);
  m_pButtonMax->setVisible(true);
  emit signalButtonRestoreClicked();
}

void CMyTitleBar::onButtonMaxClicked() {
  m_pButtonMax->setVisible(false);
  m_pButtonRestore->setVisible(true);
  emit signalButtonMaxClicked();
}

void CMyTitleBar::onButtonCloseClicked() {
  emit signalButtonCloseClicked();
}

// �÷�����Ҫ���ñ������еı�����ʾΪ������Ч��;
void CMyTitleBar::onRollTitle() {
  QString titleContent = m_titleContent;
  // ����ȡ��λ�ñ��ַ�����ʱ����ͷ��ʼ;
  if (m_titleRollPos > titleContent.length())
    m_titleRollPos = 0;

  m_pTitleContent->setText(titleContent.mid(m_titleRollPos));
  m_titleRollPos++;
}
