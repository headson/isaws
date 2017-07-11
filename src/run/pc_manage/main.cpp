#include "appinc.h"

#include "maindialog/cmaindialog.h"

TAG_APPLET   g_app;
int main(int argc, char *argv[]) {
  QApplication a(argc, argv);

  CMainDialog w;
  if (!w.CheckLogin()) {
    exit(EXIT_FAILURE);
  }

  w.show();
  return a.exec();
}
