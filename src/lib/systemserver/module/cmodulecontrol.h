/************************************************************************/
/* Author      : SoberPeng 2017-08-02
/* Description :
/************************************************************************/
#ifndef LIBSYSTEMSERVER_CMODULECONTROL_H
#define LIBSYSTEMSERVER_CMODULECONTROL_H

class CModuleMonitor {
 public:
  // 重启模块
  static void ReStartModule();

  // 系统升级停止一些模块
  static void StopSomeModule();
};

#endif  // LIBSYSTEMSERVER_CMODULECONTROL_H
