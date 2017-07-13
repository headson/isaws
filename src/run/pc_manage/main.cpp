#include "appinc.h"
#include "vzbase/helper/stdafx.h"
#include "maindialog/cmaindialog.h"

TAG_APPLET   g_app;
int main(int argc, char *argv[]) {
  InitVzLogging(argc, argv);
#ifdef WIN32
  ShowVzLoggingAlways();
#endif

  QApplication a(argc, argv);

  CDeviceDetect::Instance()->Start();

  CMainDialog w;
  //if (!w.CheckLogin()) {
  //  exit(EXIT_FAILURE);
  //}

  w.show();
  return a.exec();
}
