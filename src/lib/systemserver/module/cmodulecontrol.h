/************************************************************************/
/* Author      : SoberPeng 2017-08-02
/* Description :
/************************************************************************/
#ifndef LIBSYSTEMSERVER_CMODULECONTROL_H
#define LIBSYSTEMSERVER_CMODULECONTROL_H

class CModuleMonitor {
 public:
  // ����ģ��
  static void ReStartModule();

  // ϵͳ����ֹͣһЩģ��
  static void StopSomeModule();
};

#endif  // LIBSYSTEMSERVER_CMODULECONTROL_H
