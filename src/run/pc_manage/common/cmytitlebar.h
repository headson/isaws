#ifndef MYTITLEBAR_H
#define MYTITLEBAR_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QTimer>

enum E_TBTN_TYPE {
  E_MIN_BTN       = 0,// ��С���͹رհ�ť;
  E_MIN_MAX_BTN,      // ��С������󻯺͹رհ�ť;
  E_CLOSE_BTN         // ֻ�йرհ�ť;
};
#define DEF_TBTN_HEIGHT     30  // ��ť�߶�;
#define DEF_TBTN_WIDTH      30  // ��ť���;
#define DEF_TITLE_HEIGHT    30  // �������߶�;

class CMyTitleBar : public QWidget {
  Q_OBJECT

 public:
  CMyTitleBar(QWidget *parent = NULL);
  ~CMyTitleBar();

  // ���ñ���������ɫ;
  void setBackgroundColor(int r, int g, int b, bool isTransparent = false);
  // ���ñ�����ͼ��;
  void setTitleIcon(QString filePath);
  // ���ñ�������;
  void setTitleContent(QString titleContent, int titleFontSize = 9);
  // ���ñ���������;
  void setTitleWidth(int width);
  // ���ñ������ϰ�ť����;
  void setButtonType(E_TBTN_TYPE eBtnType);
  // ���ñ������еı����Ƿ�����;������Կ�Ч��;
  void setTitleRoll();
  // ���ô��ڱ߿���;
  void setWindowBorderWidth(int borderWidth);

  // ����/��ȡ ���ǰ���ڵ�λ�ü���С;
  void saveRestoreInfo(const QPoint point, const QSize size);
  void getRestoreInfo(QPoint& point, QSize& size);

 signals:
  // ��ť�������ź�;
  void signalButtonMinClicked();
  void signalButtonRestoreClicked();
  void signalButtonMaxClicked();
  void signalButtonCloseClicked();

 private:
  void paintEvent(QPaintEvent *event);
  void mouseDoubleClickEvent(QMouseEvent *event);
  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);

  // ��ʼ���ؼ�;
  void initControl();
  // �źŲ۵İ�;
  void initConnections();
  // ������ʽ�ļ�;
  void loadStyleSheet(const QString &sheetName);

 protected slots:
  // ��ť�����Ĳ�;
  void onButtonMinClicked();
  void onButtonRestoreClicked();
  void onButtonMaxClicked();
  void onButtonCloseClicked();
  void onRollTitle();

 private:
  QLabel*         m_pIcon;                // ������ͼ��;
  QLabel*         m_pTitleContent;        // ����������;
  QPushButton*    m_pButtonMin;           // ��С����ť;
  QPushButton*    m_pButtonRestore;       // ��󻯻�ԭ��ť;
  QPushButton*    m_pButtonMax;           // ��󻯰�ť;
  QPushButton*    m_pButtonClose;         // �رհ�ť;

  // ����������ɫ;
  int             m_colorR;
  int             m_colorG;
  int             m_colorB;

  // ��󻯣���С������;
  QPoint          m_restorePos;
  QSize           m_restoreSize;
  // �ƶ����ڵı���;
  bool            m_isPressed;
  QPoint          m_startMovePos;
  // �����������Ч��ʱ��;
  int             m_titleRollPos;
  QTimer          m_titleRollTimer;
  // ����������;
  QString         m_titleContent;
  // ��ť����;
  E_TBTN_TYPE      m_buttonType;
  // ���ڱ߿���;
  int             m_windowBorderWidth;
  // �������Ƿ�͸��;
  bool            m_isTransparent;
};

#endif // MYTITLEBAR_H
